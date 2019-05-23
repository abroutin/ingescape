//
//  igs_network.c
//
//  Created by Stephane Vales on 10/06/2016.
//  Copyright © 2017 Ingenuity i/o. All rights reserved.
//

/**
  * \file ../../src/include/ingescape.h
  */

#if (defined WIN32 || defined _WIN32)
// Define UNICODE before includes for cross compile
// Because GetModuleFileName with UNICODE is  GetModuleFileNameW(void*, wchar*, size_t)
// And GetModuleFileName without UNICODE is  GetModuleFileNameW(void*, char*, size_t)
// Call GetModuleFileName without UNICODE but with wchar as parameter result of NULL string
#ifndef UNICODE
#define UNICODE
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef __APPLE__ 
#include <TargetConditionals.h>
#if TARGET_OS_OSX
#include <libproc.h>
#endif
#endif

#include <zyre.h>
#include <czmq.h>
#if (defined WIN32 || defined _WIN32)
#include "unixfunctions.h"
#endif
#include "uthash/uthash.h"
#include "uthash/utlist.h"
#include "ingescape_private.h"
#include "ingescape_advanced.h"

#ifdef _WIN32
    #define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
    #define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
    #define MAX_TRIES 3

    size_t  count;
    char *friendly_name = NULL;
    char buff[100];
    DWORD bufflen=100;

    /* Declare and initialize variables */
    DWORD dwRetVal = 0;

    unsigned int i = 0;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    // default to unspecified address family (both)
    ULONG family = AF_UNSPEC;

    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
#endif


//global flags
bool isFrozen = false;
bool agentCanBeFrozen = false;
bool isWholeAgentMuted = false;
bool igs_Interrupted = false;
bool network_RequestOutputsFromMappedAgents = false;
bool forcedStop = false;
bool allowIpc = true;

#if ENABLE_LICENSE_ENFORCEMENT
licenseEnforcement_t *licEnforcement = NULL;
#endif

//global parameters
//prefixes for sending definitions and mappings through zyre
static const char *definitionPrefix = "EXTERNAL_DEFINITION#";
static const char *mappingPrefix = "EXTERNAL_MAPPING#";
static const char *loadMappingPrefix = "LOAD_THIS_MAPPING#";
static const char *loadDefinitionPrefix = "LOAD_THIS_DEFINITION#";
#define COMMAND_LINE_LENGTH 4096
char igsAgentName[MAX_AGENT_NAME_LENGTH] = AGENT_NAME_DEFAULT;
char igsAgentState[MAX_AGENT_NAME_LENGTH] = "";
char commandLine[COMMAND_LINE_LENGTH] = "";
char replayChannel[MAX_AGENT_NAME_LENGTH + 16] = "";


typedef struct muteCallback {
    igs_muteCallback callback_ptr;
    void *myData;
    struct muteCallback *prev;
    struct muteCallback *next;
} muteCallback_t;

typedef struct freezeCallback {
    igs_freezeCallback callback_ptr;
    void *myData;
    struct freezeCallback *prev;
    struct freezeCallback *next;
} freezeCallback_t;

typedef struct zyreCallback {
    igs_BusMessageIncoming callback_ptr;
    void *myData;
    struct zyreCallback *prev;
    struct zyreCallback *next;
} zyreCallback_t;

typedef struct forcedStopCalback {
    igs_forcedStopCallback callback_ptr;
    void *myData;
    struct forcedStopCalback *prev;
    struct forcedStopCalback *next;
} forcedStopCalback_t;

bool network_needToSendDefinitionUpdate = false;
bool network_needToUpdateMapping = false;
unsigned int network_discoveryInterval = 1000;
unsigned int network_agentTimeout = 30000;
unsigned int network_publishingPort = 0;
char *ipcFolderPath = NULL;
#define DEFAULT_IPC_PATH "/tmp/"

//we manage agent data as a global variables inside the network module for now
zyreloopElements_t *agentElements = NULL;
subscriber_t *subscribers = NULL;
muteCallback_t *muteCallbacks = NULL;
freezeCallback_t *freezeCallbacks = NULL;
zyreCallback_t *zyreCallbacks = NULL;
PUBLIC zyreAgent_t *zyreAgents = NULL;
forcedStopCalback_t *forcedStopCalbacks = NULL;

////////////////////////////////////////////////////////////////////////
// INTERNAL API
////////////////////////////////////////////////////////////////////////
int subscribeToPublisherOutput(subscriber_t *subscriber, const char *outputName)
{
    if(outputName != NULL && strlen(outputName) > 0)
    {
        bool filterAlreadyExists = false;
        mappingFilter_t *filter = NULL;
        DL_FOREACH(subscriber->mappingsFilters, filter){
            if (strcmp(filter->filter, outputName) == 0){
                filterAlreadyExists = true;
                break;
            }
        }
        if (!filterAlreadyExists){
            // Set subscriber to the output filter
            igs_debug("Subscribe to agent %s output %s",subscriber->agentName,outputName);
            zsock_set_subscribe(subscriber->subscriber, outputName);
            mappingFilter_t *f = calloc(1, sizeof(mappingFilter_t));
            strncpy(f->filter, outputName, MAX_FILTER_SIZE-1);
            DL_APPEND(subscriber->mappingsFilters, f);
        }else{
            //printf("\n****************\nFILTER BIS %s - %s\n***************\n", subscriber->igsAgentName, outputName);
        }
        return 1;
    }
    return -1;
}

int unsubscribeToPublisherOutput(subscriber_t *subscriber, const char *outputName)
{
    if(outputName != NULL && strlen(outputName) > 0)
    {
        mappingFilter_t *filter = NULL;
        DL_FOREACH(subscriber->mappingsFilters, filter){
            if (strcmp(filter->filter, outputName) == 0){
                igs_debug("Unsubscribe to agent %s output %s",igsAgentName,outputName);
                zsock_set_unsubscribe(subscriber->subscriber, outputName);
                DL_DELETE(subscriber->mappingsFilters, filter);
                break;
            }
        }
        return 1;
    }
    return -1;
}

//Timer callback to send REQUEST_OUPUTS notification for agents we subscribed to
int triggerMappingNotificationToNewcomer(zloop_t *loop, int timer_id, void *arg){
    IGS_UNUSED(loop)
    IGS_UNUSED(timer_id)

    subscriber_t *subscriber = (subscriber_t *) arg;
    if (subscriber->mappedNotificationToSend){
        zyre_whispers(agentElements->node, subscriber->agentPeerId, "REQUEST_OUPUTS");
        subscriber->mappedNotificationToSend = false;
    }
    return 0;
}

int network_manageSubscriberMapping(subscriber_t *subscriber){
    //get mapping elements for this subscriber
    mapping_element_t *el, *tmp;
    if (igs_internal_mapping != NULL){
        HASH_ITER(hh, igs_internal_mapping->map_elements, el, tmp){
            if (strcmp(subscriber->agentName, el->agent_name)==0 || strcmp(el->agent_name, "*") == 0){
                //mapping element is compatible with subscriber name
                //check if we find a compatible output in subscriber definition
                agent_iop_t *foundOutput = NULL;
                if (subscriber->definition != NULL){
                    HASH_FIND_STR(subscriber->definition->outputs_table, el->output_name, foundOutput);
                }
                //check if we find a valid input in our own definition
                agent_iop_t *foundInput = NULL;
                if (igs_internal_definition != NULL){
                    HASH_FIND_STR(igs_internal_definition->inputs_table, el->input_name, foundInput);
                }
                //check type compatibility between input and output value types
                //including implicit conversions
                if (foundOutput != NULL && foundInput != NULL && mapping_checkCompatibilityInputOutput(foundInput, foundOutput)){
                    //we have validated input, agent and output names : we can map
                    //NOTE: the call below may happen several times if our agent uses
                    //the external agent ouput on several of its inputs. This should not have any consequence.
                    subscribeToPublisherOutput(subscriber, el->output_name);
                    //mapping was successful : we set timer to notify remote agent if not already done
                    if (!subscriber->mappedNotificationToSend && network_RequestOutputsFromMappedAgents){
                        subscriber->mappedNotificationToSend = true;
                        subscriber->timerId = zloop_timer(agentElements->loop, 500, 1, triggerMappingNotificationToNewcomer, (void *)subscriber);
                    }
                }
                //NOTE: we do not clean subscriptions here because we cannot check if
                //an output is not used in another mapping element
            }
        }
    }
    return 0;
}

void sendDefinitionToAgent(const char *peerId, const char *def)
{
    if(peerId != NULL &&  def != NULL)
    {
        if(agentElements->node != NULL)
        {
            zyre_whispers(agentElements->node, peerId, "%s%s", definitionPrefix, def);
        } else {
            igs_warn("Could not send our definition to %s : our agent is not connected",peerId);
        }
    }
}

void sendMappingToAgent(const char *peerId, const char *mapping)
{
    if(peerId != NULL &&  mapping != NULL){
        if(agentElements->node != NULL){
            zyre_whispers(agentElements->node, peerId, "%s%s", mappingPrefix, mapping);
        } else {
            igs_warn("Could not send our mapping to %s : our agent is not connected",peerId);
        }
    }
}

void network_cleanAndFreeSubscriber(subscriber_t *subscriber){
    igs_debug("cleaning subscription to %s\n", subscriber->agentName);
    // clean the agent definition
    if(subscriber->definition != NULL){
#if ENABLE_LICENSE_ENFORCEMENT
        licEnforcement->currentIOPNb -= (HASH_COUNT(subscriber->definition->inputs_table) +
                                         HASH_COUNT(subscriber->definition->outputs_table) +
                                         HASH_COUNT(subscriber->definition->params_table));
        licEnforcement->currentAgentsNb--;
#endif
        definition_freeDefinition(subscriber->definition);
    }
    //clean the agent mapping
    if(subscriber->mapping != NULL){
        mapping_freeMapping(subscriber->mapping);
    }
    //clean the subscriber itself
    mappingFilter_t *elt, *tmp;
    DL_FOREACH_SAFE(subscriber->mappingsFilters,elt,tmp) {
        zsock_set_unsubscribe(subscriber->subscriber, elt->filter);
        DL_DELETE(subscriber->mappingsFilters,elt);
        free(elt);
    }
    zloop_poller_end(agentElements->loop , subscriber->pollItem);
    zsock_destroy(&subscriber->subscriber);
    free((char*)subscriber->agentName);
    free((char*)subscriber->agentPeerId);
    free(subscriber->pollItem);
    free(subscriber->subscriber);
    subscriber->subscriber = NULL;
    if (subscriber->timerId != -1){
        zloop_timer_end(agentElements->loop, subscriber->timerId);
        subscriber->timerId = -2;
    }
    HASH_DEL(subscribers, subscriber);
    free(subscriber);
    subscriber = NULL;
//    int n = HASH_COUNT(subscribers);
//    igs_debug("%d subscribers in the list\n", n);
//    subscriber_t *s, *tmps;
//    HASH_ITER(hh, subscribers, s, tmps){
//        igs_debug("\tsubscriber : %s - %s\n", s->agentName, s->agentPeerId);
//    }
}

////////////////////////////////////////////////////////////////////////
// ZMQ callbacks
////////////////////////////////////////////////////////////////////////

//manage messages from the parent thread
int manageParent (zloop_t *loop, zmq_pollitem_t *item, void *arg){
    IGS_UNUSED(loop)
    IGS_UNUSED(arg)

    if (item->revents & ZMQ_POLLIN)
    {
        zmsg_t *msg = zmsg_recv ((zsock_t *)item->socket);
        if (!msg){
            igs_error("Could not read message from main thread : Agent will interrupt immediately.");
            return -1;
        }
        char *command = zmsg_popstr (msg);
        if (streq (command, "$TERM")){
            free (command);
            zmsg_destroy (&msg);
            return -1;
        } else {
            //nothing to do so far
        }

        free (command);
        zmsg_destroy (&msg);
    }
    return 0;
}

//function actually handling messages from one of the publisher agents we subscribed to
int handleSubscriptionMessage(zmsg_t *msg, const char *subscriberPeerId){
    
    // Try to find the subscriber object
    subscriber_t * foundSubscriber = NULL;
    HASH_FIND_STR(subscribers, subscriberPeerId, foundSubscriber);
    
    if(foundSubscriber == NULL){
        igs_error("Could not find subscriber structure for agent %s", subscriberPeerId);
        return -1;
    }
    if(isFrozen == true){
        igs_debug("Message received from agent %s but all traffic in our agent has been frozen", foundSubscriber->agentName);
        return 0;
    }
    
    size_t msgSize = zmsg_size(msg);
    char *output = NULL;
    char *vType = NULL;
    iopType_t valueType = 0;
    unsigned long i = 0;
    for (i = 0; i < msgSize; i += 3){
        // Each message part must contain 3 elements
        // 1 : output name
        // 2 : output ioptType
        // 3 : value of the output as a string or zframe
        output = zmsg_popstr(msg);
        vType = zmsg_popstr(msg);
        valueType = atoi(vType);
        free(vType);
        vType = NULL;
        
        zframe_t *frame = NULL;
        void *data = NULL;
        size_t size = 0;
        char * value = NULL;
        //get data before iterating to all the mapping elements using it
        if (valueType == IGS_STRING_T){
            value = zmsg_popstr(msg);
        }else{
            frame = zmsg_pop(msg);
            data = zframe_data(frame);
            size = zframe_size(frame);
        }
        //try to find mapping elements matching with this subscriber's output
        //and update mapped input(s) value accordingly
        //TODO : some day, optimize mapping storage to avoid iterating
        mapping_element_t *elmt, *tmp;
        HASH_ITER(hh, igs_internal_mapping->map_elements, elmt, tmp) {
            if (strcmp(elmt->agent_name, foundSubscriber->agentName) == 0
                && strcmp(elmt->output_name, output) == 0){
                //we have a match on emitting agent name and its ouput name :
                //still need to check the targeted input existence in our definition
                agent_iop_t *foundInput = NULL;
                if (igs_internal_definition->inputs_table != NULL){
                    HASH_FIND_STR(igs_internal_definition->inputs_table, elmt->input_name, foundInput);
                }
                if (foundInput == NULL){
                    igs_error("Input %s is missing in our definition but expected in our mapping with %s.%s",
                               elmt->input_name,
                               elmt->agent_name,
                               elmt->output_name);
                }else{
                    //we have a fully matching mapping element : write from received output to our input
                    if (valueType == IGS_STRING_T){
                        model_writeIOP(elmt->input_name, IGS_INPUT_T, valueType, value, strlen(value)+1);
                    }else{
                        model_writeIOP(elmt->input_name, IGS_INPUT_T, valueType, data, size);
                    }
                }
            }
        }
        if (frame != NULL){
            zframe_destroy(&frame);
        }
        if (value != NULL){
            free(value);
        }
        free(output);
        output = NULL;
    }
    
    return 0;
}

//manage incoming messages from one of the publisher agents we subscribed to
int manageSubscription (zloop_t *loop, zmq_pollitem_t *item, void *arg){
    IGS_UNUSED(loop)

    //get subscriber id
    char *subscriberPeerId = (char *)arg;

    if (item->revents & ZMQ_POLLIN && strlen(subscriberPeerId) > 0){
        zmsg_t *msg = zmsg_recv(item->socket);
        handleSubscriptionMessage(msg, subscriberPeerId);
        zmsg_destroy(&msg);
    }
    return 0;
}

//manage messages received on the bus
int manageBusIncoming (zloop_t *loop, zmq_pollitem_t *item, void *arg){
    IGS_UNUSED(arg)

    zyre_t *node = agentElements->node;
    if (item->revents & ZMQ_POLLIN){
        zyre_event_t *zyre_event = zyre_event_new (node);
        const char *event = zyre_event_type (zyre_event);
        const char *peer = zyre_event_peer_uuid (zyre_event);
        const char *name = zyre_event_peer_name (zyre_event);
        const char *address = zyre_event_peer_addr (zyre_event);
        zhash_t *headers = zyre_event_headers (zyre_event);
        const char *group = zyre_event_group (zyre_event);
        zmsg_t *msg = zyre_event_msg (zyre_event);
        zmsg_t *msgDuplicate = zmsg_dup(msg);
        
        //parse event
        if (streq (event, "ENTER")){
            igs_debug("->%s has entered the network with peer id %s and endpoint %s", name, peer, address);
            zyreAgent_t *zagent = NULL;
            HASH_FIND_STR(zyreAgents, peer, zagent);
            if (zagent == NULL){
                zagent = calloc(1, sizeof(zyreAgent_t));
                zagent->reconnected = 0;
                zagent->subscriber = NULL;
                zagent->hasJoinedPrivateChannel = false;
                strncpy(zagent->peerId, peer, NAME_BUFFER_SIZE-1);
                HASH_ADD_STR(zyreAgents, peerId, zagent);
            }else{
                //Agent already exists, we set its reconnected flag
                //(this is used below to avoid agent destruction on EXIT received after timeout)
                zagent->reconnected++;
            }
            strncpy(zagent->name, name, NAME_BUFFER_SIZE-1);
            assert(headers);
            char *k;
            const char *v;
            zlist_t *keys = zhash_keys(headers);
            size_t s = zlist_size(keys);
            if (s > 0){
                igs_debug("Handling headers for agent %s", name);
            }
            while ((k = (char *)zlist_pop(keys))) {
                v = zyre_event_header (zyre_event,k);
                igs_debug("\t%s -> %s", k, v);

                // we extract the publisher adress to subscribe to from the zyre message header
                if(strncmp(k,"publisher", strlen("publisher")) == 0)
                {
                    char endpointAddress[128];
                    strncpy(endpointAddress, address, 127);

                    // IP adress extraction 
                    char *insert = endpointAddress + strlen(endpointAddress);
                    bool extractOK = true;
                    while (*insert != ':'){
                        insert--;
                        if (insert == endpointAddress){
                            igs_error("Could not extract port from address %s", address);
                            extractOK = false;
                            break;
                        }
                    }

                    if (extractOK){
                        //we found a possible publisher to subscribe to
                        *(insert + 1) = '\0'; //close endpointAddress string after ':' location
                        
                         //check towards our own ip address (without port)
                        char *incomingIpAddress = endpointAddress + 6; //ignore tcp://
#if defined __unix__ || defined __APPLE__ || defined __linux__
                        *insert = '\0';
                        bool useIPC = false;
                        const char *ipcAddress = NULL;
                        if (strcmp(agentElements->ipAddress, incomingIpAddress) == 0){
                            //same IP address : we can try to use ipc instead of TCP
                            //try to recover agent ipc address
                            ipcAddress = zyre_event_header(zyre_event, "ipc");
                            if (ipcAddress != NULL){
                                useIPC = true;
                                igs_debug("Use ipc address (%s) to subscribe to %s", ipcAddress, name);
                            }
                        }
                        *insert = ':';
#endif
                        //add port to the endpoint to compose it fully
                        strcat(endpointAddress, v);
                        subscriber_t *subscriber;
                        HASH_FIND_STR(subscribers, peer, subscriber);
                        if (subscriber != NULL){
                            //we have a reconnection with same peerId
                            igs_debug("Peer id %s was connected before with agent name %s : reset and recreate subscription", peer, subscriber->agentName);
                            HASH_DEL(subscribers, subscriber);
                            zloop_poller_end(agentElements->loop , subscriber->pollItem);
                            zsock_destroy(&subscriber->subscriber);
                            free((char*)subscriber->agentName);
                            free((char*)subscriber->agentPeerId);
                            free(subscriber->pollItem);
                            free(subscriber->subscriber);
                            if (subscriber->definition != NULL){
                                definition_freeDefinition(subscriber->definition);
                                subscriber->definition = NULL;
                            }
                            subscriber->subscriber = NULL;
                            if (subscriber->timerId != -1){
                                zloop_timer_end(agentElements->loop, subscriber->timerId);
                                subscriber->timerId = -1;
                            }
                            free(subscriber);
                            subscriber = NULL;
                        }
                        subscriber = calloc(1, sizeof(subscriber_t));
                        zagent->subscriber = subscriber;
                        subscriber->agentName = strdup(name);
                        subscriber->agentPeerId = strdup (peer);
#if defined __unix__ || defined __APPLE__ || defined __linux__
                        if (allowIpc && useIPC){
                            subscriber->subscriber = zsock_new_sub(ipcAddress, NULL);
                        }else{
                            subscriber->subscriber = zsock_new_sub(endpointAddress, NULL);
                        }
#else
                        subscriber->subscriber = zsock_new_sub(endpointAddress, NULL);
#endif
                        assert(subscriber->subscriber);
                        subscriber->definition = NULL;
                        subscriber->mappingsFilters = NULL;
                        subscriber->timerId = -1;
                        HASH_ADD_STR(subscribers, agentPeerId, subscriber);
                        subscriber->pollItem = calloc(1, sizeof(zmq_pollitem_t));
                        subscriber->pollItem->socket = zsock_resolve(subscriber->subscriber);
                        subscriber->pollItem->fd = 0;
                        subscriber->pollItem->events = ZMQ_POLLIN;
                        subscriber->pollItem->revents = 0;
                        zloop_poller (agentElements->loop, subscriber->pollItem, manageSubscription, (void*)subscriber->agentPeerId);
                        zloop_poller_set_tolerant(loop, subscriber->pollItem);
#if defined __unix__ || defined __APPLE__ || defined __linux__
                        if (useIPC){
                            igs_debug("Subscription created for %s at %s",subscriber->agentName,ipcAddress);
                        }else{
                            igs_debug("Subscription created for %s at %s",subscriber->agentName,endpointAddress);
                        }
#else
                        igs_debug("Subscription created for %s at %s",subscriber->agentName,endpointAddress);
#endif
                    }
                }
                free(k);
            }
            zlist_destroy(&keys);
        } else if (streq (event, "JOIN")){
            igs_debug("+%s has joined %s", name, group);
            if (streq(group, CHANNEL)){
                //definition is sent to every newcomer on the channel (whether it is a ingescape agent or not)
                char * definitionStr = NULL;
                definitionStr = parser_export_definition(igs_internal_definition);
                if (definitionStr != NULL){
                    sendDefinitionToAgent(peer, definitionStr);
                    free(definitionStr);
                }
                //and so is our mapping
                char *mappingStr = NULL;
                mappingStr = parser_export_mapping(igs_internal_mapping);
                if (mappingStr != NULL){
                    sendMappingToAgent(peer, mappingStr);
                    free(mappingStr);
                }else{
                    sendMappingToAgent(peer, "");
                }
                //we also send our frozen and muted states, and other usefull information
                if (isWholeAgentMuted){
                    zyre_whispers(agentElements->node, peer, "MUTED=1");
                }
                if (isFrozen){
                    zyre_whispers(agentElements->node, peer, "FROZEN=1");
                }
                if (strlen(igsAgentState) > 0){
                    zyre_whispers(agentElements->node, peer, "STATE=%s", igsAgentState);
                }
                if (igs_internal_definition != NULL){
                    agent_iop_t *current_iop, *tmp_iop;
                    HASH_ITER(hh, igs_internal_definition->outputs_table, current_iop, tmp_iop) {
                        if (current_iop->is_muted && current_iop->name != NULL){
                            zyre_whispers(agentElements->node, peer, "OUTPUT_MUTED %s", current_iop->name);
                        }
                    }
                }
                if (admin_logInStream){
                    zyre_whispers(agentElements->node, peer, "LOG_IN_STREAM=1");
                }
                if (admin_logInFile && strlen(admin_logFile) > 0){
                    zyre_whispers(agentElements->node, peer, "LOG_IN_FILE=1");
                    zyre_whispers(agentElements->node, peer, "LOG_FILE_PATH=%s", admin_logFile);
                }
                if (strlen(definitionPath) > 0){
                    zyre_whispers(agentElements->node, peer, "DEFINITION_FILE_PATH=%s", definitionPath);
                }
                if (strlen(mappingPath) > 0){
                    zyre_whispers(agentElements->node, peer, "MAPPING_FILE_PATH=%s", mappingPath);
                }
                
                zyreAgent_t *zagent = NULL;
                HASH_FIND_STR(zyreAgents, peer, zagent);
                if (zagent != NULL){
                    zagent->hasJoinedPrivateChannel = true;
                }
            }
        } else if (streq (event, "LEAVE")){
            igs_debug("-%s has left %s", name, group);
        } else if (streq (event, "SHOUT")){
            if (strcmp(group, replayChannel) == 0){
                //this is a replay message for one of our inputs
                char *input = zmsg_popstr (msgDuplicate);
                iopType_t inputType = igs_getTypeForInput(input);
                
                if (inputType > 0 && zmsg_size(msgDuplicate) > 0){
                    zframe_t *frame = NULL;
                    void *data = NULL;
                    size_t size = 0;
                    if (inputType == IGS_STRING_T){
                        char * value = NULL;
                        value = zmsg_popstr(msgDuplicate);
                        igs_debug("replaying %s (%s)", input, value);
                        igs_writeInputAsString(input, value);
                        if (value != NULL){
                            free(value);
                        }
                    }else{
                        igs_debug("replaying %s", input);
                        frame = zmsg_pop(msgDuplicate);
                        data = zframe_data(frame);
                        size = zframe_size(frame);
                        model_writeIOP(input, IGS_INPUT_T, inputType, data, size);
                        if (frame != NULL){
                            zframe_destroy(&frame);
                        }
                    }
                }else{
                    igs_warn("replay message for input %s is not correct and was ignored", input);
                }
                if (input != NULL){
                    free(input);
                }
            }
        } else if(streq (event, "WHISPER")){
            char *message = zmsg_popstr (msgDuplicate);
            
            //check if message is an EXTERNAL definition
            if(strlen(message) > strlen(definitionPrefix) && strncmp (message, definitionPrefix, strlen(definitionPrefix)) == 0)
            {
                // Extract definition from message
                char* strDefinition = calloc(strlen(message)- strlen(definitionPrefix)+1, sizeof(char));
                memcpy(strDefinition, &message[strlen(definitionPrefix)], strlen(message)- strlen(definitionPrefix));
                strDefinition[strlen(message)- strlen(definitionPrefix)] = '\0';
                
                // Load definition from string content
                definition *newDefinition = parser_loadDefinition(strDefinition);
                subscriber_t *subscriber;
                HASH_FIND_STR(subscribers, peer, subscriber);
                
                if (newDefinition != NULL && newDefinition->name != NULL && subscriber != NULL){
                    // Look if this agent already has a definition
                    if(subscriber->definition != NULL) {
                        igs_debug("Definition already exists for agent %s : new definition will overwrite the previous one...", name);
                        definition_freeDefinition(subscriber->definition);
                        subscriber->definition = NULL;
                    }
                    
                    #if ENABLE_LICENSE_ENFORCEMENT
                    licEnforcement->currentIOPNb += (HASH_COUNT(newDefinition->inputs_table) +
                                                     HASH_COUNT(newDefinition->outputs_table) +
                                                     HASH_COUNT(newDefinition->params_table));
                    if (licEnforcement->currentIOPNb > license->platformNbIOPs){
                        igs_license("Maximum number of allowed IOPs (%d) is exceeded : agent will stop", license->platformNbIOPs);
                        license_callback_t *el = NULL;
                        DL_FOREACH(licenseCallbacks, el){
                            el->callback_ptr(el->data);
                        }
                        return -1;
                    }
                    licEnforcement->currentAgentsNb++;
                    if (licEnforcement->currentAgentsNb > license->platformNbAgents){
                        igs_license("Maximum number of allowed agents (%d) is exceeded : agent will stop", license->platformNbAgents);
                        license_callback_t *el = NULL;
                        DL_FOREACH(licenseCallbacks, el){
                            el->callback_ptr(el->data);
                        }
                        return -1;
                    }
                    #endif
                    igs_debug("Store definition for agent %s", name);
                    subscriber->definition = newDefinition;
                    //Check the involvement of this new agent and its definition in our mapping and update subscriptions
                    //we check here because subscriber definition is required to handle received data
                    network_manageSubscriberMapping(subscriber);
                }else{
                    igs_error("Received definition from agent %s is NULL or has no name", name);
                    if(newDefinition != NULL) {
                        definition_freeDefinition(newDefinition);
                        newDefinition = NULL;
                    }
                }
                free(strDefinition);
            }
            //check if message is an EXTERNAL mapping
            else if(strlen(message) >= strlen(mappingPrefix) && strncmp (message, mappingPrefix, strlen(mappingPrefix)) == 0){
                subscriber_t *subscriber = NULL;
                HASH_FIND_STR(subscribers, peer, subscriber);
                if (subscriber == NULL){
                    igs_error("Could not find internal structure for agent %s", name);
                }
                
                char* strMapping = NULL;
                mapping_t *newMapping = NULL;
                if (strlen(message) != strlen(mappingPrefix)){
                    //extract mapping from message
                    strMapping = calloc(strlen(message)- strlen(mappingPrefix)+1, sizeof(char));
                    memcpy(strMapping, &message[strlen(mappingPrefix)], strlen(message)- strlen(mappingPrefix));
                    strMapping[strlen(message)- strlen(mappingPrefix)] = '\0';
                    
                    //load mapping from string content
                    newMapping = parser_LoadMap(strMapping);
                    if (newMapping == NULL){
                        igs_error("Received mapping for agent %s could not be parsed properly", name);
                    }
                }else{
                    igs_debug("Received mapping from agent %s is empty", name);
                    if(subscriber != NULL && subscriber->mapping != NULL) {
                        mapping_freeMapping(subscriber->mapping);
                        subscriber->mapping = NULL;
                    }
                }
                
                if (newMapping != NULL && subscriber != NULL){
                    //look if this agent already has a mapping
                    if(subscriber->mapping != NULL){
                        igs_debug("Mapping already exists for agent %s : new mapping will overwrite the previous one...", name);
                        mapping_freeMapping(subscriber->mapping);
                        subscriber->mapping = NULL;
                    }
                    
                    igs_debug("Store mapping for agent %s", name);
                    subscriber->mapping = newMapping;
                }else{
                    if(newMapping != NULL) {
                        mapping_freeMapping(newMapping);
                        newMapping = NULL;
                    }
                }
                free(strMapping);
            }
            //check if message is DEFINITION TO BE LOADED
            else if (strlen(message) > strlen(loadDefinitionPrefix)
                     && strncmp (message, loadDefinitionPrefix, strlen(loadDefinitionPrefix)) == 0){
                // Extract definition from message
                char* strDefinition = calloc(strlen(message)- strlen(loadDefinitionPrefix)+1, sizeof(char));
                memcpy(strDefinition, &message[strlen(loadDefinitionPrefix)], strlen(message)- strlen(loadDefinitionPrefix));
                strDefinition[strlen(message)- strlen(loadDefinitionPrefix)] = '\0';
                
                //load definition
                igs_loadDefinition(strDefinition);
                //recheck mapping towards our new definition
                subscriber_t *s, *tmp;
                HASH_ITER(hh, subscribers, s, tmp){
                    network_manageSubscriberMapping(s);
                }
                free(strDefinition);
            }
            //check if message is MAPPING TO BE LOADED
            else if (strlen(message) > strlen(loadMappingPrefix)
                     && strncmp (message, loadMappingPrefix, strlen(loadMappingPrefix)) == 0){
                // Extract mapping from message
                char* strMapping = calloc(strlen(message)- strlen(loadMappingPrefix)+1, sizeof(char));
                memcpy(strMapping, &message[strlen(loadMappingPrefix)], strlen(message)- strlen(loadMappingPrefix));
                strMapping[strlen(message)- strlen(loadMappingPrefix)] = '\0';
                
                // Load mapping from string content
                mapping_t *m = parser_LoadMap(strMapping);
                if (m != NULL){
                    if (igs_internal_mapping != NULL){
                        mapping_freeMapping(igs_internal_mapping);
                    }
                    igs_internal_mapping = m;
                    //check and activate mapping
                    subscriber_t *s, *tmp;
                    HASH_ITER(hh, subscribers, s, tmp){
                        network_manageSubscriberMapping(s);
                    }
                    network_needToUpdateMapping = true;
                }
                free(strMapping);
            }else{
                //other supported messages
                if (strlen("REQUEST_OUPUTS") == strlen(message) && strncmp (message, "REQUEST_OUPUTS", strlen("REQUEST_OUPUTS")) == 0){
                    igs_debug("Responding to outputs request received from %s", name);
                    //send all outputs via whisper to agent that mapped us
                    long nbOutputs = 0;
                    char **outputsList = NULL;
                    outputsList = igs_getOutputsList(&nbOutputs);
                    int i = 0;
                    zmsg_t *omsg = zmsg_new();
                    zmsg_addstr(omsg, "OUTPUTS");
                    for (i = 0; i < nbOutputs; i++){
                        agent_iop_t * found_iop = model_findIopByName(outputsList[i],IGS_OUTPUT_T);
                        if (found_iop != NULL){
                            switch (found_iop->value_type) {
                                case IGS_INTEGER_T:
                                    zmsg_addstr(omsg, found_iop->name);
                                    zmsg_addstrf(omsg, "%d", found_iop->value_type);
                                    zmsg_addmem(omsg, &(found_iop->value.i), sizeof(int));
                                    break;
                                case IGS_DOUBLE_T:
                                    zmsg_addstr(omsg, found_iop->name);
                                    zmsg_addstrf(omsg, "%d", found_iop->value_type);
                                    zmsg_addmem(omsg, &(found_iop->value.d), sizeof(double));
                                    break;
                                case IGS_STRING_T:
                                    zmsg_addstr(omsg, found_iop->name);
                                    zmsg_addstrf(omsg, "%d", found_iop->value_type);
                                    zmsg_addstr(omsg, found_iop->value.s);
                                    break;
                                case IGS_BOOL_T:
                                    zmsg_addstr(omsg, found_iop->name);
                                    zmsg_addstrf(omsg, "%d", found_iop->value_type);
                                    zmsg_addmem(omsg, &(found_iop->value.b), sizeof(bool));
                                    break;
                                case IGS_IMPULSION_T:
                                    //disabled
//                                    zmsg_addstr(omsg, found_iop->name);
//                                    zmsg_addstrf(omsg, "%d", found_iop->value_type);
//                                    zmsg_addmem(omsg, NULL, 0);
                                    break;
                                case IGS_DATA_T:
                                    //disabled
//                                    zmsg_addstr(omsg, found_iop->name);
//                                    zmsg_addstrf(omsg, "%d", found_iop->value_type);
//                                    zmsg_addmem(omsg, (found_iop->value.data), found_iop->valueSize);
                                    break;
                                    
                                default:
                                    break;
                            }
                            
                            
                            free(outputsList[i]);
                        }
                    }
                    zyre_whisper(node, peer, &omsg);
                    free(outputsList);
                }else if (strlen("OUTPUTS") == strlen(message) && strncmp (message, "OUTPUTS", strlen("OUTPUTS")) == 0){
                    handleSubscriptionMessage(msgDuplicate, peer);
                    igs_debug("privately received output values from %s (%s)", name, peer);
                }else if (strlen("GET_CURRENT_INPUTS") == strlen(message) && strncmp (message, "GET_CURRENT_INPUTS", strlen("GET_CURRENT_INPUTS")) == 0){
                    zmsg_t *resp = zmsg_new();
                    agent_iop_t *inputs = igs_internal_definition->inputs_table;
                    agent_iop_t *current = NULL;
                    zmsg_addstr(resp, "CURRENT_INPUTS");
                    model_readWriteLock();
                    for (current = inputs; current != NULL; current = current->hh.next){
                        zmsg_addstr(resp, current->name);
                        zmsg_addmem(resp, &current->type, sizeof(iop_t));
                        zmsg_addmem(resp, &current->value, current->valueSize);
                    }
                    model_readWriteUnlock();
                    zyre_whisper(node, peer, &resp);
                    igs_debug("send input values to %s", peer);
                }else if (strlen("GET_CURRENT_PARAMETERS") == strlen(message) && strncmp (message, "GET_CURRENT_PARAMETERS", strlen("GET_CURRENT_PARAMETERS")) == 0){
                    zmsg_t *resp = zmsg_new();
                    agent_iop_t *inputs = igs_internal_definition->params_table;
                    agent_iop_t *current = NULL;
                    zmsg_addstr(resp, "CURRENT_PARAMETERS");
                    model_readWriteLock();
                    for (current = inputs; current != NULL; current = current->hh.next){
                        zmsg_addstr(resp, current->name);
                        zmsg_addmem(resp, &current->type, sizeof(iop_t));
                        zmsg_addmem(resp, &current->value, current->valueSize);
                    }
                    model_readWriteUnlock();
                    zyre_whisper(node, peer, &resp);
                    igs_debug("send parameters values to %s", peer);
                }else if (strlen("LICENSE_INFO") == strlen(message) && strncmp (message, "LICENSE_INFO", strlen("LICENSE_INFO")) == 0){
#if !TARGET_OS_IOS
                    zmsg_t *resp = zmsg_new();
                    if (license == NULL){
                        zmsg_addstr(resp, "no license available");
                    }else{
                        char buf[128] = "";
                        struct tm ts;
                        zmsg_addstr(resp, "customer");
                        zmsg_addstr(resp, license->customer);
                        zmsg_addstr(resp, "order");
                        zmsg_addstr(resp, license->order);
                        zmsg_addstr(resp, "platformNbAgents");
                        zmsg_addstrf(resp, "%d", license->platformNbAgents);
                        zmsg_addstr(resp, "platformNbIOPs");
                        zmsg_addstrf(resp, "%d", license->platformNbIOPs);
                        zmsg_addstr(resp, "licenseExpirationDate");
                        ts = *localtime(&license->licenseExpirationDate);
                        strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
                        zmsg_addstr(resp, buf);
                        zmsg_addstr(resp, "editorOwner");
                        zmsg_addstr(resp, license->editorOwner);
                        zmsg_addstr(resp, "editorExpirationDate");
                        ts = *localtime(&license->editorExpirationDate);
                        strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
                        zmsg_addstr(resp, buf);
                        char *feature = zlist_first(license->features);
                        while (feature != NULL) {
                            zmsg_addstr(resp, "feature");
                            zmsg_addstr(resp, feature);
                            feature = zlist_next(license->features);
                        }
                        licenseForAgent_t *agent = zlist_first(license->agents);
                        while (agent != NULL) {
                            zmsg_addstr(resp, "agent");
                            zmsg_addstr(resp, agent->agentId);
                            zmsg_addstr(resp, agent->agentName);
                            agent = zlist_next(license->agents);
                        }
                    }
                    zyre_whisper(node, peer, &resp);
                    igs_debug("send license information to %s", peer);
#endif
                }else if (strlen("STOP") == strlen(message) && strncmp (message, "STOP", strlen("STOP")) == 0){
                    free(message);
                    forcedStop = true;
                    igs_debug("received STOP command from %s (%s)", name, peer);
                    //stop our zyre loop by returning -1 : this will start the cleaning process
                    return -1;
                }else if (strlen("CLEAR_MAPPING") == strlen(message) && strncmp (message, "CLEAR_MAPPING", strlen("CLEAR_MAPPING")) == 0){
                    igs_debug("received CLEAR_MAPPING command from %s (%s)", name, peer);
                    igs_clearMapping();
                }else if (strlen("FREEZE") == strlen(message) && strncmp (message, "FREEZE", strlen("FREEZE")) == 0){
                    igs_debug("received FREEZE command from %s (%s)", name, peer);
                    igs_freeze();
                }else if (strlen("UNFREEZE") == strlen(message) && strncmp (message, "UNFREEZE", strlen("UNFREEZE")) == 0){
                    igs_debug("received UNFREEZE command from %s (%s)", name, peer);
                    igs_unfreeze();
                }else if (strlen("MUTE_ALL") == strlen(message) && strncmp (message, "MUTE_ALL", strlen("MUTE_ALL")) == 0){
                    igs_debug("received MUTE_ALL command from %s (%s)", name, peer);
                    igs_mute();
                }else if (strlen("UNMUTE_ALL") == strlen(message) && strncmp (message, "UNMUTE_ALL", strlen("UNMUTE_ALL")) == 0){
                    igs_debug("received UNMUTE_ALL command from %s (%s)", name, peer);
                    igs_unmute();
                }else if ((strncmp (message, "MUTE ", strlen("MUTE ")) == 0) && (strlen(message) > strlen("MUTE ")+1)){
                    igs_debug("received MUTE command from %s (%s)", name, peer);
                    char *subStr = message + strlen("MUTE") + 1;
                    igs_muteOutput(subStr);
                }else if ((strncmp (message, "UNMUTE ", strlen("UNMUTE ")) == 0) && (strlen(message) > strlen("UNMUTE ")+1)){
                    igs_debug("received UNMUTE command from %s (%s)", name, peer);
                    char *subStr = message + strlen("UNMUTE") + 1;
                    igs_unmuteOutput(subStr);
                }else if ((strncmp (message, "SET_INPUT ", strlen("SET_INPUT ")) == 0) && (strlen(message) > strlen("SET_INPUT ")+1)){
                    igs_debug("received SET_INPUT command from %s (%s)", name, peer);
                    char *subStr = message + strlen("SET_INPUT") + 1;
                    char *_name, *value;
                    _name = strtok (subStr," ");
                    value = strtok (NULL,"\0");
                    if (_name != NULL && value != NULL){
                        igs_writeInputAsString(_name, value);//last parameter is used for DATA only
                    }
                }else if ((strncmp (message, "SET_OUTPUT ", strlen("SET_OUTPUT ")) == 0) && (strlen(message) > strlen("SET_OUTPUT ")+1)){
                    igs_debug("received SET_OUTPUT command from %s (%s)", name, peer);
                    char *subStr = message + strlen("SET_OUTPUT") + 1;
                    char *_name, *value;
                    _name = strtok (subStr," ");
                    value = strtok (NULL,"\0");
                    if (_name != NULL && value != NULL){
                        igs_writeOutputAsString(_name, value);//last paramter is used for DATA only
                    }
                }else if ((strncmp (message, "SET_PARAMETER ", strlen("SET_PARAMETER ")) == 0) && (strlen(message) > strlen("SET_PARAMETER ")+1)){
                    igs_debug("received SET_PARAMETER command from %s (%s)", name, peer);
                    char *subStr = message + strlen("SET_PARAMETER") + 1;
                    char *_name, *value;
                    _name = strtok (subStr," ");
                    value = strtok (NULL,"\0");
                    if (_name != NULL && value != NULL){
                        igs_writeParameterAsString(_name, value);//last paramter is used for DATA only
                    }
                }else if ((strncmp (message, "MAP ", strlen("MAP ")) == 0) && (strlen(message) > strlen("MAP ")+1)){
                    igs_debug("received MAP command from %s (%s)", name, peer);
                    char *subStr = message + strlen("MAP") + 1;
                    char *input, *agent, *output;
                    input = strtok (subStr," ");
                    agent = strtok (NULL," ");
                    output = strtok (NULL," ");
                    if (input != NULL && agent != NULL && output != NULL){
                        igs_addMappingEntry(input, agent, output);
                    }
                }else if ((strncmp (message, "UNMAP ", strlen("UNMAP ")) == 0) && (strlen(message) > strlen("UNMAP ")+1)){
                    igs_debug("received UNMAP command from %s (%s)", name, peer);
                    char *subStr = message + strlen("UNMAP") + 1;
                    char *input, *agent, *output;
                    input = strtok (subStr," ");
                    agent = strtok (NULL," ");
                    output = strtok (NULL," ");
                    if (input != NULL && agent != NULL && output != NULL){
                        igs_removeMappingEntryWithName(input, agent, output);
                    }
                }
                //admin API
                else if (strlen("ENABLE_LOG_STREAM") == strlen(message) && strncmp (message, "ENABLE_LOG_STREAM", strlen("ENABLE_LOG_STREAM")) == 0){
                    igs_debug("received ENABLE_LOG_STREAM command from %s (%s)", name, peer);
                    igs_setLogStream(true);
                }
                else if (strlen("DISABLE_LOG_STREAM") == strlen(message) && strncmp (message, "DISABLE_LOG_STREAM", strlen("DISABLE_LOG_STREAM")) == 0){
                    igs_debug("received DISABLE_LOG_STREAM command from %s (%s)", name, peer);
                    igs_setLogStream(false);
                }
                else if (strlen("ENABLE_LOG_FILE") == strlen(message) && strncmp (message, "ENABLE_LOG_FILE", strlen("ENABLE_LOG_FILE")) == 0){
                    igs_debug("received ENABLE_LOG_FILE command from %s (%s)", name, peer);
                    igs_setLogInFile(true);
                }
                else if (strlen("DISABLE_LOG_FILE") == strlen(message) && strncmp (message, "DISABLE_LOG_FILE", strlen("DISABLE_LOG_FILE")) == 0){
                    igs_debug("received DISABLE_LOG_FILE command from %s (%s)", name, peer);
                    igs_setLogInFile(false);
                }
                else if ((strncmp (message, "SET_LOG_PATH ", strlen("SET_LOG_PATH ")) == 0) && (strlen(message) > strlen("SET_LOG_PATH ")+1)){
                    igs_debug("received SET_LOG_PATH command from %s (%s)", name, peer);
                    char *subStr = message + strlen("SET_LOG_PATH") + 1;
                    igs_setLogPath(subStr);
                }
                else if ((strncmp (message, "SET_DEFINITION_PATH ", strlen("SET_DEFINITION_PATH ")) == 0)
                         && (strlen(message) > strlen("SET_DEFINITION_PATH ")+1)){
                    igs_debug("received SET_DEFINITION_PATH command from %s (%s)", name, peer);
                    char *subStr = message + strlen("SET_DEFINITION_PATH") + 1;
                    igs_setDefinitionPath(subStr);
                }
                else if ((strncmp (message, "SET_MAPPING_PATH ", strlen("SET_MAPPING_PATH ")) == 0)
                         && (strlen(message) > strlen("SET_MAPPING_PATH ")+1)){
                    igs_debug("received SET_MAPPING_PATH command from %s (%s)", name, peer);
                    char *subStr = message + strlen("SET_MAPPING_PATH") + 1;
                    igs_setMappingPath(subStr);
                }
                else if (strlen("SAVE_DEFINITION_TO_PATH") == strlen(message) && strncmp (message, "SAVE_DEFINITION_TO_PATH", strlen("SAVE_DEFINITION_TO_PATH")) == 0){
                    igs_debug("received SAVE_DEFINITION_TO_PATH command from %s (%s)", name, peer);
                    igs_writeDefinitionToPath();
                }
                else if (strlen("SAVE_MAPPING_TO_PATH") == strlen(message) && strncmp (message, "SAVE_MAPPING_TO_PATH", strlen("SAVE_MAPPING_TO_PATH")) == 0){
                    igs_debug("received SAVE_MAPPING_TO_PATH command from %s (%s)", name, peer);
                    igs_writeMappingToPath();
                }
                //TOKENS
                else if (strcmp (message, "TOKEN") == 0){
                    char *tokenName = zmsg_popstr(msgDuplicate);
                    igs_token_t *token = NULL;
                    if (igs_internal_definition != NULL && igs_internal_definition->tokens_table != NULL){
                        HASH_FIND_STR(igs_internal_definition->tokens_table, tokenName, token);
                        if (token != NULL ){
                            if (token->cb != NULL){
                                size_t nbArgs = 0;
                                igs_tokenArgument_t *_arg = NULL;
                                LL_COUNT(token->arguments, _arg, nbArgs);
                                if (token_addValuesToArgumentsFromMessage(tokenName, token->arguments, msgDuplicate)){
                                    (token->cb)(name, peer, tokenName, token->arguments, nbArgs, token->cbData);
                                    token_freeValuesInArguments(token->arguments);
                                }
                            }else{
                                igs_warn("no defined callback to handle received token %s", tokenName);
                            }
                        }else{
                            igs_warn("agent %s has no token named %s", name, tokenName);
                        }
                    }
                }
            }
            free(message);
        } else if (streq (event, "EXIT")){
            igs_debug("<-%s (%s) exited", name, peer);
            zyreAgent_t *a = NULL;
            HASH_FIND_STR(zyreAgents, peer, a);
            if (a != NULL){
                if (a->reconnected > 0){
                    //do not clean: we are getting a timemout now whereas
                    //the agent is reconnected
                    a->reconnected--;
                }else{
                    HASH_DEL(zyreAgents, a);
                    free(a);
                    // Try to find the subscriber to destory
                    subscriber_t *subscriber = NULL;
                    HASH_FIND_STR(subscribers, peer, subscriber);
                    if (subscriber != NULL){
                        network_cleanAndFreeSubscriber(subscriber);
                    }
                }
            }
        }
        
        //handle callbacks
        zyreCallback_t *elt;
        DL_FOREACH(zyreCallbacks,elt){
            if (zyre_event != NULL){
                zmsg_t *dup = zmsg_dup(msg);
                elt->callback_ptr(event, peer, name, address, group, headers, dup, elt->myData);
                zmsg_destroy(&dup);
            }else{
                igs_error("previous callback certainly destroyed the bus event : next callbacks will not be executed");
                break;
            }
        }
        zmsg_destroy(&msgDuplicate);
        zyre_event_destroy(&zyre_event);
    }
    return 0;
}

//Timer callback to (re)send our definition to agents present on the private channel
int triggerDefinitionUpdate(zloop_t *loop, int timer_id, void *arg){
    IGS_UNUSED(loop)
    IGS_UNUSED(timer_id)
    IGS_UNUSED(arg)

    if (network_needToSendDefinitionUpdate){
        model_readWriteLock();
        char * definitionStr = NULL;
        definitionStr = parser_export_definition(igs_internal_definition);
        if (definitionStr != NULL){
            zyreAgent_t *a, *tmp;
            HASH_ITER(hh, zyreAgents, a, tmp){
                if (a->hasJoinedPrivateChannel){
                    sendDefinitionToAgent(a->peerId, definitionStr);
                }
            }
            free(definitionStr);
        }
        network_needToSendDefinitionUpdate = false;
        //when definition changes, mapping may need to be updated as well
        network_needToUpdateMapping = true;
        model_readWriteUnlock();
    }
    return 0;
}

//Timer callback to stop loop if license is expired or demo mode is on
int triggerLicenseStop(zloop_t *loop, int timer_id, void *arg){
    IGS_UNUSED(loop);
    IGS_UNUSED(timer_id);
    IGS_UNUSED(arg);
    igs_license("License has expired and runtime duration limit is reached : stopping loop.");
    license_callback_t *el = NULL;
    DL_FOREACH(licenseCallbacks, el){
        el->callback_ptr(el->data);
    }
    return -1;
}

//Timer callback to update and (re)send our mapping to agents on the private channel
int triggerMappingUpdate(zloop_t *loop, int timer_id, void *arg){
    IGS_UNUSED(loop)
    IGS_UNUSED(timer_id)
    IGS_UNUSED(arg)

    if (network_needToUpdateMapping){
        char *mappingStr = NULL;
        mappingStr = parser_export_mapping(igs_internal_mapping);
        if (mappingStr != NULL){
            zyreAgent_t *a, *tmp;
            HASH_ITER(hh, zyreAgents, a, tmp){
                if (a->hasJoinedPrivateChannel){
                    sendMappingToAgent(a->peerId, mappingStr);
                }
            }
            free(mappingStr);
        }
        subscriber_t *s, *tmp;
        HASH_ITER(hh, subscribers, s, tmp){
            network_manageSubscriberMapping(s);
        }
        network_needToUpdateMapping = false;
    }
    return 0;
}

static void
initLoop (zsock_t *pipe, void *args){
    IGS_UNUSED(args)
    
#if ENABLE_LICENSE_ENFORCEMENT
    if (licEnforcement == NULL){
        licEnforcement = calloc(1, sizeof(licenseEnforcement_t));
    }
    licEnforcement->currentAgentsNb = 0;
    licEnforcement->currentIOPNb = 0;
#endif

    network_needToSendDefinitionUpdate = false;
    network_needToUpdateMapping = false;

    bool canContinue = true;
    //prepare zyre
    agentElements->node = zyre_new (igsAgentName);
//    zyre_set_verbose(agentElements->node);
    if (strlen(agentElements->brokerEndPoint) > 0){
        zyre_gossip_connect(agentElements->node,
                            "%s", agentElements->brokerEndPoint);
    }else{
        if (agentElements->node == NULL){
            igs_error("Could not create bus node : Agent will interrupt immediately.");
            canContinue = false;
        }else{
            zyre_set_interface(agentElements->node, agentElements->networkDevice);
            zyre_set_port(agentElements->node, agentElements->zyrePort);
        }
    }
    zyre_set_interval(agentElements->node, network_discoveryInterval);
    zyre_set_expired_timeout(agentElements->node, network_agentTimeout);
    
    //create channel for replay
    snprintf(replayChannel, MAX_AGENT_NAME_LENGTH + 15, "%s-IGS-REPLAY", igsAgentName);
    zyre_join(agentElements->node, replayChannel);
    
    //Add stored headers to zyre
    if (canContinue){
        serviceHeader_t *el, *tmp;
        HASH_ITER(hh, serviceHeaders, el, tmp){
            zyre_set_header(agentElements->node, el->key, "%s", el->value);
        }
    }
    
    //start TCP publisher
    char endpoint[512];
    if (network_publishingPort == 0){
        snprintf(endpoint, 511, "tcp://%s:*", agentElements->ipAddress);
    }else{
        snprintf(endpoint, 511, "tcp://%s:%d", agentElements->ipAddress, network_publishingPort);
    }
    agentElements->publisher = zsock_new_pub(endpoint);
    if (agentElements->publisher == NULL){
        igs_error("Could not create publishing socket (%s): Agent will interrupt immediately.", endpoint);
        canContinue = false;
    }else{
        strncpy(endpoint, zsock_endpoint(agentElements->publisher), 256);
        char *insert = endpoint + strlen(endpoint) - 1;
        while (*insert != ':' && insert > endpoint) {
            insert--;
        }
        zyre_set_header(agentElements->node, "publisher", "%s", insert + 1);
    }
    
#if defined __unix__ || defined __APPLE__ || defined __linux__
    //start ipc publisher
    char *ipcEndpoint = NULL;
    char *ipcFullPath = NULL;
    if (ipcFolderPath == NULL){
        ipcFolderPath = strdup(DEFAULT_IPC_PATH);
    }
    struct stat st;
    if(stat(ipcFolderPath,&st) != 0){
        igs_warn("Expected IPC directory %s does not exist : create it to remove this warning", ipcFolderPath);
    }
    ipcFullPath = calloc(1, strlen(ipcFolderPath)+strlen(zyre_uuid(agentElements->node))+2);
    sprintf(ipcFullPath, "%s/%s", ipcFolderPath, zyre_uuid(agentElements->node));
    ipcEndpoint = calloc(1, strlen(ipcFolderPath)+strlen(zyre_uuid(agentElements->node))+8);
    sprintf(ipcEndpoint, "ipc://%s/%s", ipcFolderPath, zyre_uuid(agentElements->node));
    
    zsock_t *ipcPublisher = agentElements->ipcPublisher = zsock_new_pub(ipcEndpoint);
    if (ipcPublisher == NULL){
        igs_warn("Could not create IPC publishing socket (%s)", ipcEndpoint);
    }else{
        zyre_set_header(agentElements->node, "ipc", "%s", ipcEndpoint);
    }
    
#endif
    
    //start logger stream if needed
    if (admin_logInStream){
        sprintf(endpoint, "tcp://%s:*", agentElements->ipAddress);
        agentElements->logger = zsock_new_pub(endpoint);
        strncpy(endpoint, zsock_endpoint(agentElements->logger), 256);
        char *insert = endpoint + strlen(endpoint) - 1;
        while (*insert != ':' && insert > endpoint) {
            insert--;
        }
        zyre_set_header(agentElements->node, "logger", "%s", insert + 1);
    }
    
    //set other headers for agent
    zyre_set_header(agentElements->node, "canBeFrozen", "%i", agentCanBeFrozen);

#if defined __unix__ || defined __APPLE__ || defined __linux__
    ssize_t ret;
    pid_t pid;
    pid = getpid();
    zyre_set_header(agentElements->node, "pid", "%i", pid);
    
    if (strlen(commandLine) == 0){
        //command line was not set manually : we try to get exec path instead
#ifdef __APPLE__
#if TARGET_OS_IOS
        char pathbuf[64] = "no_path";
        ret = 1;
#elif TARGET_OS_OSX
        char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
        ret = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
#endif
#else
        char pathbuf[4*1024];
        memset(pathbuf, 0, 4*1024);
        ret = readlink("/proc/self/exe", pathbuf, sizeof(pathbuf));
#endif
        if ( ret <= 0 ) {
            igs_error("PID %d: proc_pidpath () - %s", pid, strerror(errno));
        } else {
            igs_trace("proc %d: %s", pid, pathbuf);
        }
        zyre_set_header(agentElements->node, "commandline", "%s", pathbuf);
    }else{
        zyre_set_header(agentElements->node, "commandline", "%s", commandLine);
    }
#endif
    
#if (defined WIN32 || defined _WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    if (strlen(commandLine) == 0){
        //command line was not set manually : we try to get exec path instead

        //Use GetModuleFileName() to get exec path
        // See comment on define UNICODE in the top of this file, without define UNICODE This lines return NULL String
        WCHAR temp[MAX_PATH];
        GetModuleFileName(NULL,temp,MAX_PATH);
        
        //Conversion in char *
        char exeFilePath[MAX_PATH];
        wcstombs_s(NULL,exeFilePath,sizeof(exeFilePath),temp,sizeof(temp));
        zyre_set_header(agentElements->node, "commandline", "%s", exeFilePath);
    }else{
        zyre_set_header(agentElements->node, "commandline", "%s", commandLine);
    }
    DWORD pid = GetCurrentProcessId();
    zyre_set_header(agentElements->node, "pid", "%i", (int)pid);
#endif


    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    #if (defined WIN32 || defined _WIN32)
        WSACleanup();
    #endif
    zyre_set_header(agentElements->node, "hostname", "%s", hostname);
    //code for Fully Qualified Domain Name if needed someday
//    struct addrinfo hints, *info, *p;
//    int gai_result;
//    memset(&hints, 0, sizeof hints);
//    hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_flags = AI_CANONNAME;
//    if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
//        igs_debug("getaddrinfo error: %s\n", gai_strerror(gai_result));
//        exit(1);
//    }
//    for(p = info; p != NULL; p = p->ai_next) {
//        igs_debug("hostname: %s\n", p->ai_canonname);
//    }
//    freeaddrinfo(info);
    
    //finally start zyre now that everything is set
    if (zyre_start (agentElements->node) == -1){
        igs_error("Could not start bus node : Agent will interrupt immediately.");
        canContinue = false;
    }
    zyre_join(agentElements->node, CHANNEL);
    zsock_signal (pipe, 0); //notify main thread that we are ready
    
    zmq_pollitem_t zpipePollItem;
    zmq_pollitem_t zyrePollItem;

    //main zmq socket (i.e. main thread)
    void *zpipe = zsock_resolve(pipe);
    if (zpipe == NULL){
        igs_error("Could not get the pipe descriptor to the main thread for polling : Agent will interrupt immediately.");
        canContinue = false;
    }
    zpipePollItem.socket = zpipe;
    zpipePollItem.fd = 0;
    zpipePollItem.events = ZMQ_POLLIN;
    zpipePollItem.revents = 0;

    //zyre socket
    void *zsock = zsock_resolve(zyre_socket (agentElements->node));
    if (zsock == NULL){
        igs_error("Could not get the bus socket for polling : Agent will interrupt immediately.");
        canContinue = false;
    }
    zyrePollItem.socket = zsock;
    zyrePollItem.fd = 0;
    zyrePollItem.events = ZMQ_POLLIN;
    zyrePollItem.revents = 0;

    agentElements->loop = zloop_new ();
    assert (agentElements->loop);
    zloop_set_verbose (agentElements->loop, false);

    zloop_poller (agentElements->loop, &zpipePollItem, manageParent, agentElements);
    zloop_poller_set_tolerant(agentElements->loop, &zpipePollItem);
    zloop_poller (agentElements->loop, &zyrePollItem, manageBusIncoming, agentElements);
    zloop_poller_set_tolerant(agentElements->loop, &zyrePollItem);
    
    zloop_timer(agentElements->loop, 1000, 0, triggerDefinitionUpdate, NULL);
    zloop_timer(agentElements->loop, 1000, 0, triggerMappingUpdate, NULL);
    
#if ENABLE_LICENSE_ENFORCEMENT
    if (license != NULL && license->isLicenseExpired){
        igs_license("License is not valid : starting timer for demonstration mode (%d seconds)...", MAX_EXEC_DURATION_DURING_EVAL);
        zloop_timer(agentElements->loop, MAX_EXEC_DURATION_DURING_EVAL * 1000, 0, triggerLicenseStop, NULL);
    }
#endif

    if (canContinue){
        zloop_start (agentElements->loop); //start returns when one of the pollers returns -1
    }
    
    igs_debug("loop stopping...");

    //clean
    zyreAgent_t *zagent, *tmpa;
    HASH_ITER(hh, zyreAgents, zagent, tmpa){
        HASH_DEL(zyreAgents, zagent);
        free(zagent);
    }
    subscriber_t *s, *tmps;
    HASH_ITER(hh, subscribers, s, tmps) {
        network_cleanAndFreeSubscriber(s);
    }
    zyre_stop (agentElements->node);
    zclock_sleep (100);
    zyre_destroy (&agentElements->node);
    zsock_destroy(&agentElements->publisher);
#if defined __unix__ || defined __APPLE__ || defined __linux__
    if (agentElements->ipcPublisher != NULL){
        zsock_destroy(&agentElements->ipcPublisher);
        zsys_file_delete(ipcFullPath); //destroy ipcPath in file system
        //NB: ipcPath is based on peer id which is unique. It will never be used again.
    }
    free(ipcFullPath);
    free(ipcEndpoint);
#endif
    if (agentElements->logger != NULL){
        zsock_destroy(&agentElements->logger);
    }
    
    zloop_destroy (&agentElements->loop);
    assert (agentElements->loop == NULL);
    //call registered interruption callbacks
    forcedStopCalback_t *cb = NULL;
    if (forcedStop){
        DL_FOREACH(forcedStopCalbacks, cb){
            cb->callback_ptr(cb->myData);
        }
    }
    igs_debug("loop stopped");
    if (forcedStop){
        igs_Interrupted = true;
        //in case of forced stop, we send SIGINT to our process so
        //that it can be trapped by main thread for a proper stop
        #if defined __unix__ || defined __APPLE__ || defined __linux__
        igs_debug("triggering SIGINT");
        kill(pid, SIGINT);
        #endif
        //TODO : do that for windows also
    }
#if ENABLE_LICENSE_ENFORCEMENT
    if (licEnforcement != NULL){
        free(licEnforcement);
    }
#endif
}

////////////////////////////////////////////////////////////////////////
// PRIVATE API
////////////////////////////////////////////////////////////////////////

int network_publishOutput (const agent_iop_t *iop){
    int result = 0;
    if (iop == NULL){
        igs_warn("passed IOP is NULL");
        return 0;
    }
    
    model_readWriteLock();
    if(agentElements != NULL && agentElements->publisher != NULL && iop != NULL)
    {
        if(!isWholeAgentMuted && !iop->is_muted && iop->name != NULL && !isFrozen)
        {
            zmsg_t *msg = zmsg_new();
            zmsg_addstr(msg, iop->name);
            zmsg_addstrf(msg, "%d", iop->value_type);
            switch (iop->value_type) {
                case IGS_INTEGER_T:
                    zmsg_addmem(msg, &(iop->value.i), sizeof(int));
                    igs_debug("publish %s -> %d",iop->name,iop->value.i);
                    break;
                case IGS_DOUBLE_T:
                    zmsg_addmem(msg, &(iop->value.d), sizeof(double));
                    igs_debug("publish %s -> %f",iop->name,iop->value.d);
                    break;
                case IGS_BOOL_T:
                    zmsg_addmem(msg, &(iop->value.b), sizeof(bool));
                    igs_debug("publish %s -> %d",iop->name,iop->value.b);
                    break;
                case IGS_STRING_T:
                    zmsg_addstr(msg, iop->value.s);
                    igs_debug("publish %s -> %s",iop->name,iop->value.s);
                    break;
                case IGS_IMPULSION_T:
                    zmsg_addmem(msg, NULL, 0);
                    igs_debug("publish impulsion %s",iop->name);
                    break;
                case IGS_DATA_T:{
                    zframe_t *frame = zframe_new(iop->value.data, iop->valueSize);
                    zmsg_append(msg, &frame);
                    igs_debug("publish data %s",iop->name);
                }
                    break;
                default:
                    break;
            }
            zmsg_t *msgBis = zmsg_dup(msg);
            if (zmsg_send(&msg, agentElements->publisher) != 0){
                igs_error("Could not publish output %s on the network\n",iop->name);
                zmsg_destroy(&msgBis);
            }else{
                result = 1;
                if (agentElements->ipcPublisher != NULL){
                    //publisher can be NULL on IOS or for read/write problems with assigned IPC path
                    //in both cases, an error message has been issued at start
                    if (zmsg_send(&msgBis, agentElements->ipcPublisher) != 0){
                        igs_error("Could not publish output %s using IPC\n",iop->name);
                    }
                }
            }
        }else{
            if(iop == NULL){
                igs_error("Output %s is unknown", iop->name);
            }
            if (isWholeAgentMuted){
                igs_debug("Should publish output %s but the agent has been muted",iop->name);
            }
            if(iop->is_muted){
                igs_debug("Should publish output %s but it has been muted",iop->name);
            }
            if(isFrozen == true){
                igs_debug("Should publish output %s but the agent has been frozen",iop->name);
            }
        }
    }
    model_readWriteUnlock();
    return result;
}

int igs_observeBus(igs_BusMessageIncoming cb, void *myData){
    if (cb != NULL){
        zyreCallback_t *newCb = calloc(1, sizeof(zyreCallback_t));
        newCb->callback_ptr = cb;
        newCb->myData = myData;
        DL_APPEND(zyreCallbacks, newCb);
    }else{
        igs_error("callback is null");
        return 0;
    }
    return 1;
}

////////////////////////////////////////////////////////////////////////
// PUBLIC API
////////////////////////////////////////////////////////////////////////

/**
 *  \defgroup startStopKillFct Agent: Start / Stop / Kill functions
 *
 */

/**
 * \fn int igs_startWithDevice(const char *networkDevice, int port)
 * \ingroup startStopKillFct
 * \brief Start an agent on a specific network device and network port.
 * \param networkDevice is the name of the network device (ex: eth0, ens2 ...)
 * \param port is the network port number used
 * \return 1 if ok, else 0.
 */
int igs_startWithDevice(const char *networkDevice, unsigned int port){
    if ((networkDevice == NULL) || (strlen(networkDevice) == 0)){
        igs_error("networkDevice cannot be NULL or empty");
        return 0;
    }
    
    if (agentElements != NULL){
        //Agent is already active : need to stop it first
        igs_stop();
    }
    igs_Interrupted = false;
    forcedStop = false;
    
    agentElements = calloc(1, sizeof(zyreloopElements_t));
    strncpy(agentElements->networkDevice, networkDevice, NETWORK_DEVICE_LENGTH-1);
    agentElements->brokerEndPoint[0] = '\0';
    agentElements->ipAddress[0] = '\0';
    
#if (defined WIN32 || defined _WIN32)
    WORD version_requested = MAKEWORD (2, 2);
    WSADATA wsa_data;
    int rc = WSAStartup (version_requested, &wsa_data);
    assert (rc == 0);
    assert (LOBYTE (wsa_data.wVersion) == 2 &&
        HIBYTE (wsa_data.wVersion) == 2);
#endif

    ziflist_t *iflist = ziflist_new ();
    assert (iflist);
    const char *name = ziflist_first (iflist);
    while (name) {
//        printf (" - name=%s address=%s netmask=%s broadcast=%s\n",
//                name, ziflist_address (iflist), ziflist_netmask (iflist), ziflist_broadcast (iflist));
        if (strcmp(name, networkDevice) == 0){
            strncpy(agentElements->ipAddress, ziflist_address (iflist), IP_ADDRESS_LENGTH-1);
            igs_info("Starting with ip address %s and port %d on device %s", agentElements->ipAddress, port, networkDevice);
            break;
        }
        name = ziflist_next (iflist);
    }
    ziflist_destroy (&iflist);
    
    if (strlen(agentElements->ipAddress) == 0){
        igs_error("IP address could not be determined on device %s : our agent will NOT start", networkDevice);
        free(agentElements);
        agentElements = NULL;
        return 0;
    }
#if !TARGET_OS_IOS
    license_readLicense();
#endif
    agentElements->zyrePort = port;
    agentElements->agentActor = zactor_new (initLoop, agentElements);
    assert (agentElements->agentActor);
    return 1;
}

/**
 * \fn int igs_startWithIP(const char *ipAddress, int port)
 * \ingroup startStopKillFct
 * \brief Start an agent on a specific network IP and network port.
 * \param ipAddress is the ip address on network
 * \param port s the network port number used
 * \return 1 if ok, else 0.
 */
int igs_startWithIP(const char *ipAddress, unsigned int port){
    if ((ipAddress == NULL) || (strlen(ipAddress) == 0)){
        igs_error("IP address cannot be NULL or empty");
        return 0;
    }
    
    if (agentElements != NULL){
        //Agent is already active : need to stop it first
        igs_stop();
    }
    igs_Interrupted = false;
    forcedStop = false;
    agentElements = calloc(1, sizeof(zyreloopElements_t));
    agentElements->brokerEndPoint[0] = '\0';
    strncpy(agentElements->ipAddress, ipAddress, IP_ADDRESS_LENGTH-1);
    
#if (defined WIN32 || defined _WIN32)
    WORD version_requested = MAKEWORD (2, 2);
    WSADATA wsa_data;
    int rc = WSAStartup (version_requested, &wsa_data);
    assert (rc == 0);
    assert (LOBYTE (wsa_data.wVersion) == 2 &&
        HIBYTE (wsa_data.wVersion) == 2);
#endif

    ziflist_t *iflist = ziflist_new ();
    assert (iflist);
    const char *name = ziflist_first (iflist);
    while (name) {
//        printf (" - name=%s address=%s netmask=%s broadcast=%s\n",
//                name, ziflist_address (iflist), ziflist_netmask (iflist), ziflist_broadcast (iflist));
        if (strcmp(ziflist_address (iflist), ipAddress) == 0){
            strncpy(agentElements->networkDevice, name, 15);
            igs_info("Starting with ip address %s and port %d on device %s", ipAddress, port, agentElements->networkDevice);
            break;
        }
        name = ziflist_next (iflist);
    }
    ziflist_destroy (&iflist);
    
    if (strlen(agentElements->networkDevice) == 0){
        igs_error("Device name could not be determined for IP address %s : our agent will NOT start", ipAddress);
        agentElements = NULL;
        free(agentElements);
        return 0;
    }
#if !TARGET_OS_IOS
    license_readLicense();
#endif
    agentElements->zyrePort = port;
    agentElements->agentActor = zactor_new (initLoop, agentElements);
    assert (agentElements->agentActor);
 
    return 1;
}

int igs_startWithDeviceOnBroker(const char *networkDevice, const char *brokerIpAddress){
    //TODO: manage a list of brokers instead of just one
    if ((brokerIpAddress == NULL) || (strlen(brokerIpAddress) == 0)){
        igs_error("brokerIpAddress cannot be NULL or empty");
        return 0;
    }
    if ((networkDevice == NULL) || (strlen(networkDevice) == 0)){
        igs_error("networkDevice cannot be NULL or empty");
        return 0;
    }
    
    if (agentElements != NULL){
        igs_stop();
    }
    igs_Interrupted = false;
    forcedStop = false;
    
    agentElements = calloc(1, sizeof(zyreloopElements_t));
    strncpy(agentElements->brokerEndPoint, brokerIpAddress, IP_ADDRESS_LENGTH-1);
    strncpy(agentElements->networkDevice, networkDevice, NETWORK_DEVICE_LENGTH-1);
    agentElements->ipAddress[0] = '\0';
    
#if (defined WIN32 || defined _WIN32)
    WORD version_requested = MAKEWORD (2, 2);
    WSADATA wsa_data;
    int rc = WSAStartup (version_requested, &wsa_data);
    assert (rc == 0);
    assert (LOBYTE (wsa_data.wVersion) == 2 &&
            HIBYTE (wsa_data.wVersion) == 2);
#endif
    
    ziflist_t *iflist = ziflist_new ();
    assert (iflist);
    const char *name = ziflist_first (iflist);
    while (name) {
        //        printf (" - name=%s address=%s netmask=%s broadcast=%s\n",
        //                name, ziflist_address (iflist), ziflist_netmask (iflist), ziflist_broadcast (iflist));
        if (strcmp(name, networkDevice) == 0){
            strncpy(agentElements->ipAddress, ziflist_address (iflist), IP_ADDRESS_LENGTH-1);
            igs_info("Starting with ip address %s on device %s with broker %s",
                      agentElements->ipAddress,
                      networkDevice,
                      agentElements->brokerEndPoint);
        }
        name = ziflist_next (iflist);
    }
    ziflist_destroy (&iflist);
    
    if (strlen(agentElements->ipAddress) == 0){
        igs_error("IP address could not be determined on device %s : our agent will NOT start", networkDevice);
        free(agentElements);
        agentElements = NULL;
        return 0;
    }
#if !TARGET_OS_IOS
    license_readLicense();
#endif
    agentElements->zyrePort = 0;
    agentElements->agentActor = zactor_new (initLoop, agentElements);
    assert (agentElements->agentActor);
    
    return 1;
}

/**
 * \fn int igs_stop()
 * \ingroup startStopKillFct
 * \brief Stop the network layer of an agent.
 * \return 1 if ok, else 0.
 */
int igs_stop(){
    if (agentElements != NULL){
        //interrupting and destroying ingescape thread and zyre layer
        //this will also clean all subscribers
        if (agentElements->node != NULL){
            //we send message only if zactor is still active, i.e.
            //if its node still exists
            zstr_sendx (agentElements->agentActor, "$TERM", NULL);
        }
        if (agentElements->agentActor != NULL){
            zactor_destroy (&agentElements->agentActor);
        }
        //cleaning agent
        free (agentElements);
        agentElements = NULL;
        #if (defined WIN32 || defined _WIN32)
        zsys_shutdown();
        #endif
        igs_info("Agent stopped");
    }else{
        igs_debug("Agent already stopped");
    }
#if !TARGET_OS_IOS
    license_cleanLicense();
#endif
    return 1;
}

/**
 * \fn int igs_setAgentName(const char *name)
 * \ingroup startStopKillFct
 * \brief Set the agent name on network if different that the one defined in its json definition.
 * \param name is the name of the agent.
 * \return 1 if ok, else 0
 */
int igs_setAgentName(const char *name){
    if ((name == NULL) || (strlen(name) == 0)){
        igs_error("Agent name cannot be NULL or empty");
        return 0;
    }
    if (strcmp(igsAgentName, name) == 0){
        //nothing to do
        return 1;
    }
    char networkDevice[NETWORK_DEVICE_LENGTH] = "";
    char ipAddress[IP_ADDRESS_LENGTH] = "";
    int zyrePort = 0;
    bool needRestart = false;
    if (agentElements != NULL){
        //Agent is already started, zyre actor needs to be recreated
        strncpy(networkDevice, agentElements->networkDevice, NETWORK_DEVICE_LENGTH);
        strncpy(ipAddress, agentElements->ipAddress, IP_ADDRESS_LENGTH);
        zyrePort = agentElements->zyrePort;
        igs_stop();
        needRestart = true;
    }
    char *n = strndup(name, MAX_AGENT_NAME_LENGTH);
    if (strlen(name) > MAX_AGENT_NAME_LENGTH){
        igs_warn("Agent name '%s' exceeds maximum size and will be truncated to '%s'", name, n);
    }
    bool spaceInName = false;
    size_t lengthOfN = strlen(n);
    size_t i = 0;
    for (i = 0; i < lengthOfN; i++){
        if (n[i] == ' '){
            n[i] = '_';
            spaceInName = true;
        }
    }
    if (spaceInName){
        igs_warn("Spaces are not allowed in agent name: %s has been renamed to %s", name, n);
    }
    strncpy(igsAgentName, n, MAX_AGENT_NAME_LENGTH-1);
    free(n);
    
    if (needRestart){
        igs_startWithIP(ipAddress, zyrePort);
    }
    igs_debug("Agent name is %s", igsAgentName);
    return 1;
}

/**
 * \fn char *igs_getAgentName()
 * \ingroup startStopKillFct
 * \brief Get the agent name on the network
 * \return the name of the agent.
 * \warning Allocate memory that should be freed by the user.
 */
char *igs_getAgentName(){
    return strdup(igsAgentName);
}

/**
 *  \defgroup pauseResumeFct Agent: Pause / resume functions
 *
 */

/**
 * \fn int igs_freeze()
 * \ingroup pauseResumeFct
 * \brief Freeze agent. Execute the associated FreezeCallback to the agent.
 * by default no callback is defined.
 * \return 1 if ok, else 0
 */
int igs_freeze(){
    if (!agentCanBeFrozen){
        igs_error("agent is requested to be frozen but is still set to 'can't be Frozen' : call igs_setCanBeFrozen to change this");
        return 0;
    }
    if(isFrozen == false)
    {
        igs_debug("Agent frozen");
        if ((agentElements != NULL) && (agentElements->node != NULL)){
            zyre_shouts(agentElements->node, CHANNEL, "FROZEN=1");
        }
        isFrozen = true;
        freezeCallback_t *elt;
        DL_FOREACH(freezeCallbacks,elt){
            elt->callback_ptr(isFrozen, elt->myData);
        }
    }
    return 1;
}

/**
 * \fn bool igs_isFrozen()
 * \ingroup pauseResumeFct
 * \brief return the frozon state of a agent.
 * \return true if frozen else false.
 */
bool igs_isFrozen(){
    return isFrozen;
}

/**
 * \fn int igs_unfreeze()
 * \ingroup pauseResumeFct
 * \brief  Unfreeze agent. Execute the associated FreezeCallback to the agent.
 * by default no callback is defined.
 * \return 1 if ok, else 0
 */
int igs_unfreeze(){
    if(isFrozen == true)
    {
        igs_debug("Agent resumed (unfrozen)");
        if ((agentElements != NULL) && (agentElements->node != NULL)){
            zyre_shouts(agentElements->node, CHANNEL, "FROZEN=0");
        }
        isFrozen = false;
        freezeCallback_t *elt;
        DL_FOREACH(freezeCallbacks,elt){
            elt->callback_ptr(isFrozen, elt->myData);
        }
    }
    return 1;
}

/**
 * \fn int igs_observeFreeze(igs_freezeCallback cb, void *myData)
 * \ingroup pauseResumeFct
 * \brief Add a igs_freezeCallback on an agent.
 * \param cb is a pointer to a igs_freezeCallback
 * \param myData is pointer to user data is needed.
 * \return 1 if ok, else 0.
 */
int igs_observeFreeze(igs_freezeCallback cb, void *myData){
    if (cb != NULL){
        freezeCallback_t *newCb = calloc(1, sizeof(freezeCallback_t));
        newCb->callback_ptr = cb;
        newCb->myData = myData;
        DL_APPEND(freezeCallbacks, newCb);
    }else{
        igs_warn("callback is null");
        return 0;
    }
    return 1;
}

/**
 *  \defgroup controleAgentFct Agent: Controle functions
 *
 */

/**
 * \fn int igs_setAgentState(const char *state)
 * \ingroup controleAgentFct
 * \brief set the internal state of an agent (as the developper defined it)
 * \return 1 if ok, else 0.
 * \param state is the name of the state you want to send.
 */
int igs_setAgentState(const char *state){
    if (state == NULL){
        igs_error("state can not be NULL");
        return 0;
    }
    
    if (strcmp(state, igsAgentState) != 0){
        strncpy(igsAgentState, state, MAX_AGENT_NAME_LENGTH-1);
        igs_debug("changed to %s", igsAgentState);
        if (agentElements != NULL && agentElements->node != NULL){
            zyre_shouts(agentElements->node, CHANNEL, "STATE=%s", igsAgentState);
        }
    }
    return 1;
}

/**
 * \fn char *igs_getAgentState()
 * \ingroup controleAgentFct
 * \brief get the internal state of an agent (as the developper defined it)
 * \return the name of the state you want to get.
 * \warning Allocate memory that should be freed by the user.
 */
char *igs_getAgentState(){
    return strdup(igsAgentState);
}


/**
 * \fn void igs_setCanBeFrozen (bool canBeFrozen)
 * \ingroup setGetLibraryFct
 * \brief set or unset forzen mode on the agent.
 * \param canBeFrozen is a bool to set or unset the verbose mode
 */
void igs_setCanBeFrozen (bool canBeFrozen){
    agentCanBeFrozen = canBeFrozen;
    if (agentElements != NULL && agentElements->node != NULL){
        //update header for information to agents not arrived yet
        zyre_set_header(agentElements->node, "canBeFrozen", "%i", agentCanBeFrozen);
        //send real time notification for agents already there
        zyre_shouts(agentElements->node, CHANNEL, "CANBEFROZEN=%i", canBeFrozen);
        igs_debug("changed to %d", canBeFrozen);
    }
}

/**
 * \fn void igs_canBeFrozen (void)
 * \ingroup setGetLibraryFct
 * \brief check if our agent can be frozen
 */
bool igs_canBeFrozen (void){
    return agentCanBeFrozen;
}


/**
 *  \defgroup muteAgentFct Agent: Mute functions
 *
 */

/**
 * \fn int igs_mute()
 * \ingroup muteAgentFct
 * \brief function to mute the agent
 * \return 1 if ok else 0.
 */
int igs_mute(){
    if (!isWholeAgentMuted)
    {
        isWholeAgentMuted = true;
        if ((agentElements != NULL) && (agentElements->node != NULL)){
            zyre_shouts(agentElements->node, CHANNEL, "MUTED=%i", isWholeAgentMuted);
        }
        muteCallback_t *elt;
        DL_FOREACH(muteCallbacks,elt){
            elt->callback_ptr(isWholeAgentMuted, elt->myData);
        }
    }
    return 1;
}

/**
 * \fn int igs_unmute()
 * \ingroup muteAgentFct
 * \brief function to unmute the agent
 * \return 1 if ok or 0.
 */
int igs_unmute(){
    if (isWholeAgentMuted)
    {
        isWholeAgentMuted = false;
        if ((agentElements != NULL) && (agentElements->node != NULL)){
            zyre_shouts(agentElements->node, CHANNEL, "MUTED=%i", isWholeAgentMuted);
        }
        muteCallback_t *elt;
        DL_FOREACH(muteCallbacks,elt){
            elt->callback_ptr(isWholeAgentMuted, elt->myData);
        }
    }
    return 1;
}

/**
 * \fn bool igs_isMuted()
 * \ingroup muteAgentFct
 * \brief function to know if the agent are muted
 * \return true if it is muted else false.
 */
bool igs_isMuted(){
    return isWholeAgentMuted;
}


/**
 * \fn int igs_observeMute(igs_muteCallback cb, void *myData)
 * \ingroup muteAgentFct
 * \brief Add a igs_muteCallback on an agent.
 * \param cb is a pointer to a igs_muteCallback
 * \param myData is pointer to user data is needed.
 * \return 1 if ok, else 0.
 */
int igs_observeMute(igs_muteCallback cb, void *myData){
    if (cb != NULL){
        muteCallback_t *newCb = calloc(1, sizeof(muteCallback_t));
        newCb->callback_ptr = cb;
        newCb->myData = myData;
        DL_APPEND(muteCallbacks, newCb);
    }
    else{
        igs_warn("callback is null");
        return 0;
    }
    return 1;
}


void igs_die(){
    forcedStop = true;
    igs_stop();
}

void igs_setCommandLine(const char *line){
    strncpy(commandLine, line, COMMAND_LINE_LENGTH-1);
    igs_debug("Command line set to %s", commandLine);
}

void igs_setCommandLineFromArgs(int argc, const char * argv[]){
    if (argc < 1 || argv == NULL || argv[0] == NULL){
        igs_error("passed args must at least contain one element");
        return;
    }
    
    
    char cmd[COMMAND_LINE_LENGTH] = "";
    
#if defined __linux__ || defined __APPLE__ || defined __unix__
    int ret;
    pid_t pid;
    pid = getpid();
#ifdef __APPLE__
#if TARGET_OS_IOS
    char pathbuf[64] = "no_path";
    ret = 1;
#elif TARGET_OS_OSX
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    ret = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
#endif
#else
    char pathbuf[4*1024];
    memset(pathbuf, 0, 4*1024);
    ret = readlink("/proc/self/exe", pathbuf, sizeof(pathbuf));
#endif
    if ( ret <= 0 ) {
        igs_error("PID %d: proc_pidpath () - %s", pid, strerror(errno));
        return;
    } else {
        igs_trace("proc %d: %s", pid, pathbuf);
    }
    if (strlen(pathbuf) < COMMAND_LINE_LENGTH){
        strcat(cmd, pathbuf);
    }else{
        igs_error("path is too long: %s", pathbuf);
        return;
    }
    
#elif (defined WIN32 || defined _WIN32)
    strcat(cmd, argv[0]);
#endif
    
    int i = 1;
    for (; i<argc; i++){
        if (strlen(cmd) + strlen(argv[i]) + 2 > COMMAND_LINE_LENGTH){ // 2 is for space and EOL
            igs_error("passed arguments exceed buffer size: concatenation will stop here with '%s'", cmd);
            break;
        }else{
            strcat(cmd, " ");
        }
        strcat(cmd, argv[i]);
    }
    strncpy(commandLine, cmd, COMMAND_LINE_LENGTH);
    igs_debug("Command line set to %s", commandLine);
}

void igs_setRequestOutputsFromMappedAgents(bool notify){
    network_RequestOutputsFromMappedAgents = notify;
    igs_debug("changed to %d", notify);
}

bool igs_getRequestOutputsFromMappedAgents(){
    return network_RequestOutputsFromMappedAgents;
}

#define MAX_NUMBER_OF_NETDEVICES 16

void igs_getNetdevicesList(char ***devices, int *nb){
#if (defined WIN32 || defined _WIN32)
    WORD version_requested = MAKEWORD (2, 2);
    WSADATA wsa_data;
    int rc = WSAStartup (version_requested, &wsa_data);
    assert (rc == 0);
    assert (LOBYTE (wsa_data.wVersion) == 2 &&
            HIBYTE (wsa_data.wVersion) == 2);
#endif
    *devices = calloc(MAX_NUMBER_OF_NETDEVICES, sizeof(char*));
    int currentDeviceNb = 0;
    
    ziflist_t *iflist = ziflist_new ();
    assert (iflist);
    const char *name = ziflist_first (iflist);
    while (name) {
        //        printf (" - name=%s address=%s netmask=%s broadcast=%s\n",
        //                name, ziflist_address (iflist), ziflist_netmask (iflist), ziflist_broadcast (iflist));
        (*devices)[currentDeviceNb] = calloc(NETWORK_DEVICE_LENGTH+1, sizeof(char));
        strncpy((*devices)[currentDeviceNb], name, NETWORK_DEVICE_LENGTH);
        currentDeviceNb++;
        name = ziflist_next (iflist);
    }
    ziflist_destroy (&iflist);
    *nb = currentDeviceNb;
#if (defined WIN32 || defined _WIN32)
    WSACleanup();
#endif
}

void igs_freeNetdevicesList(char **devices, int nb){
    int i = 0;
    for (i = 0; i < nb; i++){
        if (devices[i] != NULL && strlen(devices[i]) > 0){
            free(devices[i]);
        }
    }
    free (devices);
}

void igs_observeForcedStop(igs_forcedStopCallback cb, void *myData){
    if (cb != NULL){
        forcedStopCalback_t *newCb = calloc(1, sizeof(forcedStopCalback_t));
        newCb->callback_ptr = cb;
        newCb->myData = myData;
        DL_APPEND(forcedStopCalbacks, newCb);
    }else{
        igs_warn("callback is null");
    }
}

void igs_setDiscoveryInterval(unsigned int interval){
    if (agentElements != NULL && agentElements->node != NULL){
        zyre_set_interval(agentElements->node, interval);
    }
    network_discoveryInterval = interval;
}

void igs_setAgentTimeout(unsigned int duration){
    if (agentElements != NULL && agentElements->node != NULL){
        zyre_set_expired_timeout(agentElements->node, duration);
    }
    network_agentTimeout = duration;
}

void igs_setPublishingPort(unsigned int port){
    if (agentElements != NULL && agentElements->publisher != NULL){
        igs_error("agent is already started : stop it first to change its publishing port");
        return;
    }
    network_publishingPort = port;
}

#if defined __unix__ || defined __APPLE__ || defined __linux__
void igs_setIpcFolderPath(char *path){
    if (strcmp(path, DEFAULT_IPC_PATH) == 0
        || (ipcFolderPath != NULL && strcmp(path, ipcFolderPath) == 0)){
        igs_debug("IPC folder path already is '%s'", path);
    }else{
        if (*path == '/'){
            if (ipcFolderPath != NULL){
                free(ipcFolderPath);
            }
            if (!zsys_file_exists(path)){
                igs_info("folder %s was created automatically", path);
                zsys_dir_create("%s", path);
            }
            ipcFolderPath = strdup(path);
        }else{
            igs_error("IPC folder path must be absolute");
        }
    }
}

const char* igs_getIpcFolderPath(void){
    return ipcFolderPath;
}

void igs_setAllowIpc(bool allow){
    allowIpc = allow;
}

bool igs_getAllowIpc(void){
    return allowIpc;
}
#endif

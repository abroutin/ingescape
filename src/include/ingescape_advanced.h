//
//  ingescape_advanced.h
//  ingescape - https://ingescape.com
//
//  Created by Stephane Vales on 07/04/2018.
//  Copyright © 2018 Ingenuity i/o. All rights reserved.
//

#ifndef ingescape_advanced_h
#define ingescape_advanced_h

#include <czmq.h>
#include "ingescape.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STRING_MSG_LENGTH 4096


//////////////////////////////////////////////////
/*BROKERS VS. SELF-DISCOVERY
 
 igs_startWithDevice and igs_startWithIP enable the agents to self-discover
 using UDP broadcast messages on the passed port. UDP broadcast messages can
 be blocked on some networks and can make things complex on networks with
 sub-networks.
 That is why ingescape also supports the use of brokers to relay discovery
 using TCP connections. Any agent can be a broker and agents using brokers
 simply have to use a list of broker endpoints. One broker is enough but
 several brokers can be set for robustness.
 
 For clarity, it is better if brokers are well identified on your platform,
 started before any agent, and serve only as brokers. But any other architecture
 is permitted and brokers can be restarted at any time.
 
 Endpoints have the form tcp://ip_address:port
 • A broker endpoint in igs_brokerAdd is used to connect to a given broker. Add
 as many brokers as you want. At least one declared broker is necessary to
 use igs_startWithBrokers.
 • The endpoint in igs_setAsBroker is the broker address we should be reached
 at as a broker if we want to be one. Using igs_setAsBroker makes us a broker
 when starting.
 • The endpoint in igs_brokerAdvertiseEndpoint replaces the one declared in
 igs_startWithBrokers for the registration to the brokers. This function enables
 passing through NAT and using a public address. Attention: this public address
 shall make sense to all the agents that will connect to us, independently from
 their local network.
 • Our agent endpoint in igs_startWithBrokers gives the address and port our
 agent can be reached at. This endpoint must be valid in the actual network
 configuration.
 */
PUBLIC igs_result_t igs_brokerAdd(const char *brokerEndpoint);
PUBLIC void igs_enableAsBroker(const char *ourBrokerEndpoint);
PUBLIC void igs_brokerAdvertiseEndpoint(const char *advertisedEndpoint); //parameter can be NULL
PUBLIC igs_result_t igs_startWithBrokers(const char *agentEndpoint);


//////////////////////////////////////////////////
/* SECURITY
 Security is about authentification of other agents and encrypted communications.
 Both are offered by Ingescape using a public/private keys mechanism relying on ZeroMQ.
 Security is activated optionally.
 • If public/private keys are generated on the fly, one obtains the same protection as TLS
 for HTTPS communications. Thirdparties cannot steal identities and communications are
 encrypted end-to-end. But any Ingescape agent with security enabled can join a platform.
 • If public/private keys are stored locally by each agent, no thirdparty can join a platform
 without having a public key that is well-known by the other agents. This is safer but requires
 securing and synchronizing local files with each agent accessing its private key and public
 keys of other agents.
 
 Security is enabled by calling igs_enableSecurity.
 • If privateKey is NULL, our private key is generated on the fly and any agent with
 security enabled will be able to connect, publicKeysDirectory will be ignored.
 • If privateKey is NOT NULL, private key at privateKey path will be used and only
 agents whose public keys are in publicKeysDirectory will be able to connect.
 NB: if privateKey is NOT NULL and publicKeysDirectory is NULL or does not exist,
 security will not be enabled and our agent will not start.
*/
PUBLIC igs_result_t igs_enableSecurity(const char *privateKeyFile, const char *publicKeysDirectory);
PUBLIC igs_result_t igs_brokerAddSecure(const char *brokerEndpoint, const char *publicKeyPath);

//////////////////////////////////////////////////
/* MASTER/SLAVE, leadership between agents
 Create contests between agents and designate a leader.
 Agents can participate in any contest. The designated leader
 in a specific context is notified by an agent event.
 */
PUBLIC igs_result_t igs_competeInElection(const char *electionName);
PUBLIC igs_result_t igs_leaveElection(const char *electionName);


//////////////////////////////////////////////////
// Advanced admin functions

//NETWORK CONFIGURATION
PUBLIC void igs_setPublishingPort(unsigned int port);
PUBLIC void igs_setLogStreamPort(unsigned int port);
PUBLIC void igs_setDiscoveryInterval(unsigned int interval); //in milliseconds
PUBLIC void igs_setAgentTimeout(unsigned int duration); //in milliseconds
PUBLIC void igs_raiseSocketsLimit(void); //UNIX only, to be called before any ingescape or ZeroMQ activity
//Set high water marks (HWM) for the publish/subscribe sockets.
//Setting HWM to 0 means that they are disabled.
PUBLIC void igs_setHighWaterMarks(int hwmValue);


//PERFORMANCE CHECK
//sends number of messages with defined size and displays performance
//information when finished (information displayed as INFO-level log)
PUBLIC void igs_performanceCheck(const char *peerId, size_t msgSize, size_t nbOfMsg);


//TIMERS
//Timers can be created to call code a certain number of times,
//each time after a certain delay. 0 times means repeating forever.
//Timers must be created after starting an agent.
typedef void (igs_timerCallback) (int timerId, void *myData);
PUBLIC int igs_timerStart(size_t delay, size_t times, igs_timerCallback cb, void *myData); //returns timer id or -1 if error
PUBLIC void igs_timerStop(int timerId);


//NETWORK MONITORING
//IngeScape provides an integrated monitor to detect events relative to the network
//Warning: once igs_monitoringEnable has been called, igs_monitoringDisable must be
//called to actually stop the monitor. If not stopped, it may cause an error when
//an agent terminates.
PUBLIC void igs_monitoringEnable(unsigned int period); //in milliseconds
PUBLIC void igs_monitoringEnableWithExpectedDevice(unsigned int period, const char* networkDevice, unsigned int port);
PUBLIC void igs_monitoringDisable(void);
PUBLIC bool igs_isMonitoringEnabled(void);
//When the monitor is started and igs_monitoringShallStartStopAgent is set to true :
// - IP change will cause the agent to restart on the new IP (same device, same port)
// - Network device disappearance will cause the agent to stop. Agent will restart when device is back.
PUBLIC void igs_monitoringShallStartStopAgent(bool flag);
typedef enum {
    IGS_NETWORK_OK = 1, //Default status when the monitor starts
    IGS_NETWORK_DEVICE_NOT_AVAILABLE, //Status when our network device is not available
    IGS_NETWORK_ADDRESS_CHANGED, //Status when the IP address of our network device has changed
    IGS_NETWORK_OK_AFTER_MANUAL_RESTART // Status when our agent has been manually restarted and is now OK
} igs_monitorEvent_t;
typedef void (*igs_monitorCallback)(igs_monitorEvent_t event, const char *device, const char *ipAddress, void *myData);
PUBLIC void igs_monitor(igs_monitorCallback cb, void *myData);


//////////////////////////////////////////////////
// DATA SERIALIZATION using ZeroMQ
/*
 These two functions enable sending and receiving on DATA
 inputs/outputs by using zmsg_t structures. zmsg_t structures
 offer advanced functionalities for data serialization.
 More can be found here: http://czmq.zeromq.org/manual:zmsg
 */
PUBLIC igs_result_t igs_writeOutputAsZMQMsg(const char *name, zmsg_t *msg);
PUBLIC igs_result_t igs_readInputAsZMQMsg(const char *name, zmsg_t **msg); //msg must be freed by caller using zmsg_destroy


//////////////////////////////////////////////////
// BUS channels
typedef void (*igs_BusMessageIncoming) (const char *event, const char *peerID, const char *peerName,
                                        const char *address, const char *channel,
                                        zhash_t *headers, zmsg_t *msg, void *myData);
PUBLIC void igs_observeBus(igs_BusMessageIncoming cb, void *myData);

PUBLIC igs_result_t igs_busJoinChannel(const char *channel);
PUBLIC void igs_busLeaveChannel(const char *channel);

PUBLIC igs_result_t igs_busSendStringToChannel(const char *channel, const char *msg, ...);
PUBLIC igs_result_t igs_busSendDataToChannel(const char *channel, void *data, size_t size);
PUBLIC igs_result_t igs_busSendZMQMsgToChannel(const char *channel, zmsg_t **msg_p); //destroys message after sending it

// Sending message to an agent by name or by uuid
//NB: peer ids and names are also supported by these functions but are used only if no agent is found first
//NB: if several agents share the same name, all will receive the message if addressed by name
PUBLIC igs_result_t igs_busSendStringToAgent(const char *agentNameOrAgentIdOrPeerID, const char *msg, ...);
PUBLIC igs_result_t igs_busSendDataToAgent(const char *agentNameOrAgentIdOrPeerID, void *data, size_t size);
PUBLIC igs_result_t igs_busSendZMQMsgToAgent(const char *agentNameOrAgentIdOrPeerID, zmsg_t **msg_p); //destroys message after sending it

PUBLIC igs_result_t igs_busAddServiceDescription(const char *key, const char *value);
PUBLIC igs_result_t igs_busRemoveServiceDescription(const char *key);


//////////////////////////////////////////////////
//CALLS : create, remove, call, react
/*NOTES:
 - one and only one mandatory callback per call, set using igs_handleCall : generates warning if cb missing when loading definition or receiving call
 - one optional reply per call
 - reply shall be sent in callabck, using igs_sendCall with sender's UUID or name
 - call names shall be unique for a given agent
 */

//SEND CALLS to other agents
//call arguments are provided as a chained list
typedef struct igs_callArgument{
    char *name;
    iopType_t type;
    union{
        bool b;
        int i;
        double d;
        char *c;
        void *data;
    };
    size_t size;
    struct igs_callArgument *next;
} igs_callArgument_t;

//Arguments management
//arguments list shall be initialized to NULL and filled by calling igs_add*ToArgumentsList
//Example:
// igs_callArgument_t *list = NULL;
// igs_addIntToArgumentsList(&list, 10);
PUBLIC void igs_addIntToArgumentsList(igs_callArgument_t **list, int value);
PUBLIC void igs_addBoolToArgumentsList(igs_callArgument_t **list, bool value);
PUBLIC void igs_addDoubleToArgumentsList(igs_callArgument_t **list, double value);
PUBLIC void igs_addStringToArgumentsList(igs_callArgument_t **list, const char *value);
PUBLIC void igs_addDataToArgumentsList(igs_callArgument_t **list, void *value, size_t size);
PUBLIC void igs_destroyArgumentsList(igs_callArgument_t **list);
PUBLIC igs_callArgument_t *igs_cloneArgumentsList(igs_callArgument_t *list);

//SEND a call to another agent
//requires to pass agent name or UUID, call name and a list of arguments specific to the call
//passed arguments list will be deallocated and destroyed
PUBLIC igs_result_t igs_sendCall(const char *agentNameOrUUID, const char *callName, igs_callArgument_t **list);


//CREATE CALLS for our agent
//callback model to handle calls received by our agent
typedef void (*igs_callFunction)(const char *senderAgentName, const char *senderAgentUUID,
                                 const char *callName, igs_callArgument_t *firstArgument, size_t nbArgs,
                                 void* myData);


//Manage calls supported by our agent
//Calls can be created either by code or by loading a definition. The function below will
//create a call if it does not exist or will attach callback and data if they are
//stil undefined. Warning: only one callback can be attached to a call (further attempts
//will be ignored and signaled by an error log).
PUBLIC igs_result_t igs_initCall(const char *name, igs_callFunction cb, void *myData);
PUBLIC igs_result_t igs_removeCall(const char *name);
PUBLIC igs_result_t igs_addArgumentToCall(const char *callName, const char *argName, iopType_t type);
PUBLIC igs_result_t igs_removeArgumentFromCall(const char *callName, const char *argName); //removes first occurence with this name


//Manage optional reply
//NB: a reply can be seen as a subcall used to answer to sender upon call reception.
//PUBLIC int igs_addReplyToCall(const char *callName, const char *replyName);
//PUBLIC int igs_addArgumentToReplyForCall(const char *callName, const char *argName, iopType_t type);
//PUBLIC int igs_removeArgumentFromReplyForCall(const char *callName, const char *argName);
//PUBLIC int igs_removeReplyFromCall(const char *callName); //reply elements will be destroyed as well

//introspection for calls, arguments and replies
PUBLIC size_t igs_getNumberOfCalls(void);
PUBLIC bool igs_checkCallExistence(const char *name);
PUBLIC char** igs_getCallsList(size_t *nbOfElements); //returned char** shall be freed by caller
PUBLIC void igs_freeCallsList(char ***list, size_t nbOfCalls);

PUBLIC igs_callArgument_t* igs_getFirstArgumentForCall(const char *callName);
PUBLIC size_t igs_getNumberOfArgumentsForCall(const char *callName);
PUBLIC bool igs_checkCallArgumentExistence(const char *callName, const char *argName);
//PUBLIC igs_callArgument_t* igs_getFirstArgumentForReplyInCall(const char *callName);
//PUBLIC size_t igs_getNumberOfArgumentsForReplyInCall(const char *callName);
//PUBLIC char* igs_getReplyNameInCall(const char *callName); //returned char* must be freed by caller, NULL if no reply
//PUBLIC bool igs_isReplyAddedForCall(const char *name);
//PUBLIC bool igs_checkCallReplyArgumentExistence(const char *callName, const char *argName);


//////////////////////////////////////////////////
//JSON facilities
typedef struct _igsJSON* igsJSON_t;
PUBLIC void igs_JSONfree(igsJSON_t *json);

// generate a JSON string
PUBLIC igsJSON_t igs_JSONinit(void); //must call igs_JSONfree on returned value to free it
PUBLIC void igs_JSONopenMap(igsJSON_t json);
PUBLIC void igs_JSONcloseMap(igsJSON_t json);
PUBLIC void igs_JSONopenArray(igsJSON_t json);
PUBLIC void igs_JSONcloseArray(igsJSON_t json);
PUBLIC void igs_JSONaddNULL(igsJSON_t json);
PUBLIC void igs_JSONaddBool(igsJSON_t json, bool value);
PUBLIC void igs_JSONaddInt(igsJSON_t json, long long value);
PUBLIC void igs_JSONaddDouble(igsJSON_t json, double value);
PUBLIC void igs_JSONaddString(igsJSON_t json, const char *value);
PUBLIC void igs_JSONprint(igsJSON_t json);
PUBLIC char* igs_JSONdump(igsJSON_t json); //returned value must be freed by caller
PUBLIC char* igs_JSONcompactDump(igsJSON_t json); //returned value must be freed by caller

// parse a JSON string or file based on parsing events and a callback
typedef enum {
    IGS_JSON_STRING = 1,
    IGS_JSON_NUMBER, //int or double
    IGS_JSON_MAP,
    IGS_JSON_ARRAY,
    IGS_JSON_TRUE, //not used in parsing callback
    IGS_JSON_FALSE, //not used in parsing callback
    IGS_JSON_NULL,
    IGS_JSON_KEY = 9, //not used in tree queries
    IGS_JSON_MAP_END, //not used in tree queries
    IGS_JSON_ARRAY_END, //not used in tree queries
    IGS_JSON_BOOL //not used in tree queries
} igs_JSONValueType_t;
typedef void (*igs_JSONCallback)(igs_JSONValueType_t type, void *value, size_t size, void *myData);
PUBLIC void igs_JSONparseFromFile(const char *path, igs_JSONCallback cb, void *myData);
PUBLIC void igs_JSONparseFromString(const char *content, igs_JSONCallback cb, void *myData);

// parse a JSON string or file in a tree supporting queries
typedef struct igsJSONNode {
    igs_JSONValueType_t type;
    union {
        char * string;
        struct {
            long long i; //integer value, if representable
            double  d;   //double value, if representable
            char   *r;   //unparsed number in string form
            unsigned int flags; //flags to manage double and int values
        } number;
        struct {
            const char **keys; //array of keys
            struct igsJSONNode **values; //array of nodes
            size_t len; //number of key-node-pairs
        } object;
        struct {
            struct igsJSONNode **values; //array of nodes
            size_t len; //number of nodes
        } array;
    } u;
} igsJSONTreeNode_t;
PUBLIC void igs_JSONTreeFree(igsJSONTreeNode_t **node);
PUBLIC igsJSONTreeNode_t* igs_JSONTreeParseFromFile(const char *path);
PUBLIC igsJSONTreeNode_t* igs_JSONTreeParseFromString(const char *content);
PUBLIC igsJSONTreeNode_t* igs_JSONTreeClone(igsJSONTreeNode_t *tree); //returned value must be freed by caller
PUBLIC char* igs_JSONTreeDump(igsJSONTreeNode_t *tree); //returned value must be freed by caller

PUBLIC void igs_JSONaddTree(igsJSON_t json, igsJSONTreeNode_t *tree);
PUBLIC igsJSONTreeNode_t* igs_JSONgetTree(igsJSON_t json); //returned value must be freed by caller

//add node at the end of an array
PUBLIC void igs_JSONTreeInsertInArray(igsJSONTreeNode_t *array, igsJSONTreeNode_t *nodeToInsert); //does NOT take ownership of node to insert

//add node (or replace if it already existis) in map based on named key
PUBLIC void igs_JSONTreeInsertInMap(igsJSONTreeNode_t *map, const char *key, igsJSONTreeNode_t *nodeToInsert); //does NOT take ownership of node to insert

/* Tree node can handle queries to retrieve sub-nodes
 Important notes :
 - returned value must NOT be freed manually : it is owned by the node
 - returned structure contains a type that shall be checked to handle actual contained value(s)
 */
PUBLIC igsJSONTreeNode_t* igs_JSONTreeGetNodeAtPath(igsJSONTreeNode_t *node, const char **path);

//JSON parsing creates number values. Use these two additional functions
//to check them as int and double values.
//NB: int values are considered both int and double
PUBLIC bool igs_JSONTreeIsValueAnInteger(igsJSONTreeNode_t *value);
PUBLIC bool igs_JSONTreeIsValueADouble(igsJSONTreeNode_t *value);


#ifdef __cplusplus
}
#endif

#endif /* ingescape_advanced_h */

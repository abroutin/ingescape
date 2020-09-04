//
//  ingescape_class.h
//  ingescape
//
//  Created by Stephane Vales on 18/11/2019.
//  Copyright © 2019 Ingenuity i/o. All rights reserved.
//

#ifndef ingescape_class_h
#define ingescape_class_h

#include "ingescape_advanced.h"

#ifdef __cplusplus
extern "C" {
#endif

//agent creation and destruction
typedef struct igs_agent igs_agent_t;
PUBLIC igs_agent_t *igsAgent_new(const char *name, bool activateImmediately);
PUBLIC void igsAgent_destroy(igs_agent_t **agent);

//attach and detach agent from context
PUBLIC igs_result_t igsAgent_activate(igs_agent_t *agent);
PUBLIC igs_result_t igsAgent_deactivate(igs_agent_t *agent);
PUBLIC bool igsAgent_isActivated(igs_agent_t *agent);
typedef void (*igsAgent_activateCallback)(igs_agent_t *Agent, bool isActivated, void *myData);
PUBLIC void igsAgent_observeActivate(igs_agent_t *agent, igsAgent_activateCallback cb, void *myData);

/*
 NOTICE:
 All the functions below behave the same as the functions presented
 in ingescape.h, their only addition consists in taking an agent instance
 as first argument.
 The rare exceptions in behavior changes are indicated for each affected
 function.
 */

PUBLIC void igsAgent_setAgentName(igs_agent_t *agent, const char *name);
PUBLIC char *igsAgent_getAgentName(igs_agent_t *agent); //char* must be freed by caller
PUBLIC void igsAgent_setAgentState(igs_agent_t *agent, const char *state);
PUBLIC char *igsAgent_getAgentState(igs_agent_t *agent); //char* must be freed by caller

PUBLIC void igsAgent_mute(igs_agent_t *agent);
PUBLIC void igsAgent_unmute(igs_agent_t *agent);
PUBLIC bool igsAgent_isMuted(igs_agent_t *agent);
typedef void (*igsAgent_muteCallback)(igs_agent_t *Agent, bool isMuted, void *myData);
PUBLIC void igsAgent_observeMute(igs_agent_t *agent, igsAgent_muteCallback cb, void *myData);

typedef void (*igsAgent_agentEventCallback)(igs_agent_t *Agent, igs_agent_event_t event, const char *uuid, const char *name, void *eventData, void *myData);
PUBLIC void igsAgent_observeAgentEvents(igs_agent_t *Agent, igsAgent_agentEventCallback cb, void *myData);

//IOP read, write, creation, destruction, introspection
PUBLIC igs_result_t igsAgent_readInput(igs_agent_t *agent, const char *name, void **value, size_t *size);
PUBLIC igs_result_t igsAgent_readOutput(igs_agent_t *agent, const char *name, void **value, size_t *size);
PUBLIC igs_result_t igsAgent_readParameter(igs_agent_t *agent, const char *name, void **value, size_t *size);
PUBLIC bool igsAgent_readInputAsBool(igs_agent_t *agent, const char *name);
PUBLIC int igsAgent_readInputAsInt(igs_agent_t *agent, const char *name);
PUBLIC double igsAgent_readInputAsDouble(igs_agent_t *agent, const char *name);
PUBLIC char* igsAgent_readInputAsString(igs_agent_t *agent, const char *name); //returned char* must be freed by caller
PUBLIC igs_result_t igsAgent_readInputAsData(igs_agent_t *agent, const char *name, void **data, size_t *size); //returned data must be freed by caller
PUBLIC bool igsAgent_readOutputAsBool(igs_agent_t *agent, const char *name);
PUBLIC int igsAgent_readOutputAsInt(igs_agent_t *agent, const char *name);
PUBLIC double igsAgent_readOutputAsDouble(igs_agent_t *agent, const char *name);
PUBLIC char* igsAgent_readOutputAsString(igs_agent_t *agent, const char *name); //returned char* must be freed by caller
PUBLIC igs_result_t igsAgent_readOutputAsData(igs_agent_t *agent, const char *name, void **data, size_t *size); //returned data must be freed by caller
PUBLIC bool igsAgent_readParameterAsBool(igs_agent_t *agent, const char *name);
PUBLIC int igsAgent_readParameterAsInt(igs_agent_t *agent, const char *name);
PUBLIC double igsAgent_readParameterAsDouble(igs_agent_t *agent, const char *name);
PUBLIC char* igsAgent_readParameterAsString(igs_agent_t *agent, const char *name); //returned char* must be freed by caller
PUBLIC igs_result_t igsAgent_readParameterAsData(igs_agent_t *agent, const char *name, void **data, size_t *size); //returned data must be freed by caller
PUBLIC igs_result_t igsAgent_writeInputAsBool(igs_agent_t *agent, const char *name, bool value);
PUBLIC igs_result_t igsAgent_writeInputAsInt(igs_agent_t *agent, const char *name, int value);
PUBLIC igs_result_t igsAgent_writeInputAsDouble(igs_agent_t *agent, const char *name, double value);
PUBLIC igs_result_t igsAgent_writeInputAsString(igs_agent_t *agent, const char *name, const char *value);
PUBLIC igs_result_t igsAgent_writeInputAsImpulsion(igs_agent_t *agent, const char *name);
PUBLIC igs_result_t igsAgent_writeInputAsData(igs_agent_t *agent, const char *name, void *value, size_t size);
PUBLIC igs_result_t igsAgent_writeOutputAsBool(igs_agent_t *agent, const char *name, bool value);
PUBLIC igs_result_t igsAgent_writeOutputAsInt(igs_agent_t *agent, const char *name, int value);
PUBLIC igs_result_t igsAgent_writeOutputAsDouble(igs_agent_t *agent, const char *name, double value);
PUBLIC igs_result_t igsAgent_writeOutputAsString(igs_agent_t *agent, const char *name, const char *value);
PUBLIC igs_result_t igsAgent_writeOutputAsImpulsion(igs_agent_t *agent, const char *name);
PUBLIC igs_result_t igsAgent_writeOutputAsData(igs_agent_t *agent, const char *name, void *value, size_t size);
PUBLIC igs_result_t igsAgent_writeParameterAsBool(igs_agent_t *agent, const char *name, bool value);
PUBLIC igs_result_t igsAgent_writeParameterAsInt(igs_agent_t *agent, const char *name, int value);
PUBLIC igs_result_t igsAgent_writeParameterAsDouble(igs_agent_t *agent, const char *name, double value);
PUBLIC igs_result_t igsAgent_writeParameterAsString(igs_agent_t *agent, const char *name, const char *value);
PUBLIC igs_result_t igsAgent_writeParameterAsData(igs_agent_t *agent, const char *name, void *value, size_t size);
PUBLIC void igsAgent_clearDataForInput(igs_agent_t *agent, const char *name);
PUBLIC void igsAgent_clearDataForOutput(igs_agent_t *agent, const char *name);
PUBLIC void igsAgent_clearDataForParameter(igs_agent_t *agent, const char *name);

typedef void (*igsAgent_observeCallback)(igs_agent_t *agent, iop_t iopType, const char *name, iopType_t valueType, void *value, size_t valueSize, void *myData);
PUBLIC void igsAgent_observeInput(igs_agent_t *agent, const char *name, igsAgent_observeCallback cb, void *myData);
PUBLIC void igsAgent_observeOutput(igs_agent_t *agent, const char *name, igsAgent_observeCallback cb, void * myData);
PUBLIC void igsAgent_observeParameter(igs_agent_t *agent, const char *name, igsAgent_observeCallback cb, void * myData);

PUBLIC void igsAgent_muteOutput(igs_agent_t *agent, const char *name);
PUBLIC void igsAgent_unmuteOutput(igs_agent_t *agent, const char *name);
PUBLIC bool igsAgent_isOutputMuted(igs_agent_t *agent, const char *name);
PUBLIC iopType_t igsAgent_getTypeForInput(igs_agent_t *agent, const char *name);
PUBLIC iopType_t igsAgent_getTypeForOutput(igs_agent_t *agent, const char *name);
PUBLIC iopType_t igsAgent_getTypeForParameter(igs_agent_t *agent, const char *name);
PUBLIC size_t igsAgent_getInputsNumber(igs_agent_t *agent);
PUBLIC size_t igsAgent_getOutputsNumber(igs_agent_t *agent);
PUBLIC size_t igsAgent_getParametersNumber(igs_agent_t *agent);
PUBLIC char** igsAgent_getInputsList(igs_agent_t *agent, size_t *nbOfElements); //returned char** must be freed using igs_freeIOPList
PUBLIC char** igsAgent_getOutputsList(igs_agent_t *agent, size_t *nbOfElements); //returned char** must be freed using igs_freeIOPList
PUBLIC char** igsAgent_getParametersList(igs_agent_t *agent, size_t *nbOfElements); //returned char** must be freed using igs_freeIOPList
PUBLIC bool igsAgent_checkInputExistence(igs_agent_t *agent, const char *name);
PUBLIC bool igsAgent_checkOutputExistence(igs_agent_t *agent, const char *name);
PUBLIC bool igsAgent_checkParameterExistence(igs_agent_t *agent, const char *name);

//definition
PUBLIC igs_result_t igsAgent_loadDefinition (igs_agent_t *agent, const char* json_str);
PUBLIC igs_result_t igsAgent_loadDefinitionFromPath (igs_agent_t *agent, const char* file_path);
PUBLIC void igsAgent_clearDefinition(igs_agent_t *agent);
PUBLIC char* igsAgent_getDefinition(igs_agent_t *agent);
PUBLIC char *igsAgent_getDefinitionName(igs_agent_t *agent); //returned char* must be freed by caller
PUBLIC char *igsAgent_getDefinitionDescription(igs_agent_t *agent); //returned char* must be freed by caller
PUBLIC char *igsAgent_getDefinitionVersion(igs_agent_t *agent); //returned char* must be freed by caller
PUBLIC void igsAgent_setDefinitionName(igs_agent_t *agent, const char *name);
PUBLIC void igsAgent_setDefinitionDescription(igs_agent_t *agent, const char *description);
PUBLIC void igsAgent_setDefinitionVersion(igs_agent_t *agent, const char *version);
PUBLIC igs_result_t igsAgent_createInput(igs_agent_t *agent, const char *name, iopType_t value_type, void*value, size_t size);
PUBLIC igs_result_t igsAgent_createOutput(igs_agent_t *agent, const char *name, iopType_t type, void *value, size_t size);
PUBLIC igs_result_t igsAgent_createParameter(igs_agent_t *agent, const char *name, iopType_t type, void *value, size_t size);
PUBLIC igs_result_t igsAgent_removeInput(igs_agent_t *agent, const char *name);
PUBLIC igs_result_t igsAgent_removeOutput(igs_agent_t *agent, const char *name);
PUBLIC igs_result_t igsAgent_removeParameter(igs_agent_t *agent, const char *name);

//mapping
PUBLIC igs_result_t igsAgent_loadMapping (igs_agent_t *agent, const char* json_str);
PUBLIC igs_result_t igsAgent_loadMappingFromPath (igs_agent_t *agent, const char* file_path);
PUBLIC void igsAgent_clearMapping(igs_agent_t *agent);
PUBLIC void igsAgent_clearMappingOnAgent(igs_agent_t *agent, const char *agentName);
PUBLIC char* igsAgent_getMapping(igs_agent_t *agent); //returns json string, must be freed by caller
PUBLIC char *igsAgent_getMappingName(igs_agent_t *agent); //returned char* must be freed by caller
PUBLIC char *igsAgent_getMappingDescription(igs_agent_t *agent); //returned char* must be freed by caller
PUBLIC char *igsAgent_getMappingVersion(igs_agent_t *agent); //returned char* must be freed by caller
PUBLIC void igsAgent_setMappingName(igs_agent_t *agent, const char *name);
PUBLIC void igsAgent_setMappingDescription(igs_agent_t *agent, const char *description);
PUBLIC void igsAgent_setMappingVersion(igs_agent_t *agent, const char *version);
PUBLIC size_t igsAgent_getMappingEntriesNumber(igs_agent_t *agent);
PUBLIC unsigned long igsAgent_addMappingEntry(igs_agent_t *agent, const char *fromOurInput, const char *toAgent, const char *withOutput);
PUBLIC igs_result_t igsAgent_removeMappingEntryWithId(igs_agent_t *agent, unsigned long theId);
PUBLIC igs_result_t igsAgent_removeMappingEntryWithName(igs_agent_t *agent, const char *fromOurInput, const char *toAgent, const char *withOutput);

//admin
PUBLIC void igsAgent_log(igs_logLevel_t level, const char *function, igs_agent_t *agent, const char *format, ...) CHECK_PRINTF (4);
#define igsAgent_trace(...) igsAgent_log(IGS_LOG_TRACE, __func__, __VA_ARGS__)
#define igsAgent_debug(...) igsAgent_log(IGS_LOG_DEBUG, __func__, __VA_ARGS__)
#define igsAgent_info(...) igsAgent_log(IGS_LOG_INFO, __func__, __VA_ARGS__)
#define igsAgent_warn(...) igsAgent_log(IGS_LOG_WARN, __func__, __VA_ARGS__)
#define igsAgent_error(...) igsAgent_log(IGS_LOG_ERROR, __func__, __VA_ARGS__)
#define igsAgent_fatal(...) igsAgent_log(IGS_LOG_FATAL, __func__, __VA_ARGS__)

PUBLIC void igsAgent_setRequestOutputsFromMappedAgents(igs_agent_t *agent, bool notify);
PUBLIC bool igsAgent_getRequestOutputsFromMappedAgents(igs_agent_t *agent);

PUBLIC void igsAgent_setDefinitionPath(igs_agent_t *agent, const char *path);
PUBLIC void igsAgent_setMappingPath(igs_agent_t *agent, const char *path);
PUBLIC void igsAgent_writeDefinitionToPath(igs_agent_t *agent);
PUBLIC void igsAgent_writeMappingToPath(igs_agent_t *agent);

PUBLIC igs_result_t igsAgent_writeOutputAsZMQMsg(igs_agent_t *agent, const char *name, zmsg_t *msg);
PUBLIC igs_result_t igsAgent_readInputAsZMQMsg(igs_agent_t *agent, const char *name, zmsg_t **msg); //msg must be freed by caller using zmsg_destroy

//calls
PUBLIC igs_result_t igsAgent_sendCall(igs_agent_t *agent, const char *agentNameOrUUID, const char *callName, igs_callArgument_t **list);
typedef void (*igsAgent_callFunction)(igs_agent_t *agent, const char *senderAgentName, const char *senderAgentUUID,
                                      const char *callName, igs_callArgument_t *firstArgument, size_t nbArgs,
                                      void* myData);
PUBLIC igs_result_t igsAgent_initCall(igs_agent_t *agent, const char *name, igsAgent_callFunction cb, void *myData);
PUBLIC igs_result_t igsAgent_removeCall(igs_agent_t *agent, const char *name);
PUBLIC igs_result_t igsAgent_addArgumentToCall(igs_agent_t *agent, const char *callName, const char *argName, iopType_t type);
PUBLIC igs_result_t igsAgent_removeArgumentFromCall(igs_agent_t *agent, const char *callName, const char *argName); //removes first occurence with this name
PUBLIC size_t igsAgent_getNumberOfCalls(igs_agent_t *agent);
PUBLIC bool igsAgent_checkCallExistence(igs_agent_t *agent, const char *name);
PUBLIC char** igsAgent_getCallsList(igs_agent_t *agent, size_t *nbOfElements); //returned char** shall be freed by caller
PUBLIC igs_callArgument_t* igsAgent_getFirstArgumentForCall(igs_agent_t *agent, const char *callName);
PUBLIC size_t igsAgent_getNumberOfArgumentsForCall(igs_agent_t *agent, const char *callName);
PUBLIC bool igsAgent_checkCallArgumentExistence(igs_agent_t *agent, const char *callName, const char *argName);

//master/slave
PUBLIC igs_result_t igsAgent_competeInElection(igs_agent_t *agent, const char *electionName);
PUBLIC igs_result_t igsAgent_leaveElection(igs_agent_t *agent, const char *electionName);


#ifdef __cplusplus
}
#endif
#endif /* ingescape_class_h */

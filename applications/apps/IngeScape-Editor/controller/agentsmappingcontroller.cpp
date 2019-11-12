/*
 *	IngeScape Editor
 *
 *  Copyright © 2017-2018 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *      Alexandre Lemort   <lemort@ingenuity.io>
 *
 */

#include "agentsmappingcontroller.h"

#include <QQmlEngine>
#include <QDebug>
#include <QFileDialog>
#include <misc/ingescapeutils.h>
#include <model/editorenums.h>
#include <model/actionmappingm.h>


/**
 * @brief Constructor
 * @param modelManager
 * @param jsonHelper
 * @param directoryPath
 * @param parent
 */
AgentsMappingController::AgentsMappingController(EditorModelManager* modelManager,
                                                 JsonHelper* jsonHelper,
                                                 QObject *parent) : QObject(parent),
      _viewWidth(1920 - 320), // Full HD - Width of left panel
      _viewHeight(1080 - 100), // Full HD - Height of top & bottom bars of OS
      _xSpawnZoneOffset(0),
      _ySpawnZoneOffset(0),
      _isEmptyMapping(true),
      _selectedAgent(nullptr),
      _selectedAction(nullptr),
      _selectedLink(nullptr),
      _isLoadedView(false),
      _modelManager(modelManager),
      _jsonHelper(jsonHelper)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    // Connect to signal "Count Changed" from the list of all agents in mapping
    connect(&_allAgentsInMapping, &AbstractI2CustomItemListModel::countChanged, this, &AgentsMappingController::_onAllAgentsInMappingChanged);

}


/**
 * @brief Destructor
 */
AgentsMappingController::~AgentsMappingController()
{
    // Clean-up current selections
    setselectedAgent(nullptr);
    setselectedAction(nullptr);
    setselectedLink(nullptr);

    // DIS-connect from signal "Count Changed" from the list of agents in mapping
    disconnect(&_allAgentsInMapping, nullptr, this, nullptr);

    // Clear the current mapping
    clearMapping();

    // Delete all links
    _hashFromIdToLinkInMapping.clear();
    _hashFromNameToListOfLinksInMapping.clear();
    _hashFromOutputAgentNameToListOfWaitingMappingElements.clear();
    _allLinksInMapping.deleteAllItems();

    // Delete all agents in the mapping
    _hashFromNameToAgentInMapping.clear();
    _allAgentsInMapping.deleteAllItems();

    // Delete all actions in the mapping
    _hashFromUidToActionInMapping.clear();
    _allActionsInMapping.deleteAllItems();

    // Reset pointers
    _modelManager = nullptr;
    _jsonHelper = nullptr;
}


/**
 * @brief Setter for property "is Loaded View"
 * @param value
 */
void AgentsMappingController::setisLoadedView(bool value)
{
    if (_isLoadedView != value)
    {
        _isLoadedView = value;

        // The view of the global mapping is loaded
        if ((_isLoadedView) && (_modelManager != nullptr))
        {
            bool allAgentsOFF = true;
            bool isAddedOrRemovedLink_WhileMappingWasUNactivated = false;

            if (!_allAgentsInMapping.isEmpty())
            {
                // For each agent in the global mapping
                for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
                {
                    if ((agentInMapping != nullptr) && (agentInMapping->agentsGroupedByName() != nullptr))
                    {
                        // If the agent is ON
                        if (agentInMapping->agentsGroupedByName()->isON())
                        {
                            allAgentsOFF = false;
                            //break;
                        }

                        // If the agent had links added/removed while the mapping was UN-activated (loaded from JSON)
                        if (agentInMapping->hadLinksAdded_WhileMappingWasUNactivated() || agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated())
                        {
                            isAddedOrRemovedLink_WhileMappingWasUNactivated = true;
                            //break;
                        }
                    }
                }
            }

            // All agents are OFF
            if (allAgentsOFF)
            {
                // DE-activate the mapping
                _modelManager->setisMappingConnected(false);
            }
            // There have been changes in the mapping while the mapping was UN-activated
            else if (isAddedOrRemovedLink_WhileMappingWasUNactivated)
            {
                qDebug() << "There have been changes in the mapping while the mapping was UN-activated (loaded from JSON). Force to CONTROL ?";

                // The mapping has been modified but will be lost if the user stay in mode OBSERVE
                Q_EMIT changesOnLinksWhileMappingUnactivated();
            }
        }

        Q_EMIT isLoadedViewChanged(value);
    }
}


/**
 * @brief Clear the current mapping
 */
void AgentsMappingController::clearMapping()
{
    qInfo() << "Clear the current mapping";

    // 1- First, DE-activate the mapping
    if (_modelManager != nullptr) {
        _modelManager->setisMappingConnected(false);
    }

    // Clear the hash table from "output agent name" to a list of waiting mapping elements (where the agent is involved as "output agent")
    _hashFromOutputAgentNameToListOfWaitingMappingElements.clear();

    // 2- Delete all links
    for (LinkVM* link : _allLinksInMapping.toList()) {
        _deleteLinkBetweenTwoObjectsInMapping(link);
    }

    // 3- Delete all agents in mapping
    for (AgentInMappingVM* agent : _allAgentsInMapping.toList()) {
        deleteAgentInMapping(agent);
    }

    // 4- Delete all actions in mapping
    for (ActionInMappingVM* action : _allActionsInMapping.toList()) {
        deleteActionInMapping(action);
    }

    qInfo() << "The Mapping is empty !";
}


/**
 * @brief Remove the agent from the mapping and delete the view model
 * @param agent
 */
void AgentsMappingController::deleteAgentInMapping(AgentInMappingVM* agent)
{
    if (agent != nullptr)
    {
        //qDebug() << "Delete the agent" << agent->name() << "in the Mapping";

        // Unselect our agent if needed
        if (_selectedAgent == agent) {
            setselectedAgent(nullptr);
        }

        // DIS-connect to signals from this agent in mapping
        disconnect(agent, nullptr, this, nullptr);

        // Remove from the hash table
        _hashFromNameToAgentInMapping.remove(agent->name());

        // Remove all the links with this agent
        _removeAllLinksWithAgent(agent);

        // Remove from the list to update view (QML)
        _allAgentsInMapping.remove(agent);

        // Free memory
        delete agent;
    }
}


/**
 * @brief Remove the action from the mapping and delete the view model
 * @param action
 */
void AgentsMappingController::deleteActionInMapping(ActionInMappingVM* action)
{
    if (action != nullptr)
    {
        QString actionId = action->uid();

        //qDebug() << "Delete the action" << action->name() << "(" << actionId << ") in the Mapping";

        // Unselect our action if needed
        if (_selectedAction == action) {
            setselectedAction(nullptr);
        }

        // DIS-connect to signals from this action in mapping
        disconnect(action, nullptr, this, nullptr);

        // Remove from the hash table
        _hashFromUidToActionInMapping.remove(actionId);

        // Remove all the links with this action
        //_removeAllLinksWithAction(action);
        for (LinkVM* link : _allLinksInMapping.toList())
        {
            if ( (link != nullptr) && ((link->outputObject() == action) || (link->inputObject() == action)) )
            {
                // Delete the link between two objects in the mapping
                _deleteLinkBetweenTwoObjectsInMapping(link);
            }
        }

        // Remove from the list to update view (QML)
        _allActionsInMapping.remove(action);

        // Free memory
        delete action;

        // Free the UID of the action model
        IngeScapeUtils::freeUIDofActionInMappingVM(actionId);
    }
}


/**
 * @brief Remove a link between two objects in the mapping
 * @param link
 */
void AgentsMappingController::removeLinkBetweenTwoObjectsInMapping(LinkVM* link)
{
    if ((link != nullptr)
            && (link->inputObject() != nullptr) && (link->linkInput() != nullptr)
            && (link->outputObject() != nullptr) && (link->linkOutput() != nullptr))
    {
        // 2 agents
        if ((link->inputObject()->type() == ObjectInMappingTypes::AGENT) && (link->outputObject()->type() == ObjectInMappingTypes::AGENT))
        {
            AgentInMappingVM* inputAgent = qobject_cast<AgentInMappingVM*>(link->inputObject());
            AgentInMappingVM* outputAgent = qobject_cast<AgentInMappingVM*>(link->outputObject());

            if ((inputAgent != nullptr) && (outputAgent != nullptr))
            {
                qDebug() << "Remove link between 2 AGENTS";

                // Remove the link between two agents from the mapping
                _removeLinkBetweenTwoAgents(link);
            }
        }
        // 2 actions
        else if ((link->inputObject()->type() == ObjectInMappingTypes::ACTION) && (link->outputObject()->type() == ObjectInMappingTypes::ACTION))
        {
            ActionInMappingVM* inputAction = qobject_cast<ActionInMappingVM*>(link->inputObject());
            ActionInMappingVM* outputAction = qobject_cast<ActionInMappingVM*>(link->outputObject());

            if ((inputAction != nullptr) && (outputAction != nullptr))
            {
                qDebug() << "Remove link between 2 ACTIONS";

                // Delete the link between two agents in the mapping
                _deleteLinkBetweenTwoObjectsInMapping(link);
            }
        }
        // From Action to Agent
        else if ((link->outputObject()->type() == ObjectInMappingTypes::ACTION) && (link->inputObject()->type() == ObjectInMappingTypes::AGENT))
        {
            AgentInMappingVM* inputAgent = qobject_cast<AgentInMappingVM*>(link->inputObject());
            ActionInMappingVM* outputAction = qobject_cast<ActionInMappingVM*>(link->outputObject());

            if ((inputAgent != nullptr) && (outputAction != nullptr))
            {
                qDebug() << "Remove link from ACTION to AGENT";

                // Delete the link between two agents in the mapping
                _deleteLinkBetweenTwoObjectsInMapping(link);
            }
        }
        // From Agent to Action
        else if ((link->outputObject()->type() == ObjectInMappingTypes::AGENT) && (link->inputObject()->type() == ObjectInMappingTypes::ACTION))
        {
            ActionInMappingVM* inputAction = qobject_cast<ActionInMappingVM*>(link->inputObject());
            AgentInMappingVM* outputAgent = qobject_cast<AgentInMappingVM*>(link->outputObject());

            if ((inputAction != nullptr) && (outputAgent != nullptr))
            {
                qDebug() << "Remove link from AGENT to ACTION";

                // Delete the link between two agents in the mapping
                _deleteLinkBetweenTwoObjectsInMapping(link);
            }
        }
    }
}


/**
 * @brief Called when an agent from the list is dropped on the current mapping at a position
 * @param agentName
 * @param position
 */
void AgentsMappingController::dropAgentNameToMappingAtPosition(const QString& agentName, QPointF position)
{
    // Check that there is NOT yet an agent in the current mapping for this name
    AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);

    if ((agentInMapping == nullptr) && (_modelManager != nullptr))
    {
        // Get the (view model of) agents grouped for this name
        AgentsGroupedByNameVM* agentsGroupedByName = _modelManager->getAgentsGroupedForName(agentName);
        if (agentsGroupedByName != nullptr)
        {
            // Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position with the default width
            agentInMapping = _createAgentInMappingAtPosition(agentsGroupedByName, position, AgentInMappingVM::DEFAULT_WIDTH);

            if (agentInMapping != nullptr)
            {
                // The global mapping is activated
                if (_modelManager->isMappingConnected())
                {
                    // CONTROL
                    /*if (_modelManager->isMappingControlled())
                    {
                    }
                    // OBSERVE
                    else
                    {
                    }*/

                    // Link the agent in the global mapping on its inputs (add all missing links TO the agent)
                    _linkAgentOnInputs(agentInMapping);
                }
                // The global mapping is NOT activated
                else
                {
                    // Remove all links while the global mapping is UN-activated
                    //agentInMapping->removeAllLinks_WhileMappingWasUNactivated();

                    // Traverse the list of all view models of mapping elements (which our agent is the input agent)
                    for (MappingElementVM* mappingElement : agentsGroupedByName->allMappingElements()->toList())
                    {
                        if ((mappingElement != nullptr) && (mappingElement->firstModel() != nullptr)
                                && !mappingElement->firstModel()->outputAgent().isEmpty()
                                && !mappingElement->firstModel()->output().isEmpty()
                                //&& !mappingElement->firstModel()->inputAgent().isEmpty()
                                && !mappingElement->firstModel()->input().isEmpty())
                        {
                            // Get the link input
                            LinkInputVM* linkInput = _getAloneLinkInputFromName(agentInMapping, mappingElement->firstModel()->input(), mappingElement->name());
                            if (linkInput != nullptr)
                            {
                                QString linkId = "";

                                // Get the output agent in the global mapping from the output agent name
                                AgentInMappingVM* outputAgentInMapping = getAgentInMappingFromName(mappingElement->firstModel()->outputAgent());
                                if (outputAgentInMapping != nullptr)
                                {
                                    // Get the link output
                                    LinkOutputVM* linkOutput = _getAloneLinkOutputFromName(outputAgentInMapping, mappingElement->firstModel()->output(), mappingElement->name());
                                    if (linkOutput != nullptr)
                                    {
                                        // Get the link id (with format "outputAgent##output::outputType-->inputAgent##input::inputType") from agent names and Input/Output ids
                                        linkId = LinkVM::getLinkIdFromObjectIdsAndIOids(outputAgentInMapping->uid(), linkOutput->uid(), agentInMapping->uid(), linkInput->uid());
                                    }
                                }
                                // The output agent is NOT yet in the global mapping
                                else
                                {
                                    // Get the (view model of) agents grouped for the output agent name
                                    AgentsGroupedByNameVM* outputAgent = _modelManager->getAgentsGroupedForName(mappingElement->firstModel()->outputAgent());
                                    if (outputAgent != nullptr)
                                    {
                                        QList<OutputVM*> outputsWithSameName = outputAgent->getOutputsListFromName(mappingElement->firstModel()->output());
                                        if (outputsWithSameName.count() == 1)
                                        {
                                            OutputVM* output = outputsWithSameName.at(0);
                                            if (output != nullptr)
                                            {
                                                // Get the link id (with format "outputAgent##output::outputType-->inputAgent##input::inputType") from agent names and Input/Output ids
                                                linkId = LinkVM::getLinkIdFromObjectIdsAndIOids(outputAgent->name(), output->uid(), agentInMapping->uid(), linkInput->uid());
                                            }
                                        }
                                    }
                                }

                                if (!linkId.isEmpty())
                                {
                                    // Remove the link while the global mapping is UN-activated
                                    agentInMapping->removeLink_WhileMappingWasUNactivated(linkId, mappingElement);
                                }
                            }
                        }
                    }
                }

                // Link the agent in the global mapping on its outputs (add all missing links FROM the agent)
                _linkAgentOnOutputs(agentInMapping);

                // Selects this new agent
                setselectedAgent(agentInMapping);
            }
        }
    }
}


/**
 * @brief Called when an action from the list is dropped on the current mapping at a position
 * @param action
 * @param position
 * @return
 */
void AgentsMappingController::dropActionToMappingAtPosition(ActionM* action, QPointF position)
{
    if (action != nullptr)
    {
        qDebug() << "Drop action" << action->name() << "at" << position;

        // Get an UID for our new view model of action in mapping
        QString uid = IngeScapeUtils::getUIDforNewActionInMappingVM();

        // Create a new view model of action in mapping
        ActionInMappingVM* actionInMapping = _createActionInMappingAtPosition(uid, action, position, ActionInMappingVM::DEFAULT_WIDTH);

        if (actionInMapping != nullptr)
        {
            // Selects this new action
            setselectedAction(actionInMapping);
        }
    }
}


/**
 * @brief Slot called when a link from an output is dropped over an input on the current mapping
 * Or when a link to an input is dropped over an output
 * @param outputAgent
 * @param linkOutput
 * @param inputAgent
 * @param linkInput
 */
void AgentsMappingController::dropLinkBetweenTwoAgents(AgentInMappingVM* outputAgent, LinkOutputVM* linkOutput, AgentInMappingVM* inputAgent, LinkInputVM* linkInput)
{
    if ((outputAgent != nullptr) && (outputAgent->agentsGroupedByName() != nullptr) && (linkOutput != nullptr) && (linkOutput->output() != nullptr)
            && (inputAgent != nullptr) && (inputAgent->agentsGroupedByName() != nullptr) && (linkInput != nullptr) && (linkInput->input() != nullptr))
    {
        // Check that the input can link to the output
        if (linkInput->canLinkWith(linkOutput))
        {
            // Get the link id (with format "outputAgent##output::outputType-->inputAgent##input::inputType") from agent names and Input/Output ids
            QString linkId = LinkVM::getLinkIdFromObjectIdsAndIOids(outputAgent->uid(), linkOutput->uid(), inputAgent->uid(), linkInput->uid());

            LinkVM* link = getLinkInMappingFromId(linkId);

            // Check that the link does NOT exist
            if (link == nullptr)
            {
                // Get the link name (with format "outputAgent##output-->inputAgent##input") from the list of names (each parts of a mapping element)
                QString linkName = MappingElementM::getLinkNameFromNamesList(outputAgent->name(), linkOutput->name(), inputAgent->name(), linkInput->name());

                // The global mapping is activated AND the input agent is ON
                if ((_modelManager != nullptr) && _modelManager->isMappingConnected() && inputAgent->agentsGroupedByName()->isON())
                {
                    // Create a new TEMPORARY link between the two agents
                    link = _createLinkBetweenTwoObjectsInMapping(linkName,
                                                                 outputAgent,
                                                                 linkOutput,
                                                                 inputAgent,
                                                                 linkInput,
                                                                 nullptr,
                                                                 true);

                    if ((link != nullptr) && !_hashFromLinkIdToAddedLink_WaitingReply.contains(link->uid()))
                    {
                        // Insert in the hash table the "(unique) link id" and the link for which we are waiting a reply to the request "add"
                        _hashFromLinkIdToAddedLink_WaitingReply.insert(link->uid(), link);
                    }
                    else {
                        qWarning() << "The request" << command_MapAgents << "has already been sent to add the link" << link->uid();
                    }

                    // Emit signal "Command asked to agent about Mapping Input"
                    Q_EMIT commandAskedToAgentAboutMappingInput(inputAgent->agentsGroupedByName()->peerIdsList(),
                                                                command_MapAgents,
                                                                linkInput->name(),
                                                                outputAgent->name(),
                                                                linkOutput->name());
                }
                // The global mapping is NOT activated OR the input agent is OFF
                else
                {
                    // Create a new (REAL) link between the two agents
                    link = _createLinkBetweenTwoObjectsInMapping(linkName,
                                                                 outputAgent,
                                                                 linkOutput,
                                                                 inputAgent,
                                                                 linkInput,
                                                                 nullptr,
                                                                 false);

                    if (link != nullptr)
                    {
                        MappingElementVM* mappingElement = inputAgent->getRemovedMappingElementFromLinkId_WhileMappingWasUNactivated(linkId);

                        // This link has been removed while the mapping was UN-activated...
                        if (mappingElement != nullptr)
                        {
                            // Update the view model of mapping element which have been cleared
                            link->setmappingElement(mappingElement);

                            // ...just cancel the remove of the link while the global mapping was UN-activated
                            inputAgent->cancelRemoveLink_WhileMappingWasUNactivated(linkId);
                        }
                        // Add the link while the global mapping is UN-activated
                        else
                        {
                            //inputAgent->addLink_WhileMappingWasUNactivated(link->uid(), linkInput->uid(), outputAgent->name(), linkOutput->uid());
                            inputAgent->addLink_WhileMappingWasUNactivated(link->uid(), linkInput->name(), outputAgent->name(), linkOutput->name());
                        }
                    }
                }
            }
            else {
                qWarning() << "The link" << link->uid() << "already exist";
            }
        }
        else
        {
            if ((linkOutput->output()->firstModel() != nullptr) && (linkInput->input()->firstModel() != nullptr))
            {
                qWarning() << "Can not link output" << linkOutput->name() << "with type" << AgentIOPValueTypes::staticEnumToString(linkOutput->output()->firstModel()->agentIOPValueType()) << "(of agent" << outputAgent->name() << ")"
                           << "and input" << linkInput->name() << "with type" << AgentIOPValueTypes::staticEnumToString(linkInput->input()->firstModel()->agentIOPValueType()) << "(of agent" << inputAgent->name() << ")";
            }
        }
    }
}


/**
 * @brief Slot called when a link from an output is dropped over an input on the current mapping
 * Or when a link to an input is dropped over an output
 * @param outputAction
 * @param linkOutput
 * @param inputAction
 * @param linkInput
 */
void AgentsMappingController::dropLinkBetweenTwoActions(ActionInMappingVM* outputAction, LinkOutputVM* linkOutput, ActionInMappingVM* inputAction, LinkInputVM* linkInput)
{
    if ((outputAction != nullptr) && (outputAction->action() != nullptr) && (linkOutput != nullptr)
            && (inputAction != nullptr) && (inputAction->action() != nullptr) && (linkInput != nullptr))
    {
        qDebug() << "drop Link from action" << outputAction->name() << "." << linkOutput->uid() << "to action" << inputAction->name() << "." << linkInput->uid();

        // Get the link name (with format "outputAgent##output-->inputAgent##input") from the list of names (each parts of a mapping element)
        QString linkName = MappingElementM::getLinkNameFromNamesList(outputAction->name(), linkOutput->name(), inputAction->name(), linkInput->name());

        // Create a link between two actions in the mapping
        LinkVM* link = _createLinkBetweenTwoObjectsInMapping(linkName,
                                                             outputAction,
                                                             linkOutput,
                                                             inputAction,
                                                             linkInput);

        if (link != nullptr)
        {
            // Connect to the signal "Activate Input of Object in Mapping" from the link
            connect(link, &LinkVM::activateInputOfObjectInMapping, this, &AgentsMappingController::_onActivateInputOfActionInMapping);
        }
    }
}


/**
 * @brief Slot called when a link from an output is dropped over an input on the current mapping
 * Or when a link to an input is dropped over an output
 * @param outputAction
 * @param linkOutput
 * @param inputAgent
 * @param linkInput
 */
void AgentsMappingController::dropLinkFromActionToAgent(ActionInMappingVM* outputAction, LinkOutputVM* linkOutput, AgentInMappingVM* inputAgent, LinkInputVM* linkInput)
{
    if ((outputAction != nullptr) && (outputAction->action() != nullptr) && (linkOutput != nullptr)
            && (inputAgent != nullptr) && (inputAgent->agentsGroupedByName() != nullptr) && (linkInput != nullptr) && (linkInput->input() != nullptr))
    {
        qDebug() << "drop Link from action" << outputAction->name() << "." << linkOutput->uid() << "to agent" << inputAgent->name() << "." << linkInput->uid();

        // Get the link name (with format "outputAgent##output-->inputAgent##input") from the list of names (each parts of a mapping element)
        QString linkName = MappingElementM::getLinkNameFromNamesList(outputAction->name(), linkOutput->name(), inputAgent->name(), linkInput->name());

        // Create a link between from action to agent in the mapping
        LinkVM* link = _createLinkBetweenTwoObjectsInMapping(linkName,
                                                             outputAction,
                                                             linkOutput,
                                                             inputAgent,
                                                             linkInput);

        if (link != nullptr)
        {
            // Connect to the signal "Activate Input of Object in Mapping" from the link
            connect(link, &LinkVM::activateInputOfObjectInMapping, this, &AgentsMappingController::_onWriteOnInputOfAgentInMapping);
        }
    }
}


/**
 * @brief Slot called when a link from an output is dropped over an input on the current mapping
 * Or when a link to an input is dropped over an output
 * @param outputAgent
 * @param linkOutput
 * @param inputAction
 * @param linkInput
 */
void AgentsMappingController::dropLinkFromAgentToAction(AgentInMappingVM* outputAgent, LinkOutputVM* linkOutput, ActionInMappingVM* inputAction, LinkInputVM* linkInput)
{
    if ((outputAgent != nullptr) && (outputAgent->agentsGroupedByName() != nullptr) && (linkOutput != nullptr) && (linkOutput->output() != nullptr)
            && (inputAction != nullptr) && (inputAction->action() != nullptr) && (linkInput != nullptr))
    {
        qDebug() << "drop Link from agent" << outputAgent->name() << "." << linkOutput->uid() << "to action" << inputAction->name() << "." << linkInput->uid();

        // Get the link name (with format "outputAgent##output-->inputAgent##input") from the list of names (each parts of a mapping element)
        QString linkName = MappingElementM::getLinkNameFromNamesList(outputAgent->name(), linkOutput->name(), inputAction->name(), linkInput->name());

        // Create a link between from agent to action in the mapping
        LinkVM* link = _createLinkBetweenTwoObjectsInMapping(linkName,
                                                             outputAgent,
                                                             linkOutput,
                                                             inputAction,
                                                             linkInput);

        if (link != nullptr)
        {
            // Connect to the signal "Activate Input of Object in Mapping" from the link
            connect(link, &LinkVM::activateInputOfObjectInMapping, this, &AgentsMappingController::_onActivateInputOfActionInMapping);
        }
    }
}


/**
 * @brief Get the (view model of) agent in the global mapping from an agent name
 * @param name
 * @return
 */
AgentInMappingVM* AgentsMappingController::getAgentInMappingFromName(const QString& name)
{
    return _hashFromNameToAgentInMapping.value(name, nullptr);
}


/**
 * @brief Get the (view model of) action in the global mapping from a unique id
 * @param uid
 * @return
 */
ActionInMappingVM* AgentsMappingController::getActionInMappingFromUid(const QString& uid)
{
    return _hashFromUidToActionInMapping.value(uid, nullptr);
}


/**
 * @brief Get the list of (view model of) links between agents in the global mapping from a link name
 * @param linkName
 * @return
 */
QList<LinkVM*> AgentsMappingController::getLinksInMappingFromName(const QString& linkName)
{
    return _hashFromNameToListOfLinksInMapping.value(linkName, QList<LinkVM*>());
}


/**
 * @brief Get the (view model of) link between agents in the global mapping from a link id
 * @param linkId
 * @return
 */
LinkVM* AgentsMappingController::getLinkInMappingFromId(const QString& linkId)
{
    return _hashFromIdToLinkInMapping.value(linkId, nullptr);
}


/**
 * @brief Check if an action has been inserted in the global mapping
 * @param actionM
 * @return
 */
bool AgentsMappingController::isActionInsertedInMapping(ActionM* actionM)
{
    auto isActionM = [actionM](ActionInMappingVM* actionInMapping) {
        return (actionInMapping != nullptr) && (actionInMapping->action() == actionM);
    };

    return (actionM != nullptr) && std::any_of(_allActionsInMapping.begin(), _allActionsInMapping.end(), isActionM);
}


/**
 * @brief Export the global mapping (of agents) into JSON
 * @return array of all agents and their mapping
 */
QJsonArray AgentsMappingController::exportGlobalMappingToJSON()
{
    QJsonArray jsonArray;

    if ((_jsonHelper != nullptr) && (_modelManager != nullptr))
    {
        // Copy the list of links
        QList<LinkVM*> linksList = _allLinksInMapping.toList();

        // List of agents and their mapping
        for (AgentInMappingVM* agentInMapping : _allAgentsInMapping)
        {
            if ((agentInMapping != nullptr) && (agentInMapping->agentsGroupedByName() != nullptr)
                    && (agentInMapping->agentsGroupedByName()->currentMapping() != nullptr))
            {
                QJsonObject jsonAgent;

                // Set the agent name
                jsonAgent.insert("agentName", agentInMapping->name());

                // Set the position
                QString position = QString("%1, %2").arg(QString::number(agentInMapping->position().x()), QString::number(agentInMapping->position().y()));
                jsonAgent.insert("position", position);

                // Set the width
                jsonAgent.insert("width", agentInMapping->width());


                //
                // Set the mapping
                //
                QJsonObject jsonMapping = QJsonObject();

                // The global mapping is activated AND the agent is ON
                if (_modelManager->isMappingConnected() && agentInMapping->agentsGroupedByName()->isON())
                {
                    // Export the current mapping
                    jsonMapping = _jsonHelper->exportAgentMappingToJson(agentInMapping->agentsGroupedByName()->currentMapping());
                }
                // The global mapping is NOT activated OR the agent is OFF
                else
                {
                    if (agentInMapping->hadLinksAdded_WhileMappingWasUNactivated() || agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated())
                    {
                        // Get the list of all added mapping elements while the global mapping was UN-activated
                        QList<MappingElementM*> addedMappingElements = agentInMapping->getAddedMappingElements_WhileMappingWasUNactivated();

                        // Get the list of all names of removed mapping elements while the global mapping was UN-activated
                        QStringList namesOfRemovedMappingElements = agentInMapping->getNamesOfRemovedMappingElements_WhileMappingWasUNactivated();

                        // Export the current mapping with changes (applied while the global mapping was UN-activated) into a JSON objec
                        jsonMapping = _jsonHelper->exportAgentMappingWithChangesToJson(agentInMapping->agentsGroupedByName()->currentMapping(),
                                                                                       addedMappingElements,
                                                                                       namesOfRemovedMappingElements);
                    }
                    else
                    {
                        // Export the current mapping
                        jsonMapping = _jsonHelper->exportAgentMappingToJson(agentInMapping->agentsGroupedByName()->currentMapping());
                    }
                }
                jsonAgent.insert("mapping", jsonMapping);


                //
                // Set the actions mapping
                //
                QJsonArray jsonActionsMapping = QJsonArray();

                // Make a copy because the list can be modified inside loop "for"
                QList<LinkVM*> copy = QList<LinkVM*>(linksList);
                for (LinkVM* link : copy)
                {
                    if ((link != nullptr) && (link->inputObject() != nullptr) && (link->inputObject() == agentInMapping)
                            && (link->outputObject() != nullptr))
                    {
                        QJsonObject jsonMappingElement;

                        jsonMappingElement.insert("input_name", link->linkInput()->name());

                        jsonMappingElement.insert("output_name", link->linkOutput()->name());

                        // From Action to Agent
                        if (link->outputObject()->type() == ObjectInMappingTypes::ACTION)
                        {
                            jsonMappingElement.insert("output_action_name", link->outputObject()->name());

                            ActionInMappingVM* outputAction = qobject_cast<ActionInMappingVM*>(link->outputObject());
                            if ((outputAction != nullptr) && (outputAction->action() != nullptr))
                            {
                                jsonMappingElement.insert("output_actionInMapping_id", outputAction->uid());
                                //jsonMappingElement.insert("output_action_id", outputAction->action()->uid());
                            }
                        }
                        // From Agent to Agent
                        //else if (link->outputObject()->type() == ObjectInMappingTypes::AGENT)
                        //{
                        //}

                        jsonActionsMapping.append(jsonMappingElement);

                        // Remove from the list to optimize next traversals
                        linksList.removeOne(link);
                    }
                }

                jsonAgent.insert("actions_mapping", jsonActionsMapping);

                jsonArray.append(jsonAgent);
            }
        }

        // List of actions and their mapping
        for (ActionInMappingVM* actionInMapping : _allActionsInMapping)
        {
            if ((actionInMapping != nullptr) && (actionInMapping->action() != nullptr))
            {
                QJsonObject jsonAction;

                // Set the action name
                jsonAction.insert("action_name", actionInMapping->name());
                //jsonAction.insert("action_name", actionInMapping->action()->name());

                // Set the action id and the unique id of the action in the mapping
                jsonAction.insert("action_id", actionInMapping->action()->uid());
                jsonAction.insert("actionInMapping_id", actionInMapping->uid());

                // Set the position
                QString position = QString("%1, %2").arg(QString::number(actionInMapping->position().x()), QString::number(actionInMapping->position().y()));
                jsonAction.insert("position", position);

                // Set the width
                jsonAction.insert("width", actionInMapping->width());


                //
                // Set the actions mapping
                //
                QJsonArray jsonActionsMapping = QJsonArray();

                // Make a copy because the list can be modified inside loop "for"
                QList<LinkVM*> copy = QList<LinkVM*>(linksList);
                for (LinkVM* link : copy)
                {
                    if ((link != nullptr) && (link->inputObject() != nullptr) && (link->inputObject() == actionInMapping)
                            && (link->outputObject() != nullptr))
                    {
                        QJsonObject jsonMappingElement;

                        jsonMappingElement.insert("input_name", link->linkInput()->name());

                        jsonMappingElement.insert("output_name", link->linkOutput()->name());

                        // From Agent to Action
                        if (link->outputObject()->type() == ObjectInMappingTypes::AGENT)
                        {
                            jsonMappingElement.insert("output_agent_name", link->outputObject()->name());
                        }
                        // From Action to Action
                        else if (link->outputObject()->type() == ObjectInMappingTypes::ACTION)
                        {
                            jsonMappingElement.insert("output_action_name", link->outputObject()->name());

                            ActionInMappingVM* outputAction = qobject_cast<ActionInMappingVM*>(link->outputObject());
                            if ((outputAction != nullptr) && (outputAction->action() != nullptr))
                            {
                                jsonMappingElement.insert("output_actionInMapping_id", outputAction->uid());
                                //jsonMappingElement.insert("output_action_id", outputAction->action()->uid());
                            }
                        }

                        jsonActionsMapping.append(jsonMappingElement);

                        // Remove from the list to optimize next traversals
                        linksList.removeOne(link);
                    }
                }

                jsonAction.insert("actions_mapping", jsonActionsMapping);

                jsonArray.append(jsonAction);
            }
        }
    }
    return jsonArray;
}


/**
 * @brief Import the global mapping (of agents) from JSON
 * @param jsonArrayOfAgentsInMapping
 */
void AgentsMappingController::importMappingFromJson(QJsonArray jsonArrayOfAgentsInMapping)
{
    if ((_modelManager != nullptr) && (_jsonHelper != nullptr))
    {
        QList<QPair<AgentInMappingVM*, AgentMappingM*>> listOfAgentsAndMappingToAgents;
        QList<QPair<AgentInMappingVM*, ActionMappingM*>> listOfAgentsAndMappingToActions;
        QList<QPair<ActionInMappingVM*, ActionMappingM*>> listOfActionsAndMappingToAgentsAndActions;

        for (QJsonValue jsonValue : jsonArrayOfAgentsInMapping)
        {
            if (jsonValue.isObject())
            {
                QJsonObject jsonObjectInMapping = jsonValue.toObject();

                //
                // Position
                //
                // Get the value for key "position"
                QJsonValue jsonPosition = jsonObjectInMapping.value("position");

                QPointF position = QPointF();

                if (jsonPosition.isString())
                {
                    QStringList positionStringList = jsonPosition.toString().split(", ");
                    if (positionStringList.count() == 2)
                    {
                        QString strX = positionStringList.at(0);
                        QString strY = positionStringList.at(1);

                        if (!strX.isEmpty() && !strY.isEmpty()) {
                            position = QPointF(static_cast<qreal>(strX.toFloat()), static_cast<qreal>(strY.toFloat()));
                        }
                    }
                }


                //
                // Actions Mapping
                //
                // Get the value for key "actions_mapping"
                QJsonValue jsonActionsMapping = jsonObjectInMapping.value("actions_mapping");


                //
                // Agent
                //
                if (jsonObjectInMapping.contains("agentName"))
                {
                    // Get values for key "agentName" and "mapping"
                    QJsonValue jsonName = jsonObjectInMapping.value("agentName");
                    QJsonValue jsonMapping = jsonObjectInMapping.value("mapping");

                    // Get the value for key "width"
                    QJsonValue jsonWidth = jsonObjectInMapping.value("width");
                    qreal width = jsonWidth.toDouble(AgentInMappingVM::DEFAULT_WIDTH);

                    if (jsonName.isString() && jsonMapping.isObject())
                    {
                        QString agentName = jsonName.toString();

                        // Create the agent mapping from JSON
                        AgentMappingM* agentMapping = _jsonHelper->createModelOfAgentMappingFromJSON(agentName, jsonMapping.toObject());


                        //
                        // Actions Mapping
                        //
                        ActionMappingM* actionMapping = nullptr;

                        if (jsonActionsMapping.isArray())
                        {
                            QStringList mappingIdsList_FromActionToAgent = QStringList();

                            // Traverse the list of mapping elements
                            for (QJsonValue jsonIterator : jsonActionsMapping.toArray())
                            {
                                QJsonObject jsonMappingElement = jsonIterator.toObject();

                                // Action
                                if (jsonMappingElement.contains("output_action_name"))
                                {
                                    QJsonValue jsonInputName = jsonMappingElement.value("input_name");

                                    QJsonValue jsonOutputActionName = jsonMappingElement.value("output_action_name");
                                    QJsonValue jsonOutputName = jsonMappingElement.value("output_name");

                                    //QJsonValue jsonOutputActionUID = jsonMappingElement.value("output_action_id");
                                    QJsonValue jsonOutputActionInMappingUID = jsonMappingElement.value("output_actionInMapping_id");

                                    //qDebug() << "Link Action" << jsonOutputActionName.toString() << "(" << jsonOutputActionInMappingUID.toInt() << ")" << "to Agent" << agentName << "." << jsonInputName.toString();

                                    // outputObjectInMapping##output-->inputObjectInMapping##input
                                    QString mappingId_FromActionToAgent = ActionMappingM::getMappingIdFromUIDsList(jsonOutputActionInMappingUID.toString(),
                                                                                                                   jsonOutputName.toString(),
                                                                                                                   agentName,
                                                                                                                   jsonInputName.toString());

                                    mappingIdsList_FromActionToAgent.append(mappingId_FromActionToAgent);
                                }
                            }

                            if (!mappingIdsList_FromActionToAgent.isEmpty())
                            {
                                actionMapping = new ActionMappingM(this);
                                actionMapping->setmappingIdsList_FromActionToAgent(mappingIdsList_FromActionToAgent);
                            }
                        }


                        // Get the (view model of) agents grouped for this name
                        AgentsGroupedByNameVM* agentsGroupedByName = _modelManager->getAgentsGroupedForName(agentName);

                        if ((agentsGroupedByName != nullptr) && !position.isNull())
                        {
                            //qDebug() << "Position:" << position.x() << position.y() << "is defined for" << agentName << "with" << agentsGroupedByName->models()->count() << "models";

                            // Create a new agent in the global mapping (with the "Agents Grouped by Name") at a specific position
                            AgentInMappingVM* agentInMapping = _createAgentInMappingAtPosition(agentsGroupedByName, position, width);


                            // If there are some mapping elements, save the pair [agent, its mapping] in the list
                            // We will create the corresponding links when all agents would have been added to the global mapping
                            if ((agentInMapping != nullptr) && (agentMapping != nullptr) && !agentMapping->mappingElements()->isEmpty())
                            {
                                QPair<AgentInMappingVM*, AgentMappingM*> pair = QPair<AgentInMappingVM*, AgentMappingM*>(agentInMapping, agentMapping);

                                listOfAgentsAndMappingToAgents.append(pair);
                            }


                            // If there are some mapping to actions, save the pair [agent, action mapping] in the list
                            // We will create the corresponding links when all agents and all actions would have been added to the global mapping
                            if ((agentInMapping != nullptr) && (actionMapping != nullptr))
                            {
                                QPair<AgentInMappingVM*, ActionMappingM*> pair = QPair<AgentInMappingVM*, ActionMappingM*>(agentInMapping, actionMapping);

                                listOfAgentsAndMappingToActions.append(pair);
                            }
                        }

                    }
                }
                //
                // Action
                //
                else if (jsonObjectInMapping.contains("action_name"))
                {
                    // Get value for key "action_name"
                    QJsonValue jsonActionName = jsonObjectInMapping.value("action_name");

                    QJsonValue jsonActionUID = jsonObjectInMapping.value("action_id");
                    QJsonValue jsonActionInMappingUID = jsonObjectInMapping.value("actionInMapping_id");

                    // Get the value for key "width"
                    QJsonValue jsonWidth = jsonObjectInMapping.value("width");
                    qreal width = jsonWidth.toDouble(ActionInMappingVM::DEFAULT_WIDTH);

                    if (jsonActionName.isString() && jsonActionUID.isDouble() && jsonActionInMappingUID.isString())
                    {
                        QString actionName = jsonActionName.toString();
                        int actionUID = jsonActionUID.toInt();
                        QString actionInMappingUID = jsonActionInMappingUID.toString();

                        //
                        // Actions Mapping
                        //
                        ActionMappingM* actionMapping = nullptr;

                        if (jsonActionsMapping.isArray())
                        {
                            QStringList mappingIdsList_FromAgentToAction = QStringList();
                            QStringList uidsListOfOutputActionsInMapping = QStringList();

                            // Traverse the list of mapping elements
                            for (QJsonValue jsonIterator : jsonActionsMapping.toArray())
                            {
                                QJsonObject jsonMappingElement = jsonIterator.toObject();

                                // Agent
                                if (jsonMappingElement.contains("output_agent_name"))
                                {
                                    QJsonValue jsonOutputAgentName = jsonMappingElement.value("output_agent_name");
                                    QJsonValue jsonOutputName = jsonMappingElement.value("output_name");

                                    QJsonValue jsonInputName = jsonMappingElement.value("input_name");

                                    //qDebug() << "Link Agent" << jsonOutputAgentName.toString() << "." << jsonOutputName.toString() << "to Action" << actionName;

                                    // outputObjectInMapping##output-->inputObjectInMapping##input
                                    QString mappingId_FromAgentToAction = ActionMappingM::getMappingIdFromUIDsList(jsonOutputAgentName.toString(),
                                                                                                                   jsonOutputName.toString(),
                                                                                                                   actionInMappingUID,
                                                                                                                   jsonInputName.toString());

                                    mappingIdsList_FromAgentToAction.append(mappingId_FromAgentToAction);
                                }
                                // Action
                                else if (jsonMappingElement.contains("output_action_name"))
                                {
                                    QJsonValue jsonOutputActionName = jsonMappingElement.value("output_action_name");
                                    //QJsonValue jsonOutputName = jsonMappingElement.value("output_name");

                                    //QJsonValue jsonOutputActionUID = jsonMappingElement.value("output_action_id");
                                    QJsonValue jsonOutputActionInMappingUID = jsonMappingElement.value("output_actionInMapping_id");

                                    //qDebug() << "Link Action" << jsonOutputActionName.toString() << "(" << jsonOutputActionInMappingUID.toInt() << ")" << "to Action" << actionName;

                                    uidsListOfOutputActionsInMapping.append(jsonOutputActionInMappingUID.toString());
                                }
                                else {
                                    qCritical() << "The JSON mapping element for action" << actionName << "is bad formatted !";
                                }
                            }
                            // If at least, one list is not empty
                            if (!mappingIdsList_FromAgentToAction.isEmpty() || !uidsListOfOutputActionsInMapping.isEmpty())
                            {
                                actionMapping = new ActionMappingM(this);
                                actionMapping->setmappingIdsList_FromAgentToAction(mappingIdsList_FromAgentToAction);
                                actionMapping->setuidsListOfOutputActionsInMapping(uidsListOfOutputActionsInMapping);
                            }
                        }

                        // Get the (model of) action for this name
                        ActionM* action = _modelManager->getActionWithId(actionUID);

                        if ((action != nullptr) && !position.isNull())
                        {
                            qDebug() << "Position:" << position.x() << position.y() << "is defined for" << actionName << "with" << action;

                            // Get an UID for our new view model of action in mapping
                            IngeScapeUtils::bookUIDforActionInMappingVM(actionInMappingUID);

                            // Create a new action in the global mapping with a unique id, with a model of action and at a specific position
                            ActionInMappingVM* actionInMapping = _createActionInMappingAtPosition(actionInMappingUID, action, position, width);


                            // If there are some mapping to agents and actions, save the pair [action, action mapping] in the list
                            // We will create the corresponding links when all agents and all actions would have been added to the global mapping
                            if ((actionInMapping != nullptr) && (actionMapping != nullptr))
                            {
                                QPair<ActionInMappingVM*, ActionMappingM*> pair = QPair<ActionInMappingVM*, ActionMappingM*>(actionInMapping, actionMapping);

                                listOfActionsAndMappingToAgentsAndActions.append(pair);
                            }
                        }
                    }
                }
            }
        }


        //
        // All agents have been been added to the global mapping, we can create their links...
        //
        for (QPair<AgentInMappingVM*, AgentMappingM*> pair : listOfAgentsAndMappingToAgents)
        {
            AgentInMappingVM* inputAgent = pair.first;
            AgentMappingM* agentMapping = pair.second;

            if ((agentMapping != nullptr) && (inputAgent != nullptr))
            {
                for (MappingElementM* mappingElement : agentMapping->mappingElements()->toList())
                {
                    if (mappingElement != nullptr)
                    {
                        // Get the output agent in the global mapping from the output agent name
                        AgentInMappingVM* outputAgent = getAgentInMappingFromName(mappingElement->outputAgent());
                        if (outputAgent != nullptr)
                        {
                            // Get the link input and the link output
                            LinkInputVM* linkInput = _getAloneLinkInputFromName(inputAgent, mappingElement->input(), mappingElement->name());
                            LinkOutputVM* linkOutput = _getAloneLinkOutputFromName(outputAgent, mappingElement->output(), mappingElement->name());

                            if ((linkInput != nullptr) && (linkOutput != nullptr))
                            {
                                // Simulate a drop of the link between the two agents
                                dropLinkBetweenTwoAgents(outputAgent, linkOutput, inputAgent, linkInput);
                            }
                        }
                    }
                }
            }
        }


        //
        // All agents and all actions have been been added to the global mapping, we can create their links...
        //
        for (QPair<AgentInMappingVM*, ActionMappingM*> pair : listOfAgentsAndMappingToActions)
        {
            AgentInMappingVM* inputAgent = pair.first;
            ActionMappingM* actionMapping = pair.second;

            if ((actionMapping != nullptr) && (inputAgent != nullptr))
            {
                for (QString mappingId_FromActionToAgent : actionMapping->mappingIdsList_FromActionToAgent())
                {
                    // Get the list of UIDs (each parts of a mapping) from the mapping id (with format "outputObjectInMapping##output-->inputObjectInMapping##input")
                    QStringList uidsList = ActionMappingM::getUIDsListFromMappingId(mappingId_FromActionToAgent);
                    if (uidsList.count() == 4)
                    {
                        QString outputObjectInMappingUID = uidsList.at(0);
                        QString outputUID = uidsList.at(1);
                        QString inputObjectInMappingUID = uidsList.at(2);
                        QString inputUID = uidsList.at(3);

                        // Get the (view model of) action in the global mapping from this unique id
                        ActionInMappingVM* outputAction = getActionInMappingFromUid(outputObjectInMappingUID);

                        // Get the link input
                        LinkInputVM* linkInput = _getAloneLinkInputFromName(inputAgent, inputUID, mappingId_FromActionToAgent);

                        if ((outputAction != nullptr) && (outputAction->linkOutput() != nullptr) && (linkInput != nullptr))
                        {
                            // Simulate a drop of a link between these 2 actions
                            dropLinkFromActionToAgent(outputAction, outputAction->linkOutput(), inputAgent, linkInput);
                        }
                    }
                }
            }

            // Free memory
            delete actionMapping;
        }
        listOfAgentsAndMappingToActions.clear();


        //
        // All agents and all actions have been been added to the global mapping, we can create their links...
        //
        for (QPair<ActionInMappingVM*, ActionMappingM*> pair : listOfActionsAndMappingToAgentsAndActions)
        {
            ActionInMappingVM* inputAction = pair.first;
            ActionMappingM* actionMapping = pair.second;

            if ((actionMapping != nullptr) && (inputAction != nullptr) && (inputAction->linkInput() != nullptr))
            {
                for (QString mappingId_FromAgentToAction : actionMapping->mappingIdsList_FromAgentToAction())
                {
                    // Get the list of names (each parts of a mapping) from the mapping id (with format "outputObjectInMapping##output-->inputObjectInMapping##input")
                    QStringList uidsList = ActionMappingM::getUIDsListFromMappingId(mappingId_FromAgentToAction);
                    if (uidsList.count() == 4)
                    {
                        QString outputObjectInMappingUID = uidsList.at(0);
                        QString outputUID = uidsList.at(1);
                        QString inputObjectInMappingUID = uidsList.at(2);
                        QString inputUID = uidsList.at(3);

                        // Get the output agent in the global mapping from the output agent name
                        AgentInMappingVM* outputAgent = getAgentInMappingFromName(outputObjectInMappingUID);
                        if (outputAgent != nullptr)
                        {
                            // Get the link output
                            LinkOutputVM* linkOutput = _getAloneLinkOutputFromName(outputAgent, outputUID, mappingId_FromAgentToAction);
                            if (linkOutput != nullptr)
                            {
                                // Simulate a drop of the link from the agent to the action
                                dropLinkFromAgentToAction(outputAgent, linkOutput, inputAction, inputAction->linkInput());
                            }
                        }
                    }
                }

                for (QString uidOfOutputActionInMapping : actionMapping->uidsListOfOutputActionsInMapping())
                {
                    ActionInMappingVM* outputAction = getActionInMappingFromUid(uidOfOutputActionInMapping);
                    if ((outputAction != nullptr) && (outputAction->linkOutput() != nullptr))
                    {
                        // Simulate a drop of a link between these 2 actions
                        dropLinkBetweenTwoActions(outputAction, outputAction->linkOutput(), inputAction, inputAction->linkInput());
                    }
                }
            }

            // Free memory
            delete actionMapping;
        }
        listOfActionsAndMappingToAgentsAndActions.clear();
    }
}


/**
 * @brief Reset the modifications made while the mapping was UN-activated
 */
void AgentsMappingController::resetModificationsWhileMappingWasUNactivated()
{
    qDebug() << "Reset the modifications made while the mapping was UN-activated";

    for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
    {
        if ((agentInMapping != nullptr) && agentInMapping->hadLinksAdded_WhileMappingWasUNactivated())
        {
            // Get the list of all added link Ids while the global mapping was UN-activated
            QList<QString> addedLinkIds = agentInMapping->getAddedLinkIds_WhileMappingWasUNactivated();
            for (QString linkId : addedLinkIds)
            {
                qDebug() << "Remove added link" << linkId << "while the mapping was disconnected";

                // Get the view model of link which corresponds to a mapping element
                LinkVM* link = getLinkInMappingFromId(linkId);
                if (link != nullptr)
                {
                    // Delete this link (between two agents) to cancel the add
                    _deleteLinkBetweenTwoObjectsInMapping(link);
                }
            }

            // Cancel all added links while the global mapping was UN-activated
            agentInMapping->cancelAllAddedLinks_WhileMappingWasUNactivated();
        }
    }


    for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
    {
        if ((agentInMapping != nullptr) && agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated())
        {
            // Get the list of all (view models of) removed mapping elements while the global mapping was UN-activated
            QList<MappingElementVM*> removedMappingElements = agentInMapping->getRemovedMappingElements_WhileMappingWasUNactivated();
            for (MappingElementVM* mappingElement : removedMappingElements)
            {
                if (mappingElement != nullptr)
                {
                    qDebug() << "Add removed link" << mappingElement->name() << "while the mapping was disconnected";

                    // Link the agent (in the global mapping) on its input from the mapping element (add a missing link TO the agent)
                    _linkAgentOnInputFromMappingElement(agentInMapping, mappingElement);
                }
            }

            // Cancel all removed links while the global mapping was UN-activated
            agentInMapping->cancelAllRemovedLinks_WhileMappingWasUNactivated();
        }
    }


    // Update the global mapping with agents ON and their links
    _updateMappingWithAgentsONandLinks();
}


/**
 * @brief Slot called when the flag "is Mapping Connected" changed
 * @param isMappingConnected
 */
void AgentsMappingController::onIsMappingConnectedChanged(bool isMappingConnected)
{
    if ((_modelManager != nullptr) && (_jsonHelper != nullptr) && isMappingConnected)
    {
        // CONTROL
        if (_modelManager->isMappingControlled())
        {
            qDebug() << "Mapping Activated in mode CONTROL";

            // Apply all current mappings with changes applied while the mapping was UN-activated
            for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
            {
                // Usefull only for agents ON
                if ((agentInMapping != nullptr) && (agentInMapping->agentsGroupedByName() != nullptr) && agentInMapping->agentsGroupedByName()->isON())
                {
                    // Get the JSON of the mapping of the agent as displayed in the global mapping
                    QString jsonOfMapping = _getJSONofMappingOfAgentInGlobalMapping(agentInMapping);

                    QString command = QString("%1%2").arg(command_LoadMapping, jsonOfMapping);

                    // Emit signal "Command asked to agent"
                    Q_EMIT commandAskedToAgent(agentInMapping->agentsGroupedByName()->peerIdsList(), command);
                }
                // Nothing to do for agents OFF
            }

            // FIXME Usefull ?
            /*// Cancel all changes made while the mapping was UN-activated
            for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
            {
                if (agentInMapping != nullptr)
                {
                    if (agentInMapping->hadLinksAdded_WhileMappingWasUNactivated())
                    {
                        // Cancel all added links while the global mapping was UN-activated
                        agentInMapping->cancelAllAddedLinks_WhileMappingWasUNactivated();
                    }
                    if (agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated())
                    {
                        // Cancel all removed links while the global mapping was UN-activated
                        agentInMapping->cancelAllRemovedLinks_WhileMappingWasUNactivated();
                    }
                }
            }*/
        }
        // OBSERVE
        else
        {
            qDebug() << "Mapping Activated in mode OBSERVE";

            bool isAddedOrRemovedLink_WhileMappingWasUNactivated = false;
            for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
            {
                if ((agentInMapping != nullptr)
                        && (agentInMapping->hadLinksAdded_WhileMappingWasUNactivated() || agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated()) )
                {
                    isAddedOrRemovedLink_WhileMappingWasUNactivated = true;
                    break;
                }
            }

            // There have been changes in the mapping while the mapping was UN-activated
            if (isAddedOrRemovedLink_WhileMappingWasUNactivated)
            {
                qDebug() << "There have been changes in the mapping while the mapping was UN-activated. Force to CONTROL ?";

                // The mapping has been modified but will be lost if the user stay in mode OBSERVE
                Q_EMIT changesOnLinksWhileMappingUnactivated();
            }
            else
            {
                // Update the global mapping with agents ON and their links
                _updateMappingWithAgentsONandLinks();
            }
        }
    }
}


/**
 * @brief Slot called when the flag "is Mapping Controlled" changed
 * @param isMappingControlled
 */
void AgentsMappingController::onIsMappingControlledChanged(bool isMappingControlled)
{
    qDebug() << "AgentsMappingController: Is Mapping Controlled Changed to" << isMappingControlled;
}


/**
 * @brief Slot called when a new view model of agents grouped by name has been created
 * @param agentsGroupedByName
 */
void AgentsMappingController::onAgentsGroupedByNameHasBeenCreated(AgentsGroupedByNameVM* agentsGroupedByName)
{
    if (agentsGroupedByName != nullptr)
    {
        // Connect to signals from this new agents grouped by name
        connect(agentsGroupedByName, &AgentsGroupedByNameVM::isONChanged, this, &AgentsMappingController::_onAgentIsONChanged);
        connect(agentsGroupedByName, &AgentsGroupedByNameVM::agentModelONhasBeenAdded, this, &AgentsMappingController::_onAgentModelONhasBeenAdded);
        connect(agentsGroupedByName, &AgentsGroupedByNameVM::mappingElementsHaveBeenAdded, this, &AgentsMappingController::_onMappingElementsHaveBeenAdded);
        connect(agentsGroupedByName, &AgentsGroupedByNameVM::mappingElementsWillBeRemoved, this, &AgentsMappingController::_onMappingElementsWillBeRemoved);
    }
}


/**
 * @brief Slot called when a view model of agents grouped by name will be deleted
 * @param agentsGroupedByName
 */
void AgentsMappingController::onAgentsGroupedByNameWillBeDeleted(AgentsGroupedByNameVM* agentsGroupedByName)
{
    if (agentsGroupedByName != nullptr)
    {
        // DIS-connect to signals from this old agents grouped by name
        disconnect(agentsGroupedByName, nullptr, this, nullptr);

        // Get the agent in mapping for the agent name
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentsGroupedByName->name());
        if (agentInMapping != nullptr)
        {
            // Delete this agent in the mapping
            deleteAgentInMapping(agentInMapping);
        }
    }
}


/**
 * @brief Slot called when a model of action will be deleted
 * @param action
 */
void AgentsMappingController::onActionModelWillBeDeleted(ActionM* action)
{
    if (action != nullptr)
    {
        // Make a copy of the list
        for (ActionInMappingVM* actionInMapping : _allActionsInMapping.toList())
        {
            if ((actionInMapping != nullptr) && (actionInMapping->action() == action))
            {
                // Remove the action from the mapping and delete the view model
                deleteActionInMapping(actionInMapping);
            }
        }
    }
}


/**
 * @brief Slot called when we receive the command "highlight link" from a recorder
 * @param parameters
 */
void AgentsMappingController::onHighlightLink(const QStringList& parameters)
{
    if (parameters.count() == 4)
    {
        QString inputAgentName = parameters.at(0);
        QString inputName = parameters.at(1);
        QString outputAgentName = parameters.at(2);
        QString outputName = parameters.at(3);

        // Get the link name (with format "outputAgent##output-->inputAgent##input") from the list of names (each parts of a mapping element)
        QString linkName = MappingElementM::getLinkNameFromNamesList(outputAgentName, outputName, inputAgentName, inputName);

        // Get the list of links between agents in the global mapping from a link name
        QList<LinkVM*> linksWithSameName = getLinksInMappingFromName(linkName);

        if (linksWithSameName.count() == 1)
        {
            LinkVM* link = linksWithSameName.at(0);
            if ((link != nullptr) && (link->linkOutput() != nullptr))
            {
                qDebug() << "Highlight the link" << linkName;

                // Activate the link: allows to highlight the corresponding links
                link->linkOutput()->activate();
            }
        }
    }
}


/**
 * @brief Slot called when the list of all "Agents in Mapping" changed
 */
void AgentsMappingController::_onAllAgentsInMappingChanged()
{
    // Update the flag "is Empty Mapping"
    setisEmptyMapping(_allAgentsInMapping.isEmpty());
}


/**
 * @brief Slot called when the flag "is ON" of an agent(s grouped by name) changed
 * @param isON
 */
void AgentsMappingController::_onAgentIsONChanged(bool isON)
{
    if (isON)
    {
        AgentsGroupedByNameVM* agentsGroupedByName = qobject_cast<AgentsGroupedByNameVM*>(sender());
        if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty())
        {
            AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentsGroupedByName->name());
            if ((agentInMapping != nullptr) && (_modelManager != nullptr))
            {
                // The mapping is activated
                if (_modelManager->isMappingConnected())
                {
                    // OBSERVE
                    if (!_modelManager->isMappingControlled())
                    {
                        if (agentInMapping->hadLinksAdded_WhileMappingWasUNactivated())
                        {
                            // Get the list of all added link Ids while the global mapping was UN-activated
                            QList<QString> addedLinkIds = agentInMapping->getAddedLinkIds_WhileMappingWasUNactivated();
                            for (QString linkId : addedLinkIds)
                            {
                                qDebug() << "Remove added link" << linkId << "while the mapping was disconnected";

                                // Get the view model of link which corresponds to a mapping element
                                LinkVM* link = getLinkInMappingFromId(linkId);
                                if (link != nullptr)
                                {
                                    // Delete this link (between two agents) to cancel the add
                                    _deleteLinkBetweenTwoObjectsInMapping(link);
                                }
                            }

                            // Cancel all added links while the global mapping was UN-activated
                            agentInMapping->cancelAllAddedLinks_WhileMappingWasUNactivated();
                        }

                        // USELESS
                        /*if (agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated())
                        {
                            // Get the list of all (view models of) removed mapping elements while the global mapping was UN-activated
                            QList<MappingElementVM*> removedMappingElements = agentInMapping->getRemovedMappingElements_WhileMappingWasUNactivated();
                            for (MappingElementVM* mappingElement : removedMappingElements)
                            {
                                if (mappingElement != nullptr)
                                {
                                    qDebug() << "Add removed link" << mappingElement->name() << "while the mapping was disconnected";

                                    // Link the agent (in the global mapping) on its input from the mapping element (add a missing link TO the agent)
                                    _linkAgentOnInputFromMappingElement(agentInMapping, mappingElement);
                                }
                            }

                            // Cancel all removed links while the global mapping was UN-activated
                            agentInMapping->cancelAllRemovedLinks_WhileMappingWasUNactivated();
                        }*/
                    }
                    // CONTROL
                    /*else
                    {
                    }*/
                }
            }
        }
    }
}


/**
 * @brief Slot called when a model of agent "ON" has been added to an agent(s grouped by name)
 * @param model
 */
void AgentsMappingController::_onAgentModelONhasBeenAdded(AgentM* model)
{
    // Model of Agent ON
    if ((model != nullptr) && model->isON() && !model->name().isEmpty() && !model->peerId().isEmpty()
            // The global mapping is activated
            && (_modelManager != nullptr) && _modelManager->isMappingConnected())
    {
        QString agentName = model->name();

        // Get the (view model of) agent in the global mapping from the agent name
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);

        QStringList peerIdsList = QStringList(model->peerId());

        // CONTROL
        if (_modelManager->isMappingControlled())
        {
            // The agent is already in the global mapping
            if (agentInMapping != nullptr)
            {
                qDebug() << "CONTROL:" << agentName << "is ON and in the global mapping --> LOAD the MAPPING !";

                // Get the JSON of the mapping of the agent as displayed in the global mapping
                QString jsonOfMapping = _getJSONofMappingOfAgentInGlobalMapping(agentInMapping);

                QString command = QString("%1%2").arg(command_LoadMapping, jsonOfMapping);

                // Emit signal "Command asked to agent"
                Q_EMIT commandAskedToAgent(peerIdsList, command);
            }
            // The agent is NOT in the global mapping
            else
            {
                qDebug() << "CONTROL:" << agentName << "is ON but NOT in the global mapping --> CLEAR its MAPPING !";

                // Send the command "Clear Mapping" on the network to this agent(s)
                Q_EMIT commandAskedToAgent(peerIdsList, command_ClearMapping);
            }
        }
        // OBSERVE
        else
        {
            // The agent is already in the global mapping
            if (agentInMapping != nullptr)
            {
                qDebug() << "OBSERVE:" << agentName << "is ON and in the global mapping --> Nothing to do";
            }
            // The agent is NOT in the global mapping
            else
            {
                qDebug() << "OBSERVE:" << agentName << "is ON but NOT in the global mapping --> ADD to the global mapping !";

                double randomMax = static_cast<double>(RAND_MAX);

                // Get a random position in the current window
                QPointF position = _getRandomPosition(randomMax);
                // Default with of the new agent
                qreal width = AgentInMappingVM::DEFAULT_WIDTH;

                // Get the (view model of) agents grouped for the name
                AgentsGroupedByNameVM* agentsGroupedByName = _modelManager->getAgentsGroupedForName(agentName);
                if (agentsGroupedByName != nullptr)
                {
                    // Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position
                    agentInMapping = _createAgentInMappingAtPosition(agentsGroupedByName, position, width);
                    if (agentInMapping != nullptr)
                    {
                        // No need to add links on inputs now, because the mapping will be received after the creation of this agent(s grouped by name)
                        // Link the agent in the global mapping on its inputs (add all missing links TO the agent)
                        _linkAgentOnInputs(agentInMapping);

                        // Link the agent in the global mapping on its outputs (add all missing links FROM the agent)
                        _linkAgentOnOutputs(agentInMapping);
                    }
                }
            }
        }
    }
}


/**
 * @brief Slot called when some view models of mapping elements have been added to an agent(s grouped by name)
 * @param newMappingElements
 */
void AgentsMappingController::_onMappingElementsHaveBeenAdded(QList<MappingElementVM*> newMappingElements)
{
    AgentsGroupedByNameVM* agentsGroupedByName = qobject_cast<AgentsGroupedByNameVM*>(sender());
    if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty() && !newMappingElements.isEmpty() && (_modelManager != nullptr))
    {
        // Get the input agent in the global mapping from the agent name (outside loop "for")
        AgentInMappingVM* inputAgent = getAgentInMappingFromName(agentsGroupedByName->name());
        if (inputAgent != nullptr)
        {
            for (MappingElementVM* mappingElement : newMappingElements)
            {
                if ((mappingElement != nullptr) && (mappingElement->firstModel() != nullptr))
                {
                    qInfo() << mappingElement->name() << "MAPPED";

                    // The global mapping is activated
                    if (_modelManager->isMappingConnected())
                    {
                        // Link the agent on its input from the mapping element (add a missing link TO the agent)
                        _linkAgentOnInputFromMappingElement(inputAgent, mappingElement);
                    }
                    // The global mapping is NOT activated
                    else
                    {
                        // Get the (view model of) agent in the global mapping from the output agent name
                        AgentInMappingVM* outputAgent = getAgentInMappingFromName(mappingElement->firstModel()->outputAgent());
                        if (outputAgent != nullptr)
                        {
                            // Get the link input and the link output
                            LinkInputVM* linkInput = _getAloneLinkInputFromName(inputAgent, mappingElement->firstModel()->input(), mappingElement->name());
                            LinkOutputVM* linkOutput = _getAloneLinkOutputFromName(outputAgent, mappingElement->firstModel()->output(), mappingElement->name());

                            if ((linkInput != nullptr) && (linkOutput != nullptr))
                            {
                                // Get the link id (with format "outputAgent##output::outputType-->inputAgent##input::inputType") from agent names and Input/Output ids
                                QString linkId = LinkVM::getLinkIdFromObjectIdsAndIOids(outputAgent->uid(), linkOutput->uid(), inputAgent->uid(), linkInput->uid());

                                // Simulate that the user remove this link while the global mapping is UN-activated
                                inputAgent->removeLink_WhileMappingWasUNactivated(linkId, mappingElement);
                            }
                        }
                    }
                }
            }
        }
    }
}


/**
 * @brief Slot called when some view models of mapping elements will be removed from an agent(s grouped by name)
 * @param oldMappingElements
 */
void AgentsMappingController::_onMappingElementsWillBeRemoved(QList<MappingElementVM*> oldMappingElements)
{
    //AgentsGroupedByNameVM* agentsGroupedByName = qobject_cast<AgentsGroupedByNameVM*>(sender());
    //if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty() && !oldMappingElements.isEmpty())
    if (!oldMappingElements.isEmpty() && (_modelManager != nullptr))
    {
        for (MappingElementVM* mappingElement : oldMappingElements)
        {
            // firstModel is NULL, we cannot use it
            //if ((mappingElement != nullptr) && !mappingElement->name().isEmpty() && (mappingElement->firstModel() != nullptr))
            if ((mappingElement != nullptr) && !mappingElement->name().isEmpty())
            {
                QString linkName = mappingElement->name();

                //qDebug() << "on Mapping Elements will be Removed" << linkName;
                qInfo() << linkName << "UN-mapped";

                // Get the list of links between agents in the global mapping from a link name
                QList<LinkVM*> linksWithSameName = getLinksInMappingFromName(linkName);

                // The link is NOT in the global mapping
                if (linksWithSameName.isEmpty())
                {
                    // Get the list of names (each parts of a mapping element) from the link name (with format "outputAgent##output-->inputAgent##input")
                    QStringList namesList = MappingElementM::getNamesListFromLinkName(linkName);
                    if (namesList.count() == 4)
                    {
                        QString outputAgentName = namesList.at(0);
                        //QString outputName = namesList.at(1);
                        QString inputAgentName = namesList.at(2);
                        //QString inputName = namesList.at(3);

                        AgentInMappingVM* inputAgent = getAgentInMappingFromName(inputAgentName);
                        if (inputAgent != nullptr)
                        {
                            // Get the list of all names of removed mapping elements while the global mapping was UN-activated
                            QStringList namesOfRemovedMappingElements = inputAgent->getNamesOfRemovedMappingElements_WhileMappingWasUNactivated();
                            if (namesOfRemovedMappingElements.contains(linkName))
                            {
                                // The link has been removed when the global mapping was UN-activated
                                // Or when our input agent was OFF
                                qDebug() << "There is still the corresponding removed Mapping Element" << linkName << "while the Mapping was UN-activated";
                                inputAgent->mappingElementRemoved_CorrespondingLinkRemovedWhileMappingWasUNactivated(mappingElement);
                            }
                        }

                        // Remove the "Waiting Mapping Element" on the output agent name
                        _removeWaitingMappingElementOnOutputAgent(outputAgentName, mappingElement);
                    }

                }
                // The link is in the global mapping
                else if (linksWithSameName.count() == 1)
                {
                    LinkVM* link = linksWithSameName.at(0);
                    if (link != nullptr)
                    {
                        // The global mapping is activated
                        if (_modelManager->isMappingConnected())
                        {
                            // The link was Temporary
                            if (link->isTemporary())
                            {
                                qDebug() << "The link" << link->uid() << "from TEMPORARY to DELETED";

                                // Reset the flag
                                //link->setisTemporary(false);

                                if (_hashFromLinkIdToRemovedLink_WaitingReply.contains(link->uid()))
                                {
                                    // Remove from the hash table the "(unique) link id" and the link for which we are waiting a reply to the request "remove"
                                    _hashFromLinkIdToRemovedLink_WaitingReply.remove(link->uid());
                                }
                                else {
                                    qCritical() << "The 'Temporary' link" << link->uid() << "was not in the hash table 'Removed Link Waiting Reply'";
                                }
                            }

                            // Delete the link between two agents in the mapping
                            _deleteLinkBetweenTwoObjectsInMapping(link);
                        }
                        // The global mapping is NOT activated
                        else
                        {
                            if ((link->inputObject() != nullptr) && (link->linkInput() != nullptr) && (link->outputObject() != nullptr) && (link->linkOutput() != nullptr))
                            {
                                AgentInMappingVM* inputAgent = qobject_cast<AgentInMappingVM*>(link->inputObject());
                                if (inputAgent != nullptr)
                                {
                                    // Simulate that the user add this link while the global mapping is UN-activated
                                    inputAgent->addLink_WhileMappingWasUNactivated(link->uid(), link->linkInput()->name(), link->outputObject()->name(), link->linkOutput()->name());
                                }
                            }
                        }
                    }
                }
                // There are several links with this name in the global mapping
                else {
                    qWarning() << "There are" << linksWithSameName.count() << "links with the same name" << linkName << ". We cannot choose and remove the link";
                }
            }
        }
    }
}


/**
 * @brief Slot called when some view models of link inputs have been added to an agent in mapping
 * @param addedlinkInputs
 */
void AgentsMappingController::_onLinkInputsListHaveBeenAdded(const QList<LinkInputVM*>& addedlinkInputs)
{
    AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(sender());
    if ((agentInMapping != nullptr) && !addedlinkInputs.isEmpty())
    {
        // Link the agent in the global mapping on its inputs (add all missing links TO the agent)
        _linkAgentOnInputs(agentInMapping);
    }
}


/**
 * @brief Slot called when some view models of link outputs have been added to an agent in mapping
 * @param addedlinkOutputs
 */
void AgentsMappingController::_onLinkOutputsListHaveBeenAdded(const QList<LinkOutputVM*>& addedlinkOutputs)
{
    AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(sender());
    if ((agentInMapping != nullptr) && !addedlinkOutputs.isEmpty())
    {
        // Link the agent in the global mapping on its outputs (add all missing links FROM the agent)
        _linkAgentOnOutputs(agentInMapping);
    }
}


/**
 * @brief Slot called when some view models of link inputs will be removed from an agent in mapping
 * @param removedLinkInputs
 */
void AgentsMappingController::_onLinkInputsListWillBeRemoved(const QList<LinkInputVM*>& removedLinkInputs)
{
    AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(sender());
    if ((agentInMapping != nullptr) && !removedLinkInputs.isEmpty())
    {
        // Traverse the list of all links
        for (LinkVM* link : _allLinksInMapping.toList())
        {
            if ((link != nullptr) && (link->inputObject() != nullptr) && (link->linkInput() != nullptr))
            {
                AgentInMappingVM* inputAgent = qobject_cast<AgentInMappingVM*>(link->inputObject());
                if ((inputAgent != nullptr) && (inputAgent == agentInMapping) && removedLinkInputs.contains(link->linkInput()))
                {
                    // Delete the link between two agents in the mapping
                    _deleteLinkBetweenTwoObjectsInMapping(link);
                }
            }
        }
    }
}


/**
 * @brief Slot called when some view models of link outputs will be removed from an agent in mapping
 * @param removedLinkOutputs
 */
void AgentsMappingController::_onLinkOutputsListWillBeRemoved(const QList<LinkOutputVM*>& removedLinkOutputs)
{
    AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(sender());
    if ((agentInMapping != nullptr) && !removedLinkOutputs.isEmpty())
    {
        qDebug() << "_on Link Outputs List will be Removed from agent" << agentInMapping->name() << removedLinkOutputs.count();

        // Traverse the list of all links
        for (LinkVM* link : _allLinksInMapping.toList())
        {
            if ((link != nullptr) && (link->outputObject() != nullptr) && (link->linkOutput() != nullptr))
            {
                AgentInMappingVM* outputAgent = qobject_cast<AgentInMappingVM*>(link->outputObject());
                if ((outputAgent != nullptr) && (outputAgent == agentInMapping) && removedLinkOutputs.contains(link->linkOutput()))
                {
                    if (link->mappingElement() != nullptr)
                    {
                        // Add a "Waiting Mapping Element" on the output agent (name)
                        _addWaitingMappingElementOnOutputAgent(link->outputObject()->name(), link->mappingElement());
                    }

                    // Delete the link between two agents in the mapping
                    _deleteLinkBetweenTwoObjectsInMapping(link);
                }
            }
        }
    }
}


/**
 * @brief Slot called when an output has been activated, so we have to activate an input (of an action in the global mapping)
 * @param inputObject
 * @param linkInput
 */
void AgentsMappingController::_onActivateInputOfActionInMapping(ObjectInMappingVM* inputObject, LinkInputVM* linkInput)
{
    Q_UNUSED(linkInput)

    if (inputObject != nullptr)
    {
        ActionInMappingVM* actionInMapping = qobject_cast<ActionInMappingVM*>(inputObject);
        if ((actionInMapping != nullptr) && (actionInMapping->action() != nullptr))
        {
            qDebug() << "Execute (effects of) Action in Mapping" << actionInMapping->action()->name();

            // Emit the signal "Execute Action"
            Q_EMIT executeAction(actionInMapping->action());
        }
    }
}


/**
 * @brief Slot called when an output has been activated, so we have to write on an input (of an input agent in the global mapping)
 * @param inputObject
 * @param linkInput
 */
void AgentsMappingController::_onWriteOnInputOfAgentInMapping(ObjectInMappingVM* inputObject, LinkInputVM* linkInput)
{
    if ((inputObject != nullptr) && (linkInput != nullptr) && (linkInput->input() != nullptr))
    {
        AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(inputObject);
        if ((agentInMapping != nullptr) && (agentInMapping->agentsGroupedByName() != nullptr))
        {
            qDebug() << "Write on Input" << linkInput->input()->name() << "of Agent in Mapping" << agentInMapping->agentsGroupedByName()->name();

            // Emit the signal "Command asked to agent about Setting Value"
            Q_EMIT commandAskedToAgentAboutSettingValue(agentInMapping->agentsGroupedByName()->peerIdsList(),
                                                        "SET_INPUT",
                                                        linkInput->input()->name(),
                                                        "0");
        }
    }
}


/**
 * @brief Create a new agent in the global mapping with an "Agents Grouped by Name" and at a specific position
 * @param agentsGroupedByName
 * @param position
 * @return
 */
AgentInMappingVM* AgentsMappingController::_createAgentInMappingAtPosition(AgentsGroupedByNameVM* agentsGroupedByName, QPointF position, qreal width)
{
    AgentInMappingVM* agentInMapping = nullptr;

    if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty())
    {
        agentInMapping = getAgentInMappingFromName(agentsGroupedByName->name());

        // Check that there is NOT yet an agent in the global mapping for this name
        if (agentInMapping == nullptr)
        {
            // Create a new view model of agent in the global Mapping
            agentInMapping = new AgentInMappingVM(agentsGroupedByName, position, width, this);

            // Connect to signals from this new agent in mapping
            connect(agentInMapping, &AgentInMappingVM::linkInputsListHaveBeenAdded, this, &AgentsMappingController::_onLinkInputsListHaveBeenAdded);
            connect(agentInMapping, &AgentInMappingVM::linkOutputsListHaveBeenAdded, this, &AgentsMappingController::_onLinkOutputsListHaveBeenAdded);
            connect(agentInMapping, &AgentInMappingVM::linkInputsListWillBeRemoved, this, &AgentsMappingController::_onLinkInputsListWillBeRemoved);
            connect(agentInMapping, &AgentInMappingVM::linkOutputsListWillBeRemoved, this, &AgentsMappingController::_onLinkOutputsListWillBeRemoved);

            // Add in the hash table
            _hashFromNameToAgentInMapping.insert(agentsGroupedByName->name(), agentInMapping);

            // Add in the list with all agents (for QML)
            _allAgentsInMapping.append(agentInMapping);
        }
        else {
            qCritical() << "The agent" << agentsGroupedByName->name() << "is already in the global mapping";
        }
    }
    return agentInMapping;
}


/**
 * @brief Create a new action in the global mapping with a unique id, with a model of action and at a specific position
 * @param uid
 * @param action
 * @param position
 * @return
 */
ActionInMappingVM* AgentsMappingController::_createActionInMappingAtPosition(QString uid, ActionM* action, QPointF position, qreal width)
{
    ActionInMappingVM* actionInMapping = nullptr;

    if (action != nullptr)
    {
        actionInMapping = getActionInMappingFromUid(uid);

        // Check that there is NOT yet an action in the global mapping for this uid
        if (actionInMapping == nullptr)
        {
            // Create a new view model of action in mapping
            actionInMapping = new ActionInMappingVM(uid, action, position, width);

            // Add in the hash table
            _hashFromUidToActionInMapping.insert(actionInMapping->uid(), actionInMapping);

            // Add in the list with all actions (for QML)
            _allActionsInMapping.append(actionInMapping);
        }
        else {
            qCritical() << "The action" << action->name() << "with uid" << uid << "is already in the global mapping";
        }
    }
    return actionInMapping;
}


/**
 * @brief Create a link between two objects in the mapping
 * @param linkName
 * @param outputObject
 * @param linkOutput
 * @param inputObject
 * @param linkInput
 * @param mappingElement
 * @param isTemporary
 * @return
 */
LinkVM* AgentsMappingController::_createLinkBetweenTwoObjectsInMapping(const QString& linkName,
                                                                       ObjectInMappingVM* outputObject,
                                                                       LinkOutputVM* linkOutput,
                                                                       ObjectInMappingVM* inputObject,
                                                                       LinkInputVM* linkInput,
                                                                       MappingElementVM* mappingElement,
                                                                       bool isTemporary)
{
    LinkVM* link = nullptr;

    if ((outputObject != nullptr) && (linkOutput != nullptr)
            && (inputObject != nullptr) && (linkInput != nullptr))
    {
        // Get the link id (with format "outputObject##output::outputType-->inputObject##input::inputType") from object names and Input/Output ids
        QString linkId = LinkVM::getLinkIdFromObjectIdsAndIOids(outputObject->uid(), linkOutput->uid(), inputObject->uid(), linkInput->uid());

        link = getLinkInMappingFromId(linkId);

        // Check that there is NOT yet a link in the global mapping for this id
        if (link == nullptr)
        {
            // Create a new link between two agents
            link = new LinkVM(linkName,
                              mappingElement,
                              outputObject,
                              linkOutput,
                              inputObject,
                              linkInput,
                              isTemporary,
                              this);

            // Add to the list to update the view (QML)
            _allLinksInMapping.append(link);

            // Add to the hash table
            _hashFromIdToLinkInMapping.insert(link->uid(), link);

            // Update the list of links between agents in the global mapping for this link name
            QList<LinkVM*> linksWithSameName = getLinksInMappingFromName(linkName);
            linksWithSameName.append(link);
            _hashFromNameToListOfLinksInMapping.insert(linkName, linksWithSameName);
        }
        else {
            qWarning() << "The link" << linkId << "is already in the global mapping !";
        }
    }

    return link;
}


/**
 * @brief Remove a link between two agents from the mapping
 * @param link
 */
void AgentsMappingController::_removeLinkBetweenTwoAgents(LinkVM* link)
{
    if ((link != nullptr)
            && (link->inputObject() != nullptr) && (link->linkInput() != nullptr)
            && (link->outputObject() != nullptr) && (link->linkOutput() != nullptr))
    {
        AgentInMappingVM* inputAgent = qobject_cast<AgentInMappingVM*>(link->inputObject());

        if ((inputAgent != nullptr) && (inputAgent->agentsGroupedByName() != nullptr))
        {
            qInfo() << "Remove the link between agents" << link->outputObject()->name() << "and" << link->inputObject()->name();

            // The global mapping is activated AND the input agent is ON
            if ((_modelManager != nullptr) && _modelManager->isMappingConnected() && inputAgent->agentsGroupedByName()->isON())
            {
                if (!_hashFromLinkIdToRemovedLink_WaitingReply.contains(link->uid()))
                {
                    // Insert in the hash table the "(unique) link id" and the link for which we are waiting a reply to the request "remove"
                    _hashFromLinkIdToRemovedLink_WaitingReply.insert(link->uid(), link);

                    // Update the flag to give a feedback to the user
                    link->setisTemporary(true);
                }
                else {
                    qWarning() << "The request" << command_UnmapAgents << "has already been sent to remove the link" << link->uid();
                }

                // Emit signal "Command asked to agent about Mapping Input"
                Q_EMIT commandAskedToAgentAboutMappingInput(inputAgent->agentsGroupedByName()->peerIdsList(),
                                                            command_UnmapAgents,
                                                            link->linkInput()->name(),
                                                            link->outputObject()->name(),
                                                            link->linkOutput()->name());
            }
            // The global mapping is NOT activated OR the input agent is OFF
            else
            {
                MappingElementM* mappingElement = inputAgent->getAddedMappingElementFromLinkId_WhileMappingWasUNactivated(link->uid());

                // This link has been added while the mapping was UN-activated...
                if (mappingElement != nullptr)
                {
                    // ...just cancel the add of the link while the global mapping is UN-activated
                    inputAgent->cancelAddLink_WhileMappingWasUNactivated(link->uid());
                }
                // Remove the link while the global mapping is UN-activated
                else
                {
                    inputAgent->removeLink_WhileMappingWasUNactivated(link->uid(), link->mappingElement());
                }

                // Delete the link between two agents in the mapping
                _deleteLinkBetweenTwoObjectsInMapping(link);
            }
        }
    }
}


/**
 * @brief Delete a link between two objects in the mapping
 * @param link
 */
void AgentsMappingController::_deleteLinkBetweenTwoObjectsInMapping(LinkVM* link)
{
    if (link != nullptr)
    {
        qInfo() << "Delete the link" << link->name() << "in the Mapping";

        // Unselect our link if needed
        if (_selectedLink == link) {
            setselectedLink(nullptr);
        }

        // DIS-connect to signals from the link
        disconnect(link, nullptr, this, nullptr);

        // Remove from the hash table
        _hashFromIdToLinkInMapping.remove(link->uid());

        // Update the list of links between agents in the global mapping for this link name
        QList<LinkVM*> linksWithSameName = getLinksInMappingFromName(link->name());
        linksWithSameName.removeOne(link);
        _hashFromNameToListOfLinksInMapping.insert(link->name(), linksWithSameName);

        // Remove from the list to update the view (QML)
        _allLinksInMapping.remove(link);

        // Free memory
        delete link;
    }
}


/**
 * @brief Remove all the links with an agent
 * @param agent
 */
void AgentsMappingController::_removeAllLinksWithAgent(AgentInMappingVM* agent)
{
    if ((agent != nullptr) && (agent->agentsGroupedByName() != nullptr) && (_modelManager != nullptr))
    {
        qDebug() << "Remove all Links with agent" << agent->name();

        for (LinkVM* link : _allLinksInMapping.toList())
        {
            if ( (link != nullptr) && ((link->outputObject() == agent) || (link->inputObject() == agent)) )
            {
                if (link->outputObject() == agent)
                {
                    // Add a "Waiting Mapping Element" on the output agent (name)
                   _addWaitingMappingElementOnOutputAgent(link->outputObject()->name(), link->mappingElement());
                }

                // Delete the link between two objects in the mapping
                _deleteLinkBetweenTwoObjectsInMapping(link);
            }
        }

        // The global mapping is activated AND the agent is ON
        if (_modelManager->isMappingConnected() && agent->agentsGroupedByName()->isON())
        {
            // Send the command "Clear Mapping" on the network to this agent(s)
            Q_EMIT commandAskedToAgent(agent->agentsGroupedByName()->peerIdsList(), command_ClearMapping);
        }
        // The global mapping is NOT activated OR the agent is OFF
        else
        {
            // Useless because the agent in mapping will be deleted (and data about "_WhileMappingWasUNactivated" too)
            /*for (LinkVM* link : _allLinksInMapping.toList())
            {
                if ((link != nullptr) && (link->inputAgent() == agent))
                {
                    MappingElementM* mappingElement = link->inputAgent()->getAddedMappingElementFromLinkId_WhileMappingWasUNactivated(link->uid());

                    // This link has been added while the mapping was UN-activated...
                    if (mappingElement != nullptr)
                    {
                        // ...just cancel the add of the link while the global mapping is UN-activated
                        link->inputAgent()->cancelAddLink_WhileMappingWasUNactivated(link->uid());
                    }
                    // Remove the link while the global mapping is UN-activated
                    else
                    {
                        link->inputAgent()->removeLink_WhileMappingWasUNactivated(link->uid(), link->mappingElement());
                    }
                }
            }*/
        }
    }
}


/**
 * @brief Update the global mapping with agents ON and their links
 */
void AgentsMappingController::_updateMappingWithAgentsONandLinks()
{
    if (_modelManager != nullptr)
    {
        double randomMax = static_cast<double>(RAND_MAX);

        // Traverse the list of all "agents grouped by name"
        for (AgentsGroupedByNameVM* agentsGroupedByName : _modelManager->allAgentsGroupsByName()->toList())
        {
            // Only when the agent(s) is ON
            if ((agentsGroupedByName != nullptr) && agentsGroupedByName->isON() && !agentsGroupedByName->name().isEmpty())
            {
                AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentsGroupedByName->name());

                // This agent(s) is NOT yet in the global mapping
                if (agentInMapping == nullptr)
                {
                    // Get a random position in the current window
                    QPointF position = _getRandomPosition(randomMax);
                    // Default with of the new agent
                    qreal width = AgentInMappingVM::DEFAULT_WIDTH;

                    // Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position
                    _createAgentInMappingAtPosition(agentsGroupedByName, position, width);
                }
            }
        }

        // Link all agents in the global mapping
        for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
        {
            if (agentInMapping != nullptr)
            {
                // Link the agent in the global mapping on its inputs (add all missing links TO the agent)
                _linkAgentOnInputs(agentInMapping);

                // Link the agent in the global mapping on its outputs (add all missing links FROM the agent)
                _linkAgentOnOutputs(agentInMapping);
            }
        }

        // Notify the QML to fit the view
        Q_EMIT fitToView();
    }
}


/**
 * @brief Link an agent (in the global mapping) on its inputs (add all missing links TO an agent)
 * @param agentInMapping
 */
void AgentsMappingController::_linkAgentOnInputs(AgentInMappingVM* agentInMapping)
{
    if ((agentInMapping != nullptr) && !agentInMapping->name().isEmpty()
            && (agentInMapping->agentsGroupedByName() != nullptr))
    {
        // Traverse the list of all mapping mapping elements which this agent is the input agent
        for (MappingElementVM* mappingElement : agentInMapping->agentsGroupedByName()->allMappingElements()->toList())
        {
            if (mappingElement != nullptr)
            {
                // Link the agent on its input from the mapping element (add a missing link TO the agent)
                _linkAgentOnInputFromMappingElement(agentInMapping, mappingElement);
            }
        }
    }
}


/**
 * @brief Link an agent (in the global mapping) on its input from a mapping element (add a missing link TO an agent)
 * @param inputAgent
 * @param mappingElement
 */
void AgentsMappingController::_linkAgentOnInputFromMappingElement(AgentInMappingVM* inputAgent, MappingElementVM* mappingElement)
{
    if ((inputAgent != nullptr) && !inputAgent->name().isEmpty()
            && (mappingElement != nullptr) && (mappingElement->firstModel() != nullptr) && !mappingElement->name().isEmpty()
            && (inputAgent->name() == mappingElement->firstModel()->inputAgent()))
    {
        QString linkName = mappingElement->name();

        // Get the list of links between agents in the global mapping from a link name
        QList<LinkVM*> linksWithSameName = getLinksInMappingFromName(linkName);

        // The link is NOT in the global mapping
        if (linksWithSameName.isEmpty())
        {
            //qDebug() << "Try to create link" << linkName << "TO agent" << inputAgent->name() << "involved as 'INPUT' agent";

            // Get the (view model of) agent in the global mapping from the output agent name
            AgentInMappingVM* outputAgent = getAgentInMappingFromName(mappingElement->firstModel()->outputAgent());

            // The Output agent is NOT in the mapping
            if (outputAgent == nullptr)
            {
                // Add the "Waiting Mapping Element" on the output agent (name)
                _addWaitingMappingElementOnOutputAgent(mappingElement->firstModel()->outputAgent(), mappingElement);
            }
            // The Output agent is in the mapping
            else
            {
                LinkOutputVM* linkOutput = nullptr;

                // Get the list of view models of link output from the output name
                QList<LinkOutputVM*> linkOutputsWithSameName = outputAgent->getLinkOutputsListFromName(mappingElement->firstModel()->output());

                // The output does NOT exist
                if (linkOutputsWithSameName.isEmpty())
                {
                    // Add the "Waiting Mapping Element" on the output agent (name)
                    _addWaitingMappingElementOnOutputAgent(mappingElement->firstModel()->outputAgent(), mappingElement);
                }
                // The output exist
                else if (linkOutputsWithSameName.count() == 1)
                {
                    linkOutput = linkOutputsWithSameName.at(0);
                    if (linkOutput != nullptr)
                    {
                        // Get the link input from a name if there is only one input for this name
                        LinkInputVM* linkInput = _getAloneLinkInputFromName(inputAgent, mappingElement->firstModel()->input(), linkName);
                        if (linkInput != nullptr)
                        {
                            // Create a new REAL link between the two agents
                            _createLinkBetweenTwoObjectsInMapping(linkName,
                                                                  outputAgent,
                                                                  linkOutput,
                                                                  inputAgent,
                                                                  linkInput,
                                                                  mappingElement);

                            // Remove eventually the corresponding "Waiting Mapping Element" on the output agent name
                            _removeWaitingMappingElementOnOutputAgent(outputAgent->name(), mappingElement);
                        }
                    }
                }
                // There are several outputs with the same name
                else {
                    qWarning() << "There are" << linkOutputsWithSameName.count() << "link outputs with the same name" << mappingElement->firstModel()->output() << "."
                               << "We cannot choose and create the link" << linkName;
                }
            }
        }
        // The link is in the global mapping
        else if (linksWithSameName.count() == 1)
        {
            LinkVM* link = linksWithSameName.at(0);
            if (link != nullptr)
            {
                // The link was Virtual (agents were not linked on the network), it is a real one now !
                if (link->mappingElement() == nullptr)
                {
                    qDebug() << "The link" << link->uid() << "from VIRTUAL to REAL";

                    link->setmappingElement(mappingElement);

                    // The link was Temporary
                    if (link->isTemporary())
                    {
                        qDebug() << "The link" << link->uid() << "from TEMPORARY to REAL";

                        // Reset the flag
                        link->setisTemporary(false);

                        if (_hashFromLinkIdToAddedLink_WaitingReply.contains(link->uid()))
                        {
                            // Remove from the hash table the "(unique) link id" and the link for which we are waiting a reply to the request "add"
                            _hashFromLinkIdToAddedLink_WaitingReply.remove(link->uid());
                        }
                        else {
                            qCritical() << "The 'Temporary' link" << link->uid() << "was not in the hash table 'Added Link Waiting Reply'";
                        }
                    }
                    // The link has been added when the global mapping was UN-activated
                    // Or when our input agent was OFF
                    else
                    {
                        if (link->inputObject() != nullptr)
                        {
                            AgentInMappingVM* inputAgentFromLink = qobject_cast<AgentInMappingVM*>(link->inputObject());
                            if ((inputAgentFromLink != nullptr) && inputAgentFromLink->getAddedMappingElementFromLinkId_WhileMappingWasUNactivated(link->uid()))
                            {
                                qDebug() << "There is still the corresponding added Mapping Element" << link->uid() << "while the Mapping was UN-activated";
                                inputAgentFromLink->mappingElementAdded_CorrespondingLinkAddedWhileMappingWasUNactivated(link->uid());
                            }
                        }
                    }
                }
                else {
                    qWarning() << "The link" << link->uid() << "already exist";
                }
            }
        }
        // There are several links with this name in the global mapping
        else {
            qWarning() << "There are" << linksWithSameName.count() << "links with the same name" << linkName << ". We cannot choose the link";
        }
    }
}


/**
 * @brief Link an agent (in the global mapping) on its outputs (add all missing links FROM an agent)
 * @param agentInMapping
 */
void AgentsMappingController::_linkAgentOnOutputs(AgentInMappingVM* agentInMapping)
{
    if ((agentInMapping != nullptr) && !agentInMapping->name().isEmpty())
    {
        QString agentName = agentInMapping->name();

        // Get the list of "Waiting Mapping Elements" on this Output agent name (where the agent is involved as "Output Agent")
        QList<MappingElementVM*> listOfWaitingMappingElements = _getWaitingMappingElementsOnOutputAgent(agentName);

        if (!listOfWaitingMappingElements.isEmpty())
        {
            for (MappingElementVM* waitingMappingElement : listOfWaitingMappingElements)
            {
                if ((waitingMappingElement != nullptr) && !waitingMappingElement->name().isEmpty() && (waitingMappingElement->firstModel() != nullptr))
                {
                    QString linkName = waitingMappingElement->name();

                    qDebug() << "Try to create WAITING link" << linkName << "from agent" << agentName << "involved as 'OUTPUT' agent";

                    // Get the input agent in the global mapping from the input agent name
                    AgentInMappingVM* inputAgent = getAgentInMappingFromName(waitingMappingElement->firstModel()->inputAgent());

                    if (inputAgent != nullptr)
                    {
                        // Get the link input from a name if there is only one input for this name
                        LinkInputVM* linkInput = _getAloneLinkInputFromName(inputAgent, waitingMappingElement->firstModel()->input(), linkName);

                        // Get the link output from a name if there is only one output for this name
                        LinkOutputVM* linkOutput = _getAloneLinkOutputFromName(agentInMapping, waitingMappingElement->firstModel()->output(), linkName);

                        if ((linkOutput != nullptr) && (linkInput != nullptr))
                        {
                            // Create a new REAL link between the two agents
                            _createLinkBetweenTwoObjectsInMapping(linkName,
                                                                  agentInMapping,
                                                                  linkOutput,
                                                                  inputAgent,
                                                                  linkInput,
                                                                  waitingMappingElement);

                            // Remove the corresponding "Waiting Mapping Element" on the output agent name
                            _removeWaitingMappingElementOnOutputAgent(agentName, waitingMappingElement);
                        }
                    }
                }
            }
        }
    }
}


/**
 * @brief Get a random position in the current window
 * @param randomMax
 * @return
 */
QPointF AgentsMappingController::_getRandomPosition(double randomMax)
{
    double randomX = static_cast<double>(qrand()) / randomMax;
    double randomY = static_cast<double>(qrand()) / randomMax;

    // 5% + (random * 90% of the width) + the spawn zone's offset to follow the viewport
    double x = _xSpawnZoneOffset + (0.05 * _viewWidth) + (0.85 * _viewWidth * randomX);

    // 5% + (random * 90% of the height) + the spawn zone's offset to follow the viewport
    double y = _ySpawnZoneOffset + (0.05 * _viewHeight) + (0.85 * _viewHeight * randomY);

    return QPointF(x, y);
}


/**
 * @brief Get the list of "Waiting Mapping Elements" on an output agent (name)
 * @param outputAgentName
 * @return
 */
QList<MappingElementVM*> AgentsMappingController::_getWaitingMappingElementsOnOutputAgent(const QString& outputAgentName)
{
    return _hashFromOutputAgentNameToListOfWaitingMappingElements.value(outputAgentName, QList<MappingElementVM*>());
}


/**
 * @brief Add a "Waiting Mapping Element" on an output agent (name)
 * @param outputAgentName
 * @param waitingMappingElement
 */
void AgentsMappingController::_addWaitingMappingElementOnOutputAgent(const QString& outputAgentName, MappingElementVM* waitingMappingElement)
{
    if (!outputAgentName.isEmpty() && (waitingMappingElement != nullptr))
    {
        // Get the list of "Waiting Mapping Elements" on this output agent name (where the agent is involved as "Output Agent")
        QList<MappingElementVM*> listOfWaitingMappingElements = _getWaitingMappingElementsOnOutputAgent(outputAgentName);

        if (!listOfWaitingMappingElements.contains(waitingMappingElement))
        {
            // Add the "Waiting" mapping element to the list
            listOfWaitingMappingElements.append(waitingMappingElement);

            // Update the hash table
            _hashFromOutputAgentNameToListOfWaitingMappingElements.insert(outputAgentName, listOfWaitingMappingElements);

            qDebug() << waitingMappingElement->name() << "added to 'Waiting List' on" << outputAgentName;
        }
    }
}


/**
 * @brief Remove a "Waiting Mapping Element" on an output agent (name)
 * @param outputAgentName
 * @param waitingMappingElement
 */
void AgentsMappingController::_removeWaitingMappingElementOnOutputAgent(const QString& outputAgentName, MappingElementVM* waitingMappingElement)
{
    if (!outputAgentName.isEmpty() && (waitingMappingElement != nullptr))
    {
        // Get the list of "Waiting Mapping Elements" on this output agent name (where the agent is involved as "Output Agent")
        QList<MappingElementVM*> listOfWaitingMappingElements = _getWaitingMappingElementsOnOutputAgent(outputAgentName);

        if (listOfWaitingMappingElements.contains(waitingMappingElement))
        {
            // Remove the "Waiting" mapping element from the list
            listOfWaitingMappingElements.removeOne(waitingMappingElement);

            // Update the hash table
            _hashFromOutputAgentNameToListOfWaitingMappingElements.insert(outputAgentName, listOfWaitingMappingElements);

            qDebug() << waitingMappingElement->name() << "removed from 'Waiting List' on" << outputAgentName;
        }
    }
}


/**
 * @brief Helper to get the link input from a name if there is only one input for this name
 * If there are several inputs with the same name (but different value type), we return NULL
 * @param agent
 * @param inputName
 * @param linkName
 * @return
 */
LinkInputVM* AgentsMappingController::_getAloneLinkInputFromName(AgentInMappingVM* agent, const QString& inputName, const QString& linkName)
{
    LinkInputVM* linkInput = nullptr;

    if ((agent != nullptr) && !inputName.isEmpty())
    {
        // Get the list of view models of link input from the input name
        QList<LinkInputVM*> linkInputsWithSameName = agent->getLinkInputsListFromName(inputName);

        if (linkInputsWithSameName.count() == 1) {
            linkInput = linkInputsWithSameName.at(0);
        }
        else if (linkInputsWithSameName.count() > 1) {
            qWarning() << "There are" << linkInputsWithSameName.count() << "link inputs with the same name" << inputName << "."
                       << "We cannot choose and create the link" << linkName;
        }
    }
    return linkInput;
}


/**
 * @brief Helper to get the link output from a name if there is only one output for this name
 * If there are several outputs with the same name (but different value type), we return NULL
 * @param agent
 * @param outputName
 * @param linkName
 * @return
 */
LinkOutputVM* AgentsMappingController::_getAloneLinkOutputFromName(AgentInMappingVM* agent, const QString& outputName, const QString& linkName)
{
    LinkOutputVM* linkOutput = nullptr;

    if ((agent != nullptr) && !outputName.isEmpty())
    {
        // Get the list of view models of link output from the output name
        QList<LinkOutputVM*> linkOutputsWithSameName = agent->getLinkOutputsListFromName(outputName);

        if (linkOutputsWithSameName.count() == 1) {
            linkOutput = linkOutputsWithSameName.at(0);
        }
        else if (linkOutputsWithSameName.count() > 1) {
            qWarning() << "There are" << linkOutputsWithSameName.count() << "link outputs with the same name" << outputName << "."
                       << "We cannot choose and create the link" << linkName;
        }
    }
    return linkOutput;
}


/**
 * @brief Get the JSON of the mapping of an agent as displayed in the global mapping
 * @param agentInMapping
 * @return
 */
QString AgentsMappingController::_getJSONofMappingOfAgentInGlobalMapping(AgentInMappingVM* agentInMapping)
{
    QString jsonOfMapping;

    if ((agentInMapping != nullptr) && (agentInMapping->agentsGroupedByName() != nullptr) && (agentInMapping->agentsGroupedByName()->currentMapping() != nullptr)
            && (_jsonHelper != nullptr))
    {
        if (agentInMapping->hadLinksAdded_WhileMappingWasUNactivated() || agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated())
        {
            // Get the list of all added mapping elements while the global mapping was UN-activated
            QList<MappingElementM*> addedMappingElements = agentInMapping->getAddedMappingElements_WhileMappingWasUNactivated();

            // Get the list of all names of removed mapping elements while the global mapping was UN-activated
            QStringList namesOfRemovedMappingElements = agentInMapping->getNamesOfRemovedMappingElements_WhileMappingWasUNactivated();

            // Get the JSON of the current mapping of the agent with changes applied while the mapping was UN-activated
            jsonOfMapping = _jsonHelper->getJsonOfAgentMappingWithChanges(agentInMapping->agentsGroupedByName()->currentMapping(),
                                                                          addedMappingElements,
                                                                          namesOfRemovedMappingElements,
                                                                          QJsonDocument::Compact);
        }
        else
        {
            // Get the JSON of the current mapping of the agent
            jsonOfMapping = _jsonHelper->getJsonOfAgentMapping(agentInMapping->agentsGroupedByName()->currentMapping(),
                                                               QJsonDocument::Compact);
        }
    }
    return jsonOfMapping;
}

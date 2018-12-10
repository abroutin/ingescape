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


/**
 * @brief Constructor
 * @param modelManager
 * @param jsonHelper
 * @param directoryPath
 * @param parent
 */
AgentsMappingController::AgentsMappingController(IngeScapeModelManager* modelManager,
                                                 JsonHelper* jsonHelper,
                                                 QObject *parent) : QObject(parent),
      _viewWidth(1920 - 320), // Full HD - Width of left panel
      _viewHeight(1080 - 100), // Full HD - Height of top & bottom bars of OS
      _isEmptyMapping(true),
      _selectedAgent(nullptr),
      _selectedLink(nullptr),
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
    setselectedLink(nullptr);

    // DIS-connect from signal "Count Changed" from the list of agents in mapping
    disconnect(&_allAgentsInMapping, 0, this, 0);

    // Clear the current mapping
    clearMapping();

    // Delete all links
    _hashFromIdToLinkInMapping.clear();
    _hashFromNameToListOfLinksInMapping.clear();
    _hashFromOutputAgentNameToListOfWaitingMappingElements.clear();
    _allLinksInMapping.deleteAllItems();

    // Delete all agents in mapping
    _hashFromNameToAgentInMapping.clear();
    _allAgentsInMapping.deleteAllItems();

    // Reset pointers
    _modelManager = nullptr;
    _jsonHelper = nullptr;
}


/**
 * @brief Clear the current mapping
 */
void AgentsMappingController::clearMapping()
{
    qInfo() << "Clear the current mapping";

    // 1- First, DE-activate the mapping
    if (_modelManager != nullptr) {
        _modelManager->setisMappingActivated(false);
    }

    // 2- Delete all links
    for (LinkVM* link : _allLinksInMapping.toList()) {
        _deleteLinkBetweenTwoAgents(link);
    }

    // 3- Delete all agents in mapping
    for (AgentInMappingVM* agent : _allAgentsInMapping.toList()) {
        deleteAgentInMapping(agent);
    }

    qInfo() << "The Mapping is empty !";
}


/**
 * @brief Remove the agent from the mapping and delete it
 * @param agent
 */
void AgentsMappingController::deleteAgentInMapping(AgentInMappingVM* agent)
{
    if (agent != nullptr)
    {
        qInfo() << "Delete the agent" << agent->name() << "in the Mapping";

        // Unselect our agent if needed
        if (_selectedAgent == agent) {
            setselectedAgent(nullptr);
        }

        // DIS-connect to signals from this agent in mapping
        disconnect(agent, 0, this, 0);

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
 * @brief Remove a link between two agents from the mapping
 * @param link
 * @return true if the link has been deleted during the call of our method
 */
bool AgentsMappingController::removeLinkBetweenTwoAgents(LinkVM* link)
{
    bool linkHasBeenDeleted = false;

    if ((link != nullptr)
            && (link->inputAgent() != nullptr) && (link->inputAgent()->agentsGroupedByName() != nullptr) && (link->linkInput() != nullptr)
            && (link->outputAgent() != nullptr) && (link->outputAgent()->agentsGroupedByName() != nullptr) && (link->linkOutput() != nullptr))
    {
        qInfo() << "Remove the link between agents" << link->outputAgent()->name() << "and" << link->inputAgent()->name();

        // Mapping is activated
        // AND the input agent is ON
        if ((_modelManager != nullptr) && _modelManager->isMappingActivated() && link->inputAgent()->agentsGroupedByName()->isON())
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
            Q_EMIT commandAskedToAgentAboutMappingInput(link->inputAgent()->agentsGroupedByName()->peerIdsList(),
                                                        command_UnmapAgents,
                                                        link->linkInput()->name(),
                                                        link->outputAgent()->name(),
                                                        link->linkOutput()->name());
        }
        // Mapping is NOT activated
        // OR the input agent is OFF
        else
        {
            // FIXME REPAIR removeTemporaryLink
            // Remove temporary link (this temporary link will be removed when the user will activate the mapping)
            //link->inputAgent()->removeTemporaryLink(link->linkInput()->name(), link->outputAgent()->name(), link->linkOutput()->name());

            // This link has been added while the mapping was UN-activated, just cancel the add
            /*if (_idsOfAddedLinks_WhileMappingWasUNactivated.contains(link->uid())) {
                _idsOfAddedLinks_WhileMappingWasUNactivated.removeOne(link->uid());
            }
            // Add the corresponding mapping element to the hash
            else {
                _hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated.insert(link->uid(), link->mappingElement());
            }*/


            ElementMappingM* mappingElement = link->inputAgent()->getAddedMappingElementFromLinkId_WhileMappingWasUNactivated(link->uid());

            // This link has been added while the mapping was UN-activated, just cancel the add
            if (mappingElement != nullptr)
            {
                link->inputAgent()->cancelAddLink_WhileMappingWasUNactivated(link->uid());
            }
            else
            {
                link->inputAgent()->removeLink_WhileMappingWasUNactivated(link->uid(), link->mappingElement());
            }

            // Delete the link between two agents
            _deleteLinkBetweenTwoAgents(link);

            linkHasBeenDeleted = true;
        }
    }

    return linkHasBeenDeleted;
}


/**
 * @brief Called when an agent from the list is dropped on the current mapping at a position
 * @param agentName
 * @param position
 */
void AgentsMappingController::dropAgentNameToMappingAtPosition(QString agentName, QPointF position)
{
    // Check that there is NOT yet an agent in the current mapping for this name
    AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);

    if ((agentInMapping == nullptr) && (_modelManager != nullptr))
    {
        // Get the (view model of) agents grouped for this name
        AgentsGroupedByNameVM* agentsGroupedByName = _modelManager->getAgentsGroupedForName(agentName);
        if (agentsGroupedByName != nullptr)
        {
            // Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position
            AgentInMappingVM* agentInMapping = _createAgentInMappingAtPosition(agentsGroupedByName, position);

            if (agentInMapping != nullptr)
            {
                // FIXME Usefull or Useless ?
                /*// Get the mapping currently edited (temporary until the user activate the mapping)
                AgentMappingM* temporaryMapping = agentInMapping->temporaryMapping();
                if (temporaryMapping != nullptr)
                {
                    // Delete all "mapping elements" in this temporary mapping
                    temporaryMapping->mappingElements()->deleteAllItems();

                    // Mapping is already activated
                    if ((_modelManager != nullptr) && _modelManager->isMappingActivated())
                    {
                        for (AgentM* model : agentsGroupedByName->models()->toList())
                        {
                            if (model != nullptr)
                            {
                                // OverWrite the mapping of the model of agent (with the mapping currently edited in the agent in mapping)
                                _overWriteMappingOfAgentModel(model, temporaryMapping);
                            }
                        }
                    }
                }*/

                // The mapping is activated
                if (_modelManager->isMappingActivated())
                {
                    // CONTROL
                    if (_modelManager->isMappingControlled())
                    {
                        // FIXME: CONTROL TODO ?
                    }
                    // OBSERVE
                    else
                    {
                        // Link the agent in the global mapping on its inputs (add all missing links TO the agent)
                        _linkAgentOnInputs(agentInMapping);

                        // Link the agent in the global mapping on its outputs (add all missing links FROM the agent)
                        _linkAgentOnOutputs(agentInMapping);
                    }
                }

                // Selects this new agent
                setselectedAgent(agentInMapping);
            }
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
void AgentsMappingController::dropLinkBetweenAgents(AgentInMappingVM* outputAgent, LinkOutputVM* linkOutput, AgentInMappingVM* inputAgent, LinkInputVM* linkInput)
{
    if ((outputAgent != nullptr) && (outputAgent->agentsGroupedByName() != nullptr) && (linkOutput != nullptr) && (linkOutput->output() != nullptr)
            && (inputAgent != nullptr) && (inputAgent->agentsGroupedByName() != nullptr) && (linkInput != nullptr) && (linkInput->input() != nullptr))
    {
        // Check that the input can link to the output
        if (linkInput->canLinkWith(linkOutput))
        {
            // Get the link id (with format "outputAgent##output::outputType-->inputAgent##input::inputType") from agent names and Input/Output ids
            QString linkId = LinkVM::getLinkIdFromAgentNamesAndIOids(outputAgent->name(), linkOutput->uid(), inputAgent->name(), linkInput->uid());

            LinkVM* link = getLinkInMappingFromId(linkId);

            // Check that the link does NOT exist
            if (link == nullptr)
            {
                // Get the link name (with format "outputAgent##output-->inputAgent##input") from the list of names (each parts of a mapping element)
                QString linkName = ElementMappingM::getLinkNameFromNamesList(outputAgent->name(), linkOutput->name(), inputAgent->name(), linkInput->name());

                qInfo() << "QML asked to create the link" << linkId;

                // Mapping is activated
                // AND the input agent is ON
                if ((_modelManager != nullptr) && _modelManager->isMappingActivated() && inputAgent->agentsGroupedByName()->isON())
                {
                    // Create a new TEMPORARY link between the two agents
                    link = _createLinkBetweenTwoAgents(linkName,
                                                       outputAgent,
                                                       linkOutput,
                                                       inputAgent,
                                                       linkInput,
                                                       nullptr,
                                                       true);

                    // FIXME REPAIR addTemporaryMappingElement
                    // Add a temporary mapping element (will became a real link when the agent will send its mapping update)
                    //inputAgent->addTemporaryMappingElement(link->uid(), linkInput->uid(), outputAgent->name(), linkOutput->uid());

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
                // Mapping is NOT activated
                // OR the input agent is OFF
                else
                {
                    // Create a new (REAL) link between the two agents
                    link = _createLinkBetweenTwoAgents(linkName,
                                                       outputAgent,
                                                       linkOutput,
                                                       inputAgent,
                                                       linkInput,
                                                       nullptr,
                                                       false);

                    // FIXME REPAIR addTemporaryMappingElement
                    // Add a temporary mapping element (will became a real link when the user will activate the mapping)
                    //inputAgent->addTemporaryMappingElement(link->uid(), linkInput->uid(), outputAgent->name(), linkOutput->uid());

                    // This link has been removed while the mapping was UN-activated, just cancel the suppression
                    /*if (_hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated.contains(linkId))
                    {
                        MappingElementVM* mappingElement = _hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated.value(linkId);
                        link->setmappingElement(mappingElement);

                        _hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated.remove(linkId);
                    }
                    // Add the link id to the list
                    else {
                        _idsOfAddedLinks_WhileMappingWasUNactivated.append(linkId);
                    }*/


                    MappingElementVM* mappingElement = inputAgent->getRemovedMappingElementFromLinkId_WhileMappingWasUNactivated(linkId);

                    // This link has been removed while the mapping was UN-activated, just cancel the suppression
                    if (mappingElement != nullptr)
                    {
                        link->setmappingElement(mappingElement);

                        inputAgent->cancelRemoveLink_WhileMappingWasUNactivated(linkId);
                    }
                    else
                    {
                        inputAgent->addLink_WhileMappingWasUNactivated(link->uid(), linkInput->uid(), outputAgent->name(), linkOutput->uid());
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
 * @brief Get the (view model of) agent in the global mapping from an agent name
 * @param name
 * @return
 */
AgentInMappingVM* AgentsMappingController::getAgentInMappingFromName(QString name)
{
    if (_hashFromNameToAgentInMapping.contains(name)) {
        return _hashFromNameToAgentInMapping.value(name);
    }
    else {
        return nullptr;
    }
}


/**
 * @brief Get the list of (view model of) links between agents in the global mapping from a link name
 * @param linkName
 * @return
 */
QList<LinkVM*> AgentsMappingController::getLinksInMappingFromName(QString linkName)
{
    if (_hashFromNameToListOfLinksInMapping.contains(linkName)) {
        return _hashFromNameToListOfLinksInMapping.value(linkName);
    }
    else {
        return QList<LinkVM*>();
    }
}


/**
 * @brief Get the (view model of) link between agents in the global mapping from a link id
 * @param linkId
 * @return
 */
LinkVM* AgentsMappingController::getLinkInMappingFromId(QString linkId)
{
    if (_hashFromIdToLinkInMapping.contains(linkId)) {
        return _hashFromIdToLinkInMapping.value(linkId);
    }
    else {
        return nullptr;
    }
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
        for (AgentInMappingVM* agentInMapping : _allAgentsInMapping)
        {
            if ((agentInMapping != nullptr) && (agentInMapping->agentsGroupedByName() != nullptr) && (agentInMapping->agentsGroupedByName()->currentMapping() != nullptr))
            {
                QJsonObject jsonAgent;

                // Set the agent name
                jsonAgent.insert("agentName", agentInMapping->name());

                // Set the position
                QString position = QString("%1, %2").arg(QString::number(agentInMapping->position().x()), QString::number(agentInMapping->position().y()));
                jsonAgent.insert("position", position);

                // Set the mapping
                QJsonObject jsonMapping = QJsonObject();

                // The mapping is activated
                if (_modelManager->isMappingActivated())
                {
                    // Export the current mapping
                    jsonMapping = _jsonHelper->exportAgentMappingToJson(agentInMapping->agentsGroupedByName()->currentMapping());
                }
                // The mapping is NOT activated
                else
                {
                    // FIXME REPAIR temporaryMappingElements
                    // Export the current mapping plus the temporary list of mapping elements
                    //jsonMapping = _jsonHelper->exportAgentTemporaryMappingToJson(agentInMapping->agentsGroupedByName()->currentMapping(), agentInMapping->temporaryMappingElements()->toList());
                }
                jsonAgent.insert("mapping", jsonMapping);

                jsonArray.append(jsonAgent);
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
    if (_jsonHelper != nullptr)
    {
        //QList<ElementMappingM*> listOfMappingElements;

        for (QJsonValue jsonValue : jsonArrayOfAgentsInMapping)
        {
            if (jsonValue.isObject())
            {
                QJsonObject jsonAgent = jsonValue.toObject();

                // Get values for keys "agentName", "mapping" and "position"
                QJsonValue jsonName = jsonAgent.value("agentName");
                QJsonValue jsonMapping = jsonAgent.value("mapping");
                QJsonValue jsonPosition = jsonAgent.value("position");

                if (jsonName.isString() && jsonMapping.isObject() && jsonPosition.isString())
                {
                    QString agentName = jsonName.toString();

                    // Create the agent mapping from JSON
                    AgentMappingM* agentMapping = _jsonHelper->createModelOfAgentMappingFromJSON(agentName, jsonMapping.toObject());

                    // Position
                    QPointF position = QPointF();

                    QStringList positionStringList = jsonPosition.toString().split(", ");
                    if (positionStringList.count() == 2)
                    {
                        QString strX = positionStringList.at(0);
                        QString strY = positionStringList.at(1);

                        if (!strX.isEmpty() && !strY.isEmpty()) {
                            position = QPointF(strX.toFloat(), strY.toFloat());
                        }
                    }

                    // Get the (view model of) agents grouped for this name
                    AgentsGroupedByNameVM* agentsGroupedByName = _modelManager->getAgentsGroupedForName(agentName);

                    if ((agentsGroupedByName != nullptr) && (agentMapping != nullptr) && !position.isNull())
                    {
                        qDebug() << "Position:" << position.x() << position.y() << "is defined for" << agentName << "with" << agentsGroupedByName->models()->count() << "models";

                        // Create a new agent in the global mapping (with the "Agents Grouped by Name") at a specific position
                        _createAgentInMappingAtPosition(agentsGroupedByName, position);

                        // FIXME REPAIR settemporaryMapping
                        /*// Get the agent in mapping from the name
                        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);
                        if (agentInMapping != nullptr)
                        {
                            // Set the temporary mapping
                            if (agentMapping != nullptr) {
                                agentInMapping->settemporaryMapping(agentMapping);
                            }

                            // Add the link elements
                            listOfMappingElements.append(agentMapping->mappingElements()->toList());
                        }*/
                    }
                }
            }
        }

        // FIXME: Add links ?
        /*if (!listOfMappingElements.isEmpty())
        {
            // Create all mapping links
            for (ElementMappingM* mappingElement : listOfMappingElements)
            {
                if (mappingElement != nullptr) {
                    onMapped(mappingElement);
                }
            }
        }*/
    }
}


/**
 * @brief Reset the modifications made while the mapping was UN-activated
 */
void AgentsMappingController::resetModificationsWhileMappingWasUNactivated()
{
    qDebug() << "Reset the modifications made while the mapping was UN-activated";

    /*for (QString linkId : _idsOfAddedLinks_WhileMappingWasUNactivated)
    {
        qDebug() << "Remove added link" << linkId << "while the mapping was disconnected";

        // Get the view model of link which corresponds to a mapping element
        LinkVM* link = getLinkInMappingFromId(linkId);
        if (link != nullptr)
        {
            // Delete this link (between two agents) to cancel the add
            _deleteLinkBetweenTwoAgents(link);
        }
    }*/
    for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
    {
        if ((agentInMapping != nullptr) && agentInMapping->hadLinksAdded_WhileMappingWasUNactivated())
        {
            QList<QString> addedLinkIds = agentInMapping->getAllAddedLinkIds_WhileMappingWasUNactivated();
            for (QString linkId : addedLinkIds)
            {
                qDebug() << "Remove added link" << linkId << "while the mapping was disconnected";

                // Get the view model of link which corresponds to a mapping element
                LinkVM* link = getLinkInMappingFromId(linkId);
                if (link != nullptr)
                {
                    // Delete this link (between two agents) to cancel the add
                    _deleteLinkBetweenTwoAgents(link);
                }
            }

            agentInMapping->cancelAllAddedLinks_WhileMappingWasUNactivated();
        }
    }

    /*for (QString linkId : _hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated.keys())
    {
        qDebug() << "Add removed link" << linkId << "while the mapping was disconnected";

        MappingElementVM* mappingElement = _hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated.value(linkId);
        if ((mappingElement != nullptr) && (mappingElement->firstModel() != nullptr))
        {
            AgentInMappingVM* inputAgent = getAgentInMappingFromName(mappingElement->firstModel()->inputAgent());
            if (inputAgent != nullptr)
            {
                // Link the agent (in the global mapping) on its input from the mapping element (add a missing link TO the agent)
                _linkAgentOnInputFromMappingElement(inputAgent, mappingElement);
            }
        }
    }*/
    for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
    {
        if ((agentInMapping != nullptr) && agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated())
        {
            QList<MappingElementVM*> removedMappingElements = agentInMapping->getAllRemovedMappingElements_WhileMappingWasUNactivated();
            for (MappingElementVM* mappingElement : removedMappingElements)
            {
                if (mappingElement != nullptr)
                {
                    qDebug() << "Add removed link" << mappingElement->name() << "while the mapping was disconnected";

                    // Link the agent (in the global mapping) on its input from the mapping element (add a missing link TO the agent)
                    _linkAgentOnInputFromMappingElement(agentInMapping, mappingElement);
                }
            }

            agentInMapping->cancelAllRemovedLinks_WhileMappingWasUNactivated();
        }
    }


    // Clear modifications made while the mapping was UN-activated
    //_idsOfAddedLinks_WhileMappingWasUNactivated.clear();
    //_hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated.clear();


    // Update the global mapping with agents ON and their links
    _updateMappingWithAgentsONandLinks();
}


/**
 * @brief Slot called when the flag "is Mapping Activated" changed
 * @param isMappingActivated
 */
void AgentsMappingController::onIsMappingActivatedChanged(bool isMappingActivated)
{
    if ((_modelManager != nullptr) && (_jsonHelper != nullptr) && isMappingActivated)
    {
        // CONTROL
        if (_modelManager->isMappingControlled())
        {
            qDebug() << "Mapping Activated in mode CONTROL";

            // Clear modifications made while the mapping was UN-activated
            //_idsOfAddedLinks_WhileMappingWasUNactivated.clear();
            //_hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated.clear();

            for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
            {
                if (agentInMapping != nullptr)
                {
                    if (agentInMapping->hadLinksAdded_WhileMappingWasUNactivated()) {
                        agentInMapping->cancelAllAddedLinks_WhileMappingWasUNactivated();
                    }
                    if (agentInMapping->hadLinksRemoved_WhileMappingWasUNactivated()) {
                        agentInMapping->cancelAllRemovedLinks_WhileMappingWasUNactivated();
                    }
                }
            }


            // Apply all temporary mappings
            for (AgentInMappingVM* agentInMapping : _allAgentsInMapping.toList())
            {
                if ((agentInMapping != nullptr) && (agentInMapping->agentsGroupedByName() != nullptr) && (agentInMapping->agentsGroupedByName()->currentMapping() != nullptr))
                {
                    QString jsonOfMapping;

                    // FIXME REPAIR temporaryMappingElements
                    //if (agentInMapping->temporaryMappingElements()->isEmpty())
                    //{
                        // Get the JSON of the current mapping of the agent
                        jsonOfMapping = _jsonHelper->getJsonOfAgentMapping(agentInMapping->agentsGroupedByName()->currentMapping(),
                                                                           QJsonDocument::Compact);
                    /*}
                    else
                    {
                        // Get the JSON of the current mapping of the agent
                        jsonOfMapping = _jsonHelper->getJsonOfAgentTemporaryMapping(agentInMapping->agentsGroupedByName()->currentMapping(),
                                                                                    agentInMapping->temporaryMappingElements()->toList(),
                                                                                    QJsonDocument::Compact);
                    }*/

                    QString command = QString("%1%2").arg(command_LoadMapping, jsonOfMapping);

                    // Emit signal "Command asked to agent"
                    Q_EMIT commandAskedToAgent(agentInMapping->agentsGroupedByName()->peerIdsList(), command);
                }
            }
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
    if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty() && (_modelManager != nullptr))
    {
        // Connect to signals from this new agents grouped by name
        connect(agentsGroupedByName, &AgentsGroupedByNameVM::isONChanged, this, &AgentsMappingController::onAgentIsONChanged);
        connect(agentsGroupedByName, &AgentsGroupedByNameVM::mappingElementsHaveBeenAdded, this, &AgentsMappingController::onMappingElementsHaveBeenAdded);
        connect(agentsGroupedByName, &AgentsGroupedByNameVM::mappingElementsWillBeRemoved, this, &AgentsMappingController::onMappingElementsWillBeRemoved);

        QString agentName = agentsGroupedByName->name();

        // The mapping is activated
        if (_modelManager->isMappingActivated())
        {
            // CONTROL
            if (_modelManager->isMappingControlled())
            {
                qDebug() << "CONTROL:" << agentName << "has been created. CLEAR its MAPPING !";

                // Send the command "Clear Mapping" on the network to this agent(s)
                Q_EMIT commandAskedToAgent(agentsGroupedByName->peerIdsList(), command_ClearMapping);
            }
            // OBSERVE
            else
            {
                qDebug() << "OBSERVE:" << agentName << "has been created. ADD it in the MAPPING view !";

                double randomMax = (double)RAND_MAX;

                // Get a random position in the current window
                QPointF position = _getRandomPosition(randomMax);

                // Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position
                AgentInMappingVM* agentInMapping = _createAgentInMappingAtPosition(agentsGroupedByName, position);

                if (agentInMapping != nullptr)
                {
                    // No need to add links on inputs now, because the mapping will be received after the creation of this agent(s grouped by name)
                    // Link the agent in the global mapping on its inputs (add all missing links TO the agent)
                    //_linkAgentOnInputs(agentInMapping);

                    // Link the agent in the global mapping on its outputs (add all missing links FROM the agent)
                    _linkAgentOnOutputs(agentInMapping);
                }
            }
        }
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
        disconnect(agentsGroupedByName, 0, this, 0);

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
 * @brief Slot when an active agent has been defined
 * @param agent
 */
/*void AgentsMappingController::onActiveAgentDefined(AgentM* agent)
{
    if ((agent != nullptr) && (_modelManager != nullptr))
    {
        QString agentName = agent->name();

        // Get the agent in mapping for the agent name
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);

        // The mapping is activated
        if (_modelManager->isMappingActivated())
        {
            // CONTROL
            if (_modelManager->isMappingControlled())
            {
                QStringList peerIdsList = QStringList(agent->peerId());

                // The agent is not yet in the mapping...
                if (agentInMapping == nullptr)
                {
                    qDebug() << "CONTROL: Model of" << agentName << "is defined. CLEAR its MAPPING !";

                    // Send the command "Clear Mapping" on the network to this agent
                    Q_EMIT commandAskedToAgent(peerIdsList, command_ClearMapping);
                }
                // The agent is already in the mapping
                else
                {
                    // OverWrite the mapping of the model of agent (with the mapping currently edited in the agent in mapping)
                    _overWriteMappingOfAgentModel(agent, agentInMapping->temporaryMapping());
                }
            }
            // OBSERVE
            else
            {
                //qDebug() << "OBSERVE: Model of" << agentName << "is defined. ADD in MAPPING view !" << agent;

                // The agent is not in the mapping
                if (agentInMapping == nullptr)
                {
                    // Get the (view model of) agents grouped for a name
                    AgentsGroupedByNameVM* agentsGroupedByName = _modelManager->getAgentsGroupedForName(agentName);

                    // If our model is the first (and only) one, we add the agent to the mapping
                    if ((agentsGroupedByName != nullptr) && (agentsGroupedByName->models()->count() == 1))
                    {
                        double randomMax = (double)RAND_MAX;

                        // Get a random position in the current window
                        QPointF position = _getRandomPosition(randomMax);

                        // Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position
                        _createAgentInMappingAtPosition(agentsGroupedByName, position);
                    }
                }
                // The agent is already in the mapping
                else
                {
                    // Nothing to do...already added to AgentsGroupedByName
                }

                // If there are waiting links (where this agent is involved as "Output Agent")
                if (_hashFromOutputAgentNameToListOfWaitingMappingElements.contains(agentName))
                {
                    QList<ElementMappingM*> listOfWaitingMappingElements = _hashFromOutputAgentNameToListOfWaitingMappingElements.value(agentName);
                    for (ElementMappingM* mappingElement : listOfWaitingMappingElements)
                    {
                        qDebug() << "Create waiting MAP..." << mappingElement->name();

                        // Create the link corresponding to the mapping element
                        //onMapped(mappingElement);
                    }
                }
            }
        }
        // The mapping is NOT activated
        else
        {
            // Nothing to do...already added to AgentsGroupedByName

            // FIXME: Even if the definition is different, the agent in mapping will be modified !
            // What do we do with the mapping (links) of this agent ?
        }
    }
}*/


/**
 * @brief Slot called when the mapping of an active agent has been defined
 * @param agent
 */
/*void AgentsMappingController::onActiveAgentMappingDefined(AgentM* agent)
{
    if ((agent != nullptr) && (agent->mapping() != nullptr) && (_modelManager != nullptr) && _modelManager->isMappingActivated())
    {
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agent->name());
        if ((agentInMapping != nullptr) && (agentInMapping->temporaryMapping() != nullptr))
        {
            QStringList namesOfRemovedMappingElements;
            for (QString namePreviousList : agentInMapping->temporaryMapping()->namesOfMappingElements())
            {
                if (!agent->mapping()->namesOfMappingElements().contains(namePreviousList)) {
                    namesOfRemovedMappingElements.append(namePreviousList);
                }
            }

            QStringList namesOfAddedMappingElements;
            for (QString nameNewList : agent->mapping()->namesOfMappingElements())
            {
                if (!agentInMapping->temporaryMapping()->namesOfMappingElements().contains(nameNewList)) {
                    namesOfAddedMappingElements.append(nameNewList);
                }
            }

            // If there are some Removed mapping elements
            if (!namesOfRemovedMappingElements.isEmpty())
            {
                qDebug() << "unmapped" << namesOfRemovedMappingElements;

                for (ElementMappingM* mappingElement : agentInMapping->temporaryMapping()->mappingElements()->toList())
                {
                    if ((mappingElement != nullptr) && namesOfRemovedMappingElements.contains(mappingElement->name()))
                    {
                        onUnmapped(mappingElement);
                    }
                }
            }
            // If there are some Added mapping elements
            if (!namesOfAddedMappingElements.isEmpty())
            {
                qDebug() << "mapped" << namesOfAddedMappingElements;

                for (ElementMappingM* mappingElement : agent->mapping()->mappingElements()->toList())
                {
                    if ((mappingElement != nullptr) && namesOfAddedMappingElements.contains(mappingElement->name()))
                    {
                        onMapped(mappingElement);
                    }
                }
            }
        }
    }
}*/


/**
 * @brief Slot when two agents are mapped
 * @param mappingElement
 */
/*void AgentsMappingController::onMapped(ElementMappingM* mappingElement)
{
    if (mappingElement != nullptr)
    {
        //qDebug() << "MAPPED" << mappingElement->name();

        // Get the (view model of) link between agents in the global mapping from a link name
        LinkVM* link = getLinkInMappingFromName(mappingElement->name());

        if (link == nullptr)
        {
            AgentInMappingVM* outputAgent = getAgentInMappingFromName(mappingElement->outputAgent());
            AgentInMappingVM* inputAgent = getAgentInMappingFromName(mappingElement->inputAgent());

            if ((outputAgent != nullptr) && (inputAgent != nullptr))
            {
                LinkOutputVM* linkOutput = nullptr;
                LinkInputVM* linkInput = nullptr;

                if (outputAgent->agentsGroupedByName() != nullptr)
                {
                    // Get the list of view models of link output from the output name
                    QList<LinkOutputVM*> linkOutputsWithSameName = outputAgent->getLinkOutputsListFromName(mappingElement->output());
                    if (linkOutputsWithSameName.count() == 1) {
                        linkOutput = linkOutputsWithSameName.at(0);
                    }
                    else {
                        qWarning() << "There are" << linkOutputsWithSameName.count() << "link outputs with the same name" << mappingElement->output() << "."
                                   << "We cannot choose and create the link" << mappingElement->name();
                    }
                }

                if (inputAgent->agentsGroupedByName() != nullptr)
                {
                    // Get the list of view models of input from the input name
                    QList<LinkInputVM*> linkInputsWithSameName = inputAgent->getLinkInputsListFromName(mappingElement->input());
                    if (linkInputsWithSameName.count() == 1) {
                        linkInput = linkInputsWithSameName.at(0);
                    }
                    else {
                        qWarning() << "There are" << linkInputsWithSameName.count() << "link inputs with the same name" << mappingElement->input() << "."
                                   << "We cannot choose and create the link" << mappingElement->name();
                    }
                }

                if ((linkOutput != nullptr) && (linkInput != nullptr))
                {
                    qInfo() << "MAPPED" << mappingElement->name();

                    // Create a new map between agents
                    link = _createLinkBetweenTwoAgents(outputAgent, linkOutput, inputAgent, linkInput, false, this);

                    // If there is a list of waiting links (where the agent is involved as "Output Agent")
                    if (_hashFromOutputAgentNameToListOfWaitingMappingElements.contains(mappingElement->outputAgent()))
                    {
                        QList<ElementMappingM*> listOfWaitingMappingElements = _hashFromOutputAgentNameToListOfWaitingMappingElements.value(mappingElement->outputAgent());
                        if (listOfWaitingMappingElements.contains(mappingElement))
                        {
                            // Remove the mapping element from the list because the corresponding link is really created
                            listOfWaitingMappingElements.removeOne(mappingElement);

                            // Update the hash table
                            _hashFromOutputAgentNameToListOfWaitingMappingElements.insert(mappingElement->outputAgent(), listOfWaitingMappingElements);
                        }
                    }
                }
            }
            // Output agent is not yet in the mapping
            else if (inputAgent != nullptr) // && (outputAgent == nullptr)
            {
                QList<ElementMappingM*> listOfWaitingMappingElements;

                if (_hashFromOutputAgentNameToListOfWaitingMappingElements.contains(mappingElement->outputAgent())) {
                    listOfWaitingMappingElements = _hashFromOutputAgentNameToListOfWaitingMappingElements.value(mappingElement->outputAgent());
                }

                // Add the mapping element to the list to create the corresponding link later
                listOfWaitingMappingElements.append(mappingElement);

                // Update the hash table
                _hashFromOutputAgentNameToListOfWaitingMappingElements.insert(mappingElement->outputAgent(), listOfWaitingMappingElements);

                qDebug() << "MAP will be created later:" << mappingElement->name();
            }
        }
        else
        {
            if (link->isTemporary())
            {
                qInfo() << "MAPPED" << mappingElement->name();

                // Update the flag if needed
                link->setisTemporary(false);
            }
        }
    }
}*/


/**
 * @brief Slot when two agents are unmapped
 * @param mappingElement
 */
/*void AgentsMappingController::onUnmapped(ElementMappingM* mappingElement)
{
    if (mappingElement != nullptr)
    {
        //qInfo() << "UN-mapped" << mappingElement->name();

        // Get the (view model of) link between agents in the global mapping from a link name
        LinkVM* link = getLinkInMappingFromName(mappingElement->name());

        if (link != nullptr)
        {
            if (link->inputAgent() != nullptr)
            {
                // Remove the temporary link that correspond to this real link
                //bool hasBeenRemoved = link->inputAgent()->removeTemporaryLink(mappingElement->input(), mappingElement->outputAgent(), mappingElement->output());
                link->inputAgent()->removeTemporaryLink(mappingElement->input(), mappingElement->outputAgent(), mappingElement->output());
            }

            // Delete the link between two agents
            _deleteLinkBetweenTwoAgents(link);
        }
    }
}*/


/**
 * @brief Slot called when the flag "is ON" of an agent changed
 * @param isON
 */
void AgentsMappingController::onAgentIsONChanged(bool isON)
{
    if (isON)
    {
        AgentsGroupedByNameVM* agentsGroupedByName = qobject_cast<AgentsGroupedByNameVM*>(sender());
        if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty())
        {
            // Check that there is NOT yet an agent in the current mapping for this name
            AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentsGroupedByName->name());

            if ((agentInMapping == nullptr) && (_modelManager != nullptr))
            {
                // The mapping is activated
                if (_modelManager->isMappingActivated())
                {
                    // CONTROL
                    if (_modelManager->isMappingControlled())
                    {
                        // FIXME TODO ? CONTROL

                        // FIXME REPAIR temporaryMappingElements
                        /*if (!agentInMapping->temporaryMappingElements()->isEmpty())
                        {

                        }*/
                    }
                    // OBSERVE
                    else
                    {
                        double randomMax = (double)RAND_MAX;

                        // Get a random position in the current window
                        QPointF position = _getRandomPosition(randomMax);

                        // Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position
                        agentInMapping = _createAgentInMappingAtPosition(agentsGroupedByName, position);

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
}


/**
 * @brief Slot called when some view models of mapping elements have been added to an agent(s grouped by name)
 * @param newMappingElements
 */
void AgentsMappingController::onMappingElementsHaveBeenAdded(QList<MappingElementVM*> newMappingElements)
{
    AgentsGroupedByNameVM* agentsGroupedByName = qobject_cast<AgentsGroupedByNameVM*>(sender());
    if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty() && !newMappingElements.isEmpty())
    {
        // Get the input agent in the global mapping from the agent name (outside loop "for")
        AgentInMappingVM* inputAgent = getAgentInMappingFromName(agentsGroupedByName->name());
        if (inputAgent != nullptr)
        {
            for (MappingElementVM* mappingElement : newMappingElements)
            {
                if (mappingElement != nullptr)
                {
                    qInfo() << mappingElement->name() << "MAPPED";

                    // Link the agent on its input from the mapping element (add a missing link TO the agent)
                    _linkAgentOnInputFromMappingElement(inputAgent, mappingElement);
                }
            }
        }
    }
}


/**
 * @brief Slot called when some view models of mapping elements will be removed from an agent(s grouped by name)
 * @param oldMappingElements
 */
void AgentsMappingController::onMappingElementsWillBeRemoved(QList<MappingElementVM*> oldMappingElements)
{
    //AgentsGroupedByNameVM* agentsGroupedByName = qobject_cast<AgentsGroupedByNameVM*>(sender());
    //if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty() && !oldMappingElements.isEmpty())
    if (!oldMappingElements.isEmpty())
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
                    QStringList namesList = ElementMappingM::getNamesListFromLinkName(linkName);
                    if (namesList.count() == 4)
                    {
                        QString outputAgentName = namesList.at(0);
                        //QString outputName = namesList.at(1);
                        //QString inputAgentName = namesList.at(2);
                        //QString inputName = namesList.at(3);

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

                        // Delete the link between two agents
                        _deleteLinkBetweenTwoAgents(link);
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
 * @brief Slot called when we receive the command highlight link from a recorder
 * @param parameters
 */
void AgentsMappingController::onHighlightLink(QStringList parameters)
{
    if (parameters.count() == 4)
    {
        QString inputAgentName = parameters.at(0);
        QString inputName = parameters.at(1);
        QString outputAgentName = parameters.at(2);
        QString outputName = parameters.at(3);

        // Get the link name (with format "outputAgent##output-->inputAgent##input") from the list of names (each parts of a mapping element)
        QString linkName = ElementMappingM::getLinkNameFromNamesList(outputAgentName, outputName, inputAgentName, inputName);

        // Get the list of links between agents in the global mapping from a link name
        QList<LinkVM*> linksWithSameName = getLinksInMappingFromName(linkName);

        if (linksWithSameName.count() == 1)
        {
            LinkVM* link = linksWithSameName.at(0);
            if ((link != nullptr) && (link->linkOutput() != nullptr))
            {
                qDebug() << "Highlight the link" << linkName;

                // Simulate that the current value of output model changed: allows to highlight the link
                link->linkOutput()->simulateCurrentValueOfModelChanged();
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
    if (_allAgentsInMapping.isEmpty()) {
        setisEmptyMapping(true);
    }
    else {
        setisEmptyMapping(false);
    }
}


/**
 * @brief Slot called when some view models of link inputs have been added to an agent in mapping
 * @param addedlinkInputs
 */
void AgentsMappingController::_onLinkInputsListHaveBeenAdded(QList<LinkInputVM*> addedlinkInputs)
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
void AgentsMappingController::_onLinkOutputsListHaveBeenAdded(QList<LinkOutputVM*> addedlinkOutputs)
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
void AgentsMappingController::_onLinkInputsListWillBeRemoved(QList<LinkInputVM*> removedLinkInputs)
{
    AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(sender());
    if ((agentInMapping != nullptr) && !removedLinkInputs.isEmpty())
    {
        // Traverse the list of all links
        for (LinkVM* link : _allLinksInMapping.toList())
        {
            if ((link != nullptr) && (link->inputAgent() != nullptr) && (link->inputAgent() == agentInMapping)
                    && (link->linkInput() != nullptr) && removedLinkInputs.contains(link->linkInput()))
            {
                // Delete the link between two agents
                _deleteLinkBetweenTwoAgents(link);
            }
        }
    }
}


/**
 * @brief Slot called when some view models of link outputs will be removed from an agent in mapping
 * @param removedLinkOutputs
 */
void AgentsMappingController::_onLinkOutputsListWillBeRemoved(QList<LinkOutputVM*> removedLinkOutputs)
{
    AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(sender());
    if ((agentInMapping != nullptr) && !removedLinkOutputs.isEmpty())
    {
        qDebug() << "_on Link Outputs List will be Removed from agent" << agentInMapping->name() << removedLinkOutputs.count();

        // Traverse the list of all links
        for (LinkVM* link : _allLinksInMapping.toList())
        {
            if ((link != nullptr) && (link->outputAgent() != nullptr) && (link->outputAgent() == agentInMapping)
                    && (link->linkOutput() != nullptr) && removedLinkOutputs.contains(link->linkOutput()))
            {
                if (link->mappingElement() != nullptr)
                {
                     // Add a "Waiting Mapping Element" on the output agent (name)
                    _addWaitingMappingElementOnOutputAgent(link->outputAgent()->name(), link->mappingElement());
                }

                // Delete the link between two agents
                _deleteLinkBetweenTwoAgents(link);
            }
        }
    }
}


/**
 * @brief Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position
 * @param agentsGroupedByName
 * @param position
 * @return
 */
AgentInMappingVM* AgentsMappingController::_createAgentInMappingAtPosition(AgentsGroupedByNameVM* agentsGroupedByName, QPointF position)
{
    AgentInMappingVM* agentInMapping = nullptr;

    if ((agentsGroupedByName != nullptr) && !agentsGroupedByName->name().isEmpty())
    {
        agentInMapping = getAgentInMappingFromName(agentsGroupedByName->name());

        // Check that there is NOT yet an agent in the global mapping for this name
        if (agentInMapping == nullptr)
        {
            // Create a new view model of agent in the global Mapping
            agentInMapping = new AgentInMappingVM(agentsGroupedByName, position, this);

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
 * @brief Create a link between two agents
 * @param linkName
 * @param outputAgent
 * @param linkOutput
 * @param inputAgent
 * @param linkInput
 * @param mappingElement
 * @param isTemporary
 * @return
 */
LinkVM* AgentsMappingController::_createLinkBetweenTwoAgents(QString linkName,
                                                             AgentInMappingVM* outputAgent,
                                                             LinkOutputVM* linkOutput,
                                                             AgentInMappingVM* inputAgent,
                                                             LinkInputVM* linkInput,
                                                             MappingElementVM* mappingElement,
                                                             bool isTemporary)
{
    LinkVM* link = nullptr;

    if ((outputAgent != nullptr) && (linkOutput != nullptr)
            && (inputAgent != nullptr) && (linkInput != nullptr))
    {
        // Get the link id (with format "outputAgent##output::outputType-->inputAgent##input::inputType") from agent names and Input/Output ids
        QString linkId = LinkVM::getLinkIdFromAgentNamesAndIOids(outputAgent->name(), linkOutput->uid(), inputAgent->name(), linkInput->uid());

        link = getLinkInMappingFromId(linkId);

        // Check that there is NOT yet a link in the global mapping for this id
        if (link == nullptr)
        {
            // Create a new link between two agents
            link = new LinkVM(linkName,
                              mappingElement,
                              outputAgent,
                              linkOutput,
                              inputAgent,
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
 * @brief Delete a link between two agents
 * @param link
 */
void AgentsMappingController::_deleteLinkBetweenTwoAgents(LinkVM* link)
{
    if (link != nullptr)
    {
        qInfo() << "Delete the link" << link->name() << "in the Mapping";

        // Unselect our link if needed
        if (_selectedLink == link) {
            setselectedLink(nullptr);
        }

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
    if ((agent != nullptr) && !_allLinksInMapping.isEmpty())
    {
        for (LinkVM* link : _allLinksInMapping.toList())
        {
            if ( (link != nullptr) && ((link->outputAgent() == agent) || (link->inputAgent() == agent)) )
            {
                // Remove a link between two agents from the mapping
                bool linkHasBeenDeleted = removeLinkBetweenTwoAgents(link);

                if (!linkHasBeenDeleted)
                {
                    // We have to delete the link to clean our HMI (even if we do not have yet received the message "UNMAPPED")
                    _deleteLinkBetweenTwoAgents(link);
                }
            }
        }
    }
}


/**
 * @brief OverWrite the mapping of the model of agent (with the mapping currently edited in the agent in mapping)
 * @param agentModel
 * @param temporaryMapping
 */
/*void AgentsMappingController::_overWriteMappingOfAgentModel(AgentM* agentModel, AgentMappingM* temporaryMapping)
{
    // Model is ON
    // AND
    // Global mapping is activated and controlled !
    if ((agentModel != nullptr) && agentModel->isON() && (temporaryMapping != nullptr) && (_jsonHelper != nullptr)
            && (_modelManager != nullptr) && _modelManager->isMappingActivated() && _modelManager->isMappingControlled())
    {
        QStringList peerIdsList = QStringList(agentModel->peerId());

        // Get the JSON of the agent mapping
        QString jsonOfMapping = _jsonHelper->getJsonOfAgentMapping(temporaryMapping, QJsonDocument::Compact);

        QString command = QString("%1%2").arg(command_LoadMapping, jsonOfMapping);

        // Emit signal "Command asked to agent"
        Q_EMIT commandAskedToAgent(peerIdsList, command);
    }
}*/


/**
 * @brief Update the global mapping with agents ON and their links
 */
void AgentsMappingController::_updateMappingWithAgentsONandLinks()
{
    if (_modelManager != nullptr)
    {
        double randomMax = (double)RAND_MAX;

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

                    // Create a new agent in the global mapping (with an "Agents Grouped by Name") at a specific position
                    _createAgentInMappingAtPosition(agentsGroupedByName, position);
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
            qDebug() << "Try to create link" << linkName << "TO agent" << inputAgent->name() << "involved as 'INPUT' agent";

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
                            _createLinkBetweenTwoAgents(linkName,
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
                    else
                    {
                        if (link->inputAgent() != nullptr)
                        {
                            // FIXME TODO: WhileMappingWasUNactivated
                        }
                    }

                    // FIXME REPAIR removeTemporaryMappingElement
                    /*if (link->inputAgent() != nullptr) {
                        link->inputAgent()->removeTemporaryMappingElement(link->uid());
                    }*/
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
                            _createLinkBetweenTwoAgents(linkName,
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
    double randomX = (double)qrand() / randomMax;
    double randomY = (double)qrand() / randomMax;

    // 5% + (random * 90% of the width)
    double x = 0.05 * _viewWidth + (0.90 * _viewWidth * randomX);

    // 5% + (random * 90% of the height)
    double y = 0.05 * _viewHeight + (0.90 * _viewHeight * randomY);

    return QPointF(x, y);
}


/**
 * @brief Get the list of "Waiting Mapping Elements" on an output agent (name)
 * @param outputAgentName
 * @return
 */
QList<MappingElementVM*> AgentsMappingController::_getWaitingMappingElementsOnOutputAgent(QString outputAgentName)
{
    if (_hashFromOutputAgentNameToListOfWaitingMappingElements.contains(outputAgentName)) {
        return _hashFromOutputAgentNameToListOfWaitingMappingElements.value(outputAgentName);
    }
    else {
        return QList<MappingElementVM*>();
    }
}


/**
 * @brief Add a "Waiting Mapping Element" on an output agent (name)
 * @param outputAgentName
 * @param waitingMappingElement
 */
void AgentsMappingController::_addWaitingMappingElementOnOutputAgent(QString outputAgentName, MappingElementVM* waitingMappingElement)
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
void AgentsMappingController::_removeWaitingMappingElementOnOutputAgent(QString outputAgentName, MappingElementVM* waitingMappingElement)
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
LinkInputVM* AgentsMappingController::_getAloneLinkInputFromName(AgentInMappingVM* agent, QString inputName, QString linkName)
{
    LinkInputVM* linkInput = nullptr;

    if ((agent != nullptr) && !inputName.isEmpty())
    {
        // Get the list of view models of link input from the input name
        QList<LinkInputVM*> linkInputsWithSameName = agent->getLinkInputsListFromName(inputName);

        if (linkInputsWithSameName.count() == 1) {
            linkInput = linkInputsWithSameName.at(0);
        }
        else {
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
LinkOutputVM* AgentsMappingController::_getAloneLinkOutputFromName(AgentInMappingVM* agent, QString outputName, QString linkName)
{
    LinkOutputVM* linkOutput = nullptr;

    if ((agent != nullptr) && !outputName.isEmpty())
    {
        // Get the list of view models of link output from the output name
        QList<LinkOutputVM*> linkOutputsWithSameName = agent->getLinkOutputsListFromName(outputName);

        if (linkOutputsWithSameName.count() == 1) {
            linkOutput = linkOutputsWithSameName.at(0);
        }
        else {
            qWarning() << "There are" << linkOutputsWithSameName.count() << "link outputs with the same name" << outputName << "."
                       << "We cannot choose and create the link" << linkName;
        }
    }
    return linkOutput;
}

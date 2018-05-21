/*
 *	IngeScape Editor
 *
 *  Copyright © 2017 Ingenuity i/o. All rights reserved.
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
 * @brief Default constructor
 * @param modelManager
 * @param mapping directory path
 * @param parent
 */
AgentsMappingController::AgentsMappingController(IngeScapeModelManager* modelManager,
                                                 QString mappingsPath,
                                                 QObject *parent)
    : QObject(parent),
      _viewWidth(1920 - 320), // Full HD - Width of left panel
      _viewHeight(1080 - 100), // Full HD - Height of top & bottom bars of OS
      _isEmptyMapping(true),
      _selectedAgent(NULL),
      _selectedLink(NULL),
      _modelManager(modelManager),
      _mappingsDirectoryPath(mappingsPath)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    if (_modelManager != NULL)
    {   
        // Connect to signal "Count Changed" from the list of agents in mapping
        connect(&_allAgentsInMapping, &AbstractI2CustomItemListModel::countChanged, this, &AgentsMappingController::_onAgentsInMappingChanged);
    }

    // Create the helper to manage JSON definitions of agents
    _jsonHelper = new JsonHelper(this);
}


/**
 * @brief Destructor
 */
AgentsMappingController::~AgentsMappingController()
{
    // Clean-up current selections
    setselectedAgent(NULL);
    setselectedLink(NULL);

    // DIS-connect from signal "Count Changed" from the list of agents in mapping
    disconnect(&_allAgentsInMapping, 0, this, 0);

    // Delete all links
    _allLinksInMapping.deleteAllItems();
    _hashFromAgentNameToListOfWaitingLinks.clear();

    // Delete all agents in mapping
    _previousListOfAgentsInMapping.clear();
    _allAgentsInMapping.deleteAllItems();

    _modelManager = NULL;

    // Delete json helper
    if(_jsonHelper != NULL)
    {
        delete _jsonHelper;
        _jsonHelper = NULL;
    }
}


/**
 * @brief Create a new Mapping
 */
void AgentsMappingController::createNewMapping()
{
    qInfo() << "Clear current (previous) mapping";

    // 1- First, DE-activate the mapping
    if (_modelManager != NULL) {
        _modelManager->setisMappingActivated(false);
    }

    // 2- Delete all links
    foreach (MapBetweenIOPVM* link, _allLinksInMapping.toList()) {
        _deleteLinkBetweenTwoAgents(link);
    }

    // 3- Delete all agents in mapping
    foreach (AgentInMappingVM* agent, _allAgentsInMapping.toList()) {
        deleteAgentInMapping(agent);
    }

    qInfo() << "Create a new (empty) Mapping";
}


/**
 * @brief Open a Mapping
 */
void AgentsMappingController::openMapping()
{
    // "File Dialog" to get the files (paths) to open
    QString mappingFilePath = QFileDialog::getOpenFileName(NULL,
                                                                "Open mapping",
                                                                _mappingsDirectoryPath,
                                                                "JSON (*.json)");

    // Open the mapping from JSON file
    _openMappingFromFile(mappingFilePath);
}

/**
 * @brief Save a Mapping
 */
void AgentsMappingController::saveMapping()
{
    // "File Dialog" to get the file (path) to save
    QString mappingFilePath = QFileDialog::getSaveFileName(NULL,
                                                              "Save mapping",
                                                              _mappingsDirectoryPath,
                                                              "JSON (*.json)");

    if(!mappingFilePath.isEmpty()) {
        // Save the mapping to JSON file
        _saveMappingToFile(mappingFilePath);
    }
}

/**
 * @brief Open the mapping from JSON file
 * @param mappingFilePath
 */
void AgentsMappingController::_openMappingFromFile(QString mappingFilePath)
{
    if (!mappingFilePath.isEmpty() && (_jsonHelper != NULL))
    {
        qInfo() << "Open the mapping from JSON file" << mappingFilePath;

        QFile jsonFile(mappingFilePath);
        if (jsonFile.exists())
        {
            if (jsonFile.open(QIODevice::ReadOnly))
            {

                QByteArray byteArrayOfJson = jsonFile.readAll();
                jsonFile.close();

                // Import the new mapping
                importMappingFromJson(byteArrayOfJson);
            }
            else {
                qCritical() << "Can not open file" << mappingFilePath;
            }
        }
        else {
            qWarning() << "There is no file" << mappingFilePath;
        }
    }
}

/**
 * @brief Save the mapping to JSON file
 * @param mappingFilePath
 */
void AgentsMappingController::_saveMappingToFile(QString mappingFilePath)
{
    if (!mappingFilePath.isEmpty() && (_jsonHelper != NULL))
    {
        qInfo() << "Save the mapping to JSON file" << mappingFilePath;

        QJsonArray jsonArray = _jsonHelper->exportAllAgentsInMapping(_allAgentsInMapping.toList());

        if(jsonArray.count() > 0)
        {
            // Create json document
            QJsonDocument jsonDocument = QJsonDocument(jsonArray);
            QByteArray byteArrayOfJson = jsonDocument.toJson();

            // Write file
            QFile jsonFile(mappingFilePath);
            if (jsonFile.open(QIODevice::WriteOnly))
            {
                jsonFile.write(byteArrayOfJson);
                jsonFile.close();
            }
            else {
                qCritical() << "Can not open file" << mappingFilePath;
            }
        }
    }
}



/**
 * @brief Remove the agent from the mapping and delete it
 * @param agent
 */
void AgentsMappingController::deleteAgentInMapping(AgentInMappingVM* agent)
{
    if (agent != NULL)
    {
        qInfo() << "Delete Agent in Mapping" << agent->name();

        // Unselect our agent if needed
        if (_selectedAgent == agent) {
            setselectedAgent(NULL);
        }

        // Remove from hash table
        _mapFromNameToAgentInMapping.remove(agent->name());

        // Remove all the links with this agent
        _removeAllLinksWithAgent(agent);

        // Remove this Agent In Mapping from the list to update view (QML)
        _allAgentsInMapping.remove(agent);

        // Free memory
        delete agent;
    }
}

/**
 * @brief Remove the selected agent from the mapping and delete it
 */
void AgentsMappingController::deleteSelectedAgentInMapping()
{
    if(_selectedAgent != NULL)
    {
        deleteAgentInMapping(_selectedAgent);
    }
}


/**
 * @brief Remove a link between two agents from the mapping
 * @param link
 */
void AgentsMappingController::removeLinkBetweenTwoAgents(MapBetweenIOPVM* link)
{
    if ((link != NULL) && (link->inputAgent() != NULL) && (link->input() != NULL) && (link->outputAgent() != NULL) && (link->output() != NULL))
    {
        qInfo() << "QML asked to delete the link between agents" << link->outputAgent()->name() << "and" << link->inputAgent()->name();

        // Mapping is activated
        if ((_modelManager != NULL) && _modelManager->isMappingActivated())
        {
            // Set to virtual to give a feedback to the user
            link->setisVirtual(true);

            // Emit signal "Command asked to agent about Mapping Input"
            Q_EMIT commandAskedToAgentAboutMappingInput(link->inputAgent()->peerIdsList(), "UNMAP", link->input()->name(), link->outputAgent()->name(), link->output()->name());
        }
        // Mapping is NOT activated
        else
        {
            // Remove temporary link (this temporary link will be removed when the user will activate the mapping)
            link->inputAgent()->removeTemporaryLink(link->input()->name(), link->outputAgent()->name(), link->output()->name());

            // Delete the link between two agents
            _deleteLinkBetweenTwoAgents(link);
        }
    }
}


/**
 * @brief Called when an agent from the list is dropped on the current mapping at a position
 * @param agentName
 * @param models
 * @param position
 */
void AgentsMappingController::dropAgentToMappingAtPosition(QString agentName, AbstractI2CustomItemListModel* models, QPointF position)
{
    // Check that there is NOT yet an agent in the current mapping for this name
    AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);
    if (agentInMapping == NULL)
    {
        I2CustomItemListModel<AgentM>* agentsList = dynamic_cast<I2CustomItemListModel<AgentM>*>(models);
        if (agentsList != NULL)
        {
            // Add new model(s) of agent to the current mapping at a specific position
            _addAgentModelsToMappingAtPosition(agentName, agentsList->toList(), position);

            agentInMapping = getAgentInMappingFromName(agentName);
            if (agentInMapping != NULL)
            {
                // Get the mapping currently edited (temporary until the user activate the mapping)
                AgentMappingM* temporaryMapping = agentInMapping->temporaryMapping();
                if (temporaryMapping != NULL)
                {
                    // Delete all "mapping elements" in this temporary mapping
                    temporaryMapping->mappingElements()->deleteAllItems();

                    // Mapping is already activated
                    if ((_modelManager != NULL) && _modelManager->isMappingActivated())
                    {
                        foreach (AgentM* model, agentsList->toList()) {
                            if (model != NULL)
                            {
                                // OverWrite the mapping of the model of agent (with the mapping currently edited in the agent in mapping)
                                _overWriteMappingOfAgentModel(model, temporaryMapping);
                            }
                        }
                    }
                }

                // Selects this new agent
                setselectedAgent(agentInMapping);
            }
        }
    }
}


/**
 * @brief Slot when a link from an output is dropped over an input on the current mapping (or when a link to an input is dropped over an output)
 * @param outputAgent
 * @param output
 * @param inputAgent
 * @param input
 */
void AgentsMappingController::dropLinkBetweenAgents(AgentInMappingVM* outputAgent, OutputVM* output, AgentInMappingVM* inputAgent, InputVM* input)
{
    if ((outputAgent != NULL) && (output != NULL) && (inputAgent != NULL) && (input != NULL))
    {
        // Check that the input can link to the output
        if (input->canLinkWith(output))
        {
            // Search if the same link already exists
            bool alreadyLinked = false;
            foreach (MapBetweenIOPVM* iterator, _allLinksInMapping.toList()) {
                if ((iterator != NULL) && (iterator->outputAgent() == outputAgent) && (iterator->output() == output)
                        && (iterator->inputAgent() == inputAgent) && (iterator->input() == input)) {
                    alreadyLinked = true;
                    break;
                }
            }

            // Check that the same link does not yet exist
            if (!alreadyLinked)
            {
                qInfo() << "QML asked to create the link between agents" << outputAgent->name() << "and" << inputAgent->name();

                // Mapping is activated
                if ((_modelManager != NULL) && _modelManager->isMappingActivated())
                {
                    // Create a new VIRTUAL link between agents
                    MapBetweenIOPVM* link = new MapBetweenIOPVM(outputAgent, output, inputAgent, input, true, this);

                    // Add to the list
                    _allLinksInMapping.append(link);

                    // Emit signal "Command asked to agent about Mapping Input"
                    Q_EMIT commandAskedToAgentAboutMappingInput(inputAgent->peerIdsList(), "MAP", input->name(), outputAgent->name(), output->name());
                }
                // Mapping is NOT activated
                else
                {
                    // Add a temporary link (this temporary link will became a real link when the user will activate the mapping)
                    inputAgent->addTemporaryLink(input->name(), outputAgent->name(), output->name());

                    // Create a new link between agents
                    MapBetweenIOPVM* link = new MapBetweenIOPVM(outputAgent, output, inputAgent, input, false, this);

                    // Add to the list
                    _allLinksInMapping.append(link);
                }
            }
            else {
                qWarning() << "The input" << input->name() << "(of agent" << inputAgent->name() << ") is already linked to output" << output->name() << "(of agent" << outputAgent->name() << ")";
            }
        }
        else {
            if ((output->firstModel() != NULL) && (input->firstModel() != NULL)) {
                qDebug() << "Can not link output" << output->name() << "with type" << AgentIOPValueTypes::staticEnumToString(output->firstModel()->agentIOPValueType()) << "(of agent" << outputAgent->name() << ")"
                         << "and input" << input->name() << "with type" << AgentIOPValueTypes::staticEnumToString(input->firstModel()->agentIOPValueType()) << "(of agent" << inputAgent->name() << ")";
            }
        }
    }
}


/**
 * @brief Get the agent in mapping from an agent name
 * @param name
 * @return
 */
AgentInMappingVM* AgentsMappingController::getAgentInMappingFromName(QString name)
{
    if (_mapFromNameToAgentInMapping.contains(name)) {
        return _mapFromNameToAgentInMapping.value(name);
    }
    else {
        return NULL;
    }
}


/**
 * @brief Slot when a previous agent model is replaced by a new one strictly identical
 * @param previousModel
 * @param newModel
 */
void AgentsMappingController::onIdenticalAgentModelReplaced(AgentM* previousModel, AgentM* newModel)
{
    if ((previousModel != NULL) && (newModel != NULL))
    {
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(newModel->name());
        if (agentInMapping != NULL)
        {
            int index = agentInMapping->models()->indexOf(previousModel);
            if (index > -1)
            {
                agentInMapping->models()->replace(index, newModel);

                // Our global mapping is controlled
                if (_modelManager && _modelManager->isMappingControlled()) {
                    // OverWrite the mapping of the model of agent (with the mapping currently edited in the agent in mapping)
                    _overWriteMappingOfAgentModel(newModel, agentInMapping->temporaryMapping());
                }
            }
        }
    }
}


/**
 * @brief Slot when an identical agent model is added
 * @param newModel
 */
void AgentsMappingController::onIdenticalAgentModelAdded(AgentM* newModel)
{
    if (newModel != NULL)
    {
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(newModel->name());
        if (agentInMapping != NULL)
        {
            agentInMapping->models()->append(newModel);

            // Our global mapping is controlled
            if (_modelManager && _modelManager->isMappingControlled()) {
                // OverWrite the mapping of the model of agent (with the mapping currently edited in the agent in mapping)
                _overWriteMappingOfAgentModel(newModel, agentInMapping->temporaryMapping());
            }
        }
    }
}


/**
 * @brief Slot when the flag "is Mapping Activated" changed
 * @param isMappingActivated
 */
void AgentsMappingController::onIsMappingActivatedChanged(bool isMappingActivated)
{
    if (isMappingActivated)
    {
        // Empty mapping
        if (_isEmptyMapping)
        {
            qDebug() << "Mapping Activated and it is completely EMPTY, we add agents (and links) on our HMI";

            /*if (_modelManager != NULL)
            {
                if (_modelManager->isMappingControlled()) {
                    qDebug() << "Mapping Activated in mode CONTROL";
                }
                else {
                    qDebug() << "Mapping Activated in mode OBSERVE";
                }

                // Get the map from agent name to list of active agents
                QHash<QString, QList<AgentM*>> mapFromAgentNameToActiveAgentsList = _modelManager->getMapFromAgentNameToActiveAgentsList();

                double randomMax = (double)RAND_MAX;

                // Create all agents in mapping
                foreach (QString agentName, mapFromAgentNameToActiveAgentsList.keys())
                {
                    QList<AgentM*> activeAgentsList = mapFromAgentNameToActiveAgentsList.value(agentName);

                    double randomX = (double)qrand() / randomMax;
                    double randomY = (double)qrand() / randomMax;
                    QPointF position = QPointF(randomX * availableMinWidth, randomY * availableMinHeight);
                    //qDebug() << "Random position:" << position << "for agent" << agentName << "(" << randomX << randomY << ")";

                    // Add new model(s) of agent to the current mapping
                    _addAgentModelsToMappingAtPosition(agentName, activeAgentsList, position);
                }

                // Create all links in mapping
                foreach (AgentInMappingVM* agent, _allAgentsInMapping.toList())
                {
                    if ((agent != NULL) && (agent->temporaryMapping() != NULL))
                    {
                        // Delete all "mapping elements" in the temporary mapping
                        agent->temporaryMapping()->mappingElements()->deleteAllItems();

                        foreach (AgentM* model, agent->models()->toList()) {
                            if ((model != NULL) && (model->mapping() != NULL))
                            {
                                foreach (ElementMappingM* mappingElement, model->mapping()->mappingElements()->toList())
                                {
                                    if (mappingElement != NULL)
                                    {
                                        // Add a temporary link for each real link
                                        //agent->addTemporaryLink(mappingElement->input(), mappingElement->outputAgent(), mappingElement->output());

                                        // Simulate slot "on Mapped"
                                        onMapped(mappingElement);
                                    }
                                }
                            }
                        }
                    }
                }

                // Notify the QML to fit the view
                Q_EMIT fitToView();
            }*/
        }
        // Mapping has some agents (and links)
        else
        {
            qDebug() << "Mapping Activated and there are already some agents (and links) on our HMI";

            if (_modelManager != NULL)
            {
                if (_modelManager->isMappingControlled())
                {
                    qDebug() << "Mapping Activated in mode CONTROL";

                    // Apply all temporary mappings
                    foreach (AgentInMappingVM* agent, _allAgentsInMapping.toList())
                    {
                        if ((agent != NULL) && (agent->temporaryMapping() != NULL)) // && (agent->temporaryMapping()->mappingElements()->count() > 0)
                        {
                            // Get the JSON of a mapping
                            QString jsonOfMapping = _modelManager->getJsonOfMapping(agent->temporaryMapping());

                            QString command = QString("LOAD_THIS_MAPPING#%1").arg(jsonOfMapping);

                            // Emit signal "Command asked to agent"
                            Q_EMIT commandAskedToAgent(agent->peerIdsList(), command);
                        }
                    }
                }
                else {
                    qDebug() << "Mapping Activated in mode OBSERVE";
                }
            }
        }
    }
}


/**
 * @brief Slot when the flag "is Mapping Controlled" changed
 * @param isMappingControlled
 */
void AgentsMappingController::onIsMappingControlledChanged(bool isMappingControlled)
{
    qDebug() << "Mapping Ctrl Is Mapping Controlled Changed to" << isMappingControlled;
}


/**
 * @brief Slot when a model of agent will be deleted
 * @param agent
 */
void AgentsMappingController::onAgentModelWillBeDeleted(AgentM* agent)
{
    if (agent != NULL)
    {
        // Get the agent in mapping for the agent name
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agent->name());
        if (agentInMapping != NULL)
        {
            // Remove the model
            agentInMapping->models()->remove(agent);

            // If it was the last one...
            if (agentInMapping->models()->count() == 0)
            {
                // ...delete this agent in mapping
                deleteAgentInMapping(agentInMapping);
            }
        }
    }
}


/**
 * @brief Slot when an active agent has been defined
 * @param agent
 */
void AgentsMappingController::onActiveAgentDefined(AgentM* agent)
{
    if ((agent != NULL) && (_modelManager != NULL))
    {
        QString agentName = agent->name();

        // CONTROL
        if (_modelManager->isMappingControlled())
        {
            //qDebug() << "CONTROL: Model of" << agentName << "is defined. CLEAR its MAPPING !";

            QStringList peerIdsList;
            peerIdsList.append(agent->peerId());

            // Get the agent in mapping for the agent name
            AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);

            // The agent is not in the mapping...
            if (agentInMapping == NULL)
            {
                // Send the command "CLEAR_MAPPING" on the network to this agent
                Q_EMIT commandAskedToAgent(peerIdsList, "CLEAR_MAPPING");
            }
        }
        // OBSERVE
        else
        {
            //qDebug() << "OBSERVE: Model of" << agentName << "is defined. ADD in MAPPING view !" << agent;

            // Get the agent in mapping for the agent name
            AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);

            // The agent is not in the mapping...
            if (agentInMapping == NULL)
            {
                QList<AgentM*> activeAgentsList = QList<AgentM*>();
                activeAgentsList.append(agent);

                double randomMax = (double)RAND_MAX;
                double randomX = (double)qrand() / randomMax;
                double randomY = (double)qrand() / randomMax;
                QPointF position = QPointF(randomX * _viewWidth, randomY * _viewHeight);
                //qDebug() << "Random position:" << position << "for agent" << agentName << "(" << randomX << randomY << ")";

                // Add new model(s) of agent to the current mapping
                _addAgentModelsToMappingAtPosition(agentName, activeAgentsList, position);
            }
            //else {
                // Nothing to do (because the model has already been added to the models list of this "Agent in Mapping VM")
            //}

            // If there are waiting links (where this agent is involved as "Output Agent")
            if (_hashFromAgentNameToListOfWaitingLinks.contains(agentName))
            {
                QList<ElementMappingM*> listOfWaitingLinks = _hashFromAgentNameToListOfWaitingLinks.value(agentName);
                for (ElementMappingM* mappingElement : listOfWaitingLinks)
                {
                    qDebug() << "Create waiting MAP..." << mappingElement->outputAgent() << "." << mappingElement->output() << "-->" << mappingElement->inputAgent() << "." << mappingElement->input();

                    // Create the link corresponding to the mapping element
                    onMapped(mappingElement);
                }
            }
        }
    }
}


/**
 * @brief Slot called when the mapping of an active agent has been defined
 * @param agent
 */
void AgentsMappingController::onActiveAgentMappingDefined(AgentM* agent)
{
    if ((agent != NULL) && (agent->mapping() != NULL))
    {
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agent->name());
        if ((agentInMapping != NULL) && (agentInMapping->temporaryMapping() != NULL))
        {
            QStringList idsOfRemovedMappingElements;
            foreach (QString idPreviousList, agentInMapping->temporaryMapping()->idsOfMappingElements()) {
                if (!agent->mapping()->idsOfMappingElements().contains(idPreviousList)) {
                    idsOfRemovedMappingElements.append(idPreviousList);
                }
            }

            QStringList idsOfAddedMappingElements;
            foreach (QString idNewList, agent->mapping()->idsOfMappingElements()) {
                if (!agentInMapping->temporaryMapping()->idsOfMappingElements().contains(idNewList)) {
                    idsOfAddedMappingElements.append(idNewList);
                }
            }

            // If there are some Removed mapping elements
            if (!idsOfRemovedMappingElements.isEmpty())
            {
                qDebug() << "unmapped" << idsOfRemovedMappingElements;

                foreach (ElementMappingM* mappingElement, agentInMapping->temporaryMapping()->mappingElements()->toList()) {
                    if ((mappingElement != NULL) && idsOfRemovedMappingElements.contains(mappingElement->id()))
                    {
                        onUnmapped(mappingElement);
                    }
                }
            }
            // If there are some Added mapping elements
            if (!idsOfAddedMappingElements.isEmpty())
            {
                qDebug() << "mapped" << idsOfAddedMappingElements;

                foreach (ElementMappingM* mappingElement, agent->mapping()->mappingElements()->toList()) {
                    if ((mappingElement != NULL) && idsOfAddedMappingElements.contains(mappingElement->id()))
                    {
                        onMapped(mappingElement);
                    }
                }
            }
        }
    }
}


/**
 * @brief Slot when two agents are mapped
 * @param mappingElement
 */
void AgentsMappingController::onMapped(ElementMappingM* mappingElement)
{
    if (mappingElement != NULL)
    {
        //qDebug() << "MAPPED" << mappingElement->outputAgent() << "." << mappingElement->output() << "-->" << mappingElement->inputAgent() << "." << mappingElement->input();

        // Try to get the virtual link which corresponds to the mapping element
        MapBetweenIOPVM* link = _getLinkFromMappingElement(mappingElement);

        if (link == NULL)
        {
            AgentInMappingVM* outputAgent = getAgentInMappingFromName(mappingElement->outputAgent());
            AgentInMappingVM* inputAgent = getAgentInMappingFromName(mappingElement->inputAgent());

            if ((outputAgent != NULL) && (inputAgent != NULL))
            {
                OutputVM* output = NULL;
                InputVM* input = NULL;

                // Get the list of view models of output from the output name
                QList<OutputVM*> outputsWithSameName = outputAgent->getOutputsListFromName(mappingElement->output());
                if (outputsWithSameName.count() == 1) {
                    output = outputsWithSameName.first();
                }
                else {
                    qWarning() << "There are" << outputsWithSameName.count() << "outputs with the same name" << mappingElement->output() << "."
                               << "We cannot choose and create the link" << mappingElement->outputAgent() << "." << mappingElement->output() << "-->" << mappingElement->inputAgent() << "." << mappingElement->input();
                }

                // Get the list of view models of input from the input name
                QList<InputVM*> inputsWithSameName = inputAgent->getInputsListFromName(mappingElement->input());
                if (inputsWithSameName.count() == 1) {
                    input = inputsWithSameName.first();
                }
                else {
                    qWarning() << "There are" << inputsWithSameName.count() << "inputs with the same name" << mappingElement->input() << "."
                               << "We cannot choose and create the link" << mappingElement->outputAgent() << "." << mappingElement->output() << "-->" << mappingElement->inputAgent() << "." << mappingElement->input();
                }

                if ((output != NULL) && (input != NULL))
                {
                    qInfo() << "MAPPED" << mappingElement->id();

                    // Create a new map between agents
                    link = new MapBetweenIOPVM(outputAgent, output, inputAgent, input, false, this);

                    // Add to the list
                    _allLinksInMapping.append(link);

                    qDebug() << "MAP has been created:" << mappingElement->outputAgent() << "." << mappingElement->output() << "-->" << mappingElement->inputAgent() << "." << mappingElement->input();

                    // If there is a list of waiting links (where the agent is involved as "Output Agent")
                    if (_hashFromAgentNameToListOfWaitingLinks.contains(mappingElement->outputAgent()))
                    {
                        QList<ElementMappingM*> listOfWaitingLinks = _hashFromAgentNameToListOfWaitingLinks.value(mappingElement->outputAgent());
                        if (listOfWaitingLinks.contains(mappingElement))
                        {
                            // Remove the mapping element from the list because the corresponding link is really created
                            listOfWaitingLinks.removeOne(mappingElement);

                            // Update the hash table
                            _hashFromAgentNameToListOfWaitingLinks.insert(mappingElement->outputAgent(), listOfWaitingLinks);
                        }
                    }
                }
            }
            // Output agent is not yet in the mapping
            else if (inputAgent != NULL) // && (outputAgent == NULL)
            {
                QList<ElementMappingM*> listOfWaitingLinks = QList<ElementMappingM*>();

                if (_hashFromAgentNameToListOfWaitingLinks.contains(mappingElement->outputAgent())) {
                    listOfWaitingLinks = _hashFromAgentNameToListOfWaitingLinks.value(mappingElement->outputAgent());
                }

                // Add the mapping element to the list to create the corresponding link later
                listOfWaitingLinks.append(mappingElement);

                // Update the hash table
                _hashFromAgentNameToListOfWaitingLinks.insert(mappingElement->outputAgent(), listOfWaitingLinks);

                qDebug() << "MAP will be created later:" << mappingElement->outputAgent() << "." << mappingElement->output() << "-->" << mappingElement->inputAgent() << "." << mappingElement->input();
            }
        }
        else {
            // Update the flag if needed
            link->setisVirtual(false);
        }

        if ((link != NULL) && (link->inputAgent() != NULL)) {
            // Add the temporary link that correspond to this real link (if it does not yet exist)
            link->inputAgent()->addTemporaryLink(mappingElement->input(), mappingElement->outputAgent(), mappingElement->output());
        }
    }
}


/**
 * @brief Slot when two agents are unmapped
 * @param mappingElement
 */
void AgentsMappingController::onUnmapped(ElementMappingM* mappingElement)
{
    if (mappingElement != NULL)
    {
        qInfo() << "UN-mapped" << mappingElement->id();

        // Get the view model of link which corresponds to a mapping element
        MapBetweenIOPVM* link = _getLinkFromMappingElement(mappingElement);
        if (link != NULL)
        {
            if (link->inputAgent() != NULL) {
                // Remove the temporary link that correspond to this real link
                link->inputAgent()->removeTemporaryLink(mappingElement->input(), mappingElement->outputAgent(), mappingElement->output());
            }

            // Delete the link between two agents
            _deleteLinkBetweenTwoAgents(link);
        }
    }
}


/**
 * @brief Slot when the list of "Agents in Mapping" changed
 */
void AgentsMappingController::_onAgentsInMappingChanged()
{
    // Update the flag "is Empty Mapping"
    if (_allAgentsInMapping.count() == 0) {
        setisEmptyMapping(true);
    }
    else {
        setisEmptyMapping(false);
    }

    QList<AgentInMappingVM*> newListOfAgentsInMapping = _allAgentsInMapping.toList();

    // Agent in Mapping added
    if (_previousListOfAgentsInMapping.count() < newListOfAgentsInMapping.count())
    {
        //qDebug() << _previousListOfAgentsInMapping.count() << "--> Agent in Mapping ADDED --> " << newListOfAgentsInMapping.count();

        for (AgentInMappingVM* agentInMapping : newListOfAgentsInMapping) {
            if ((agentInMapping != NULL) && !_previousListOfAgentsInMapping.contains(agentInMapping))
            {
                qDebug() << "Agents Mapping Controller: Agent in mapping" << agentInMapping->name() << "ADDED";

                // Emit the signal "Agent in Mapping Added"
                Q_EMIT agentInMappingAdded(agentInMapping);

                // Connect to signals from the new agent in mapping
                connect(agentInMapping, &AgentInMappingVM::inputsListHaveBeenAdded, this, &AgentsMappingController::_onInputsListHaveBeenAdded);
                connect(agentInMapping, &AgentInMappingVM::outputsListHaveBeenAdded, this, &AgentsMappingController::_onOutputsListHaveBeenAdded);
                connect(agentInMapping, &AgentInMappingVM::inputsListWillBeRemoved, this, &AgentsMappingController::_onInputsListWillBeRemoved);
                connect(agentInMapping, &AgentInMappingVM::outputsListWillBeRemoved, this, &AgentsMappingController::_onOutputsListWillBeRemoved);
            }
        }
    }
    // Agent in Mapping removed
    else if (_previousListOfAgentsInMapping.count() > newListOfAgentsInMapping.count())
    {
        //qDebug() << _previousListOfAgentsInMapping.count() << "--> Agent in Mapping REMOVED --> " << newListOfAgentsInMapping.count();

        for (AgentInMappingVM* agentInMapping : _previousListOfAgentsInMapping) {
            if ((agentInMapping != NULL) && !newListOfAgentsInMapping.contains(agentInMapping))
            {
                qDebug() << "Agents Mapping Controller: Agent in mapping" << agentInMapping->name() << "REMOVED";

                // Emit the signal "Agent in Mapping Removed"
                Q_EMIT agentInMappingRemoved(agentInMapping);

                // DIS-connect to signals from the previous agent in mapping
                disconnect(agentInMapping, &AgentInMappingVM::inputsListHaveBeenAdded, this, &AgentsMappingController::_onInputsListHaveBeenAdded);
                disconnect(agentInMapping, &AgentInMappingVM::outputsListHaveBeenAdded, this, &AgentsMappingController::_onOutputsListHaveBeenAdded);
                disconnect(agentInMapping, &AgentInMappingVM::inputsListWillBeRemoved, this, &AgentsMappingController::_onInputsListWillBeRemoved);
                disconnect(agentInMapping, &AgentInMappingVM::outputsListWillBeRemoved, this, &AgentsMappingController::_onOutputsListWillBeRemoved);

            }
        }
    }

    _previousListOfAgentsInMapping = newListOfAgentsInMapping;
}


/**
 * @brief Slot called when some view models of inputs have been added to an agent in mapping
 * @param inputsListHaveBeenAdded
 */
void AgentsMappingController::_onInputsListHaveBeenAdded(QList<InputVM*> inputsListHaveBeenAdded)
{
    // TODO
}


/**
 * @brief Slot called when some view models of outputs have been added to an agent in mapping
 * @param outputsListHaveBeenAdded
 */
void AgentsMappingController::_onOutputsListHaveBeenAdded(QList<OutputVM*> outputsListHaveBeenAdded)
{
    // TODO
}


/**
 * @brief Slot when some view models of inputs will be removed from an agent in mapping
 * @param inputsListWillBeRemoved
 */
void AgentsMappingController::_onInputsListWillBeRemoved(QList<InputVM*> inputsListWillBeRemoved)
{
    AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(sender());
    if ((agentInMapping != NULL) && !inputsListWillBeRemoved.isEmpty())
    {
        //qDebug() << "_on Intputs List Will Be Removed from agent" << agentInMapping->name() << inputsListWillBeRemoved.count();

        for (MapBetweenIOPVM* link : _allLinksInMapping.toList())
        {
            if ((link != NULL) && (link->inputAgent() != NULL) && (link->inputAgent() == agentInMapping)
                    && (link->input() != NULL) && inputsListWillBeRemoved.contains(link->input()))
            {
                inputsListWillBeRemoved.removeOne(link->input());

                // Remove the link between two agents from the mapping
                //removeLinkBetweenTwoAgents(link);

                // Delete the link between two agents
                _deleteLinkBetweenTwoAgents(link);

                if (inputsListWillBeRemoved.isEmpty()) {
                    break;
                }
            }
        }
    }
}


/**
 * @brief Slot when some view models of outputs will be removed from an agent in mapping
 * @param outputsListWillBeRemoved
 */
void AgentsMappingController::_onOutputsListWillBeRemoved(QList<OutputVM*> outputsListWillBeRemoved)
{
    AgentInMappingVM* agentInMapping = qobject_cast<AgentInMappingVM*>(sender());
    if ((agentInMapping != NULL) && !outputsListWillBeRemoved.isEmpty())
    {
        //qDebug() << "_on Outputs List Will Be Removed from agent" << agentInMapping->name() << outputsListWillBeRemoved.count();

        for (MapBetweenIOPVM* link : _allLinksInMapping.toList())
        {
            if ((link != NULL) && (link->outputAgent() != NULL) && (link->outputAgent() == agentInMapping)
                    && (link->output() != NULL) && outputsListWillBeRemoved.contains(link->output()))
            {
                outputsListWillBeRemoved.removeOne(link->output());

                // Remove the link between two agents from the mapping
                //removeLinkBetweenTwoAgents(link);

                // Delete the link between two agents
                _deleteLinkBetweenTwoAgents(link);

                if (outputsListWillBeRemoved.isEmpty()) {
                    break;
                }
            }
        }
    }
}


/**
 * @brief Add new model(s) of agent to the current mapping at a specific position
 * @param agentName
 * @param agentsList
 * @param position
 */
void AgentsMappingController::_addAgentModelsToMappingAtPosition(QString agentName, QList<AgentM*> agentsList, QPointF position)
{
    if (!agentName.isEmpty() && !agentsList.isEmpty())
    {
        AgentInMappingVM* agentInMapping = getAgentInMappingFromName(agentName);

        // The agent is defined, only add models of agent in the internal list of the agentInMappingVM
        if (agentInMapping != NULL)
        {   
            foreach (AgentM* model, agentsList) {
                if (model != NULL) {
                    agentInMapping->models()->append(model);
                }
            }

            qInfo() << "The agent in mapping already exist, new agent models added to" << agentName;
        }
        // Instanciate a new view model of agent in mapping (and add models)
        else
        {
            // Create a new Agent In Mapping
            agentInMapping = new AgentInMappingVM(agentsList, position, this);

            // Add in the map list
            _mapFromNameToAgentInMapping.insert(agentName, agentInMapping);

            // Add this new Agent In Mapping VM in the list for the qml
            _allAgentsInMapping.append(agentInMapping);

            qInfo() << "A new agent mapping has been added:" << agentName;
        }
    }
}


/**
 * @brief Delete a link between two agents
 * @param link
 */
void AgentsMappingController::_deleteLinkBetweenTwoAgents(MapBetweenIOPVM* link)
{
    if (link != NULL)
    {
        // Unselect the link if it is the currently selected one
        if (_selectedLink == link) {
            setselectedLink(NULL);
        }

        // Remove from the list
        _allLinksInMapping.remove(link);

        // Free memory
        delete link;
    }
}


/**
 * @brief Get the view model of link which corresponds to a mapping element
 * @param mappingElement
 * @return
 */
MapBetweenIOPVM* AgentsMappingController::_getLinkFromMappingElement(ElementMappingM* mappingElement)
{
    MapBetweenIOPVM* link = NULL;

    if (mappingElement != NULL)
    {
        foreach (MapBetweenIOPVM* iterator, _allLinksInMapping.toList())
        {
            // FIXME: An agent in mapping can have several Inputs (or Outputs) with the same name but with different types
            // --> Instead, this method must return a list of MapBetweenIOPVM
            if ((iterator != NULL)
                    && (iterator->outputAgent() != NULL) && (iterator->outputAgent()->name() == mappingElement->outputAgent())
                    && (iterator->inputAgent() != NULL) && (iterator->inputAgent()->name() == mappingElement->inputAgent())
                    && (iterator->output() != NULL) && (iterator->output()->name() == mappingElement->output())
                    && (iterator->input() != NULL) && (iterator->input()->name() == mappingElement->input()))
            {
                link = iterator;
                break;
            }
        }
    }

    return link;
}


/**
 * @brief Remove all the links with this agent
 * @param agent
 */
void AgentsMappingController::_removeAllLinksWithAgent(AgentInMappingVM* agent)
{
    if ((agent != NULL) && (_modelManager != NULL))
    {
        foreach (MapBetweenIOPVM* link, _allLinksInMapping.toList())
        {
            if ( (link != NULL) &&
                    ((link->outputAgent() == agent) || (link->inputAgent() == agent)) )
            {
                // Remove a link between two agents from the mapping
                removeLinkBetweenTwoAgents(link);

                // Mapping is activated
                if (_modelManager->isMappingActivated())
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
void AgentsMappingController::_overWriteMappingOfAgentModel(AgentM* agentModel, AgentMappingM* temporaryMapping)
{
    if (agentModel != NULL)
    {
        // Model is ON
        if (agentModel->isON())
        {
            if ((_modelManager != NULL) && (temporaryMapping != NULL))
            {
                QStringList peerIdsList;
                peerIdsList.append(agentModel->peerId());

                // Get the JSON of a mapping
                QString jsonOfMapping = _modelManager->getJsonOfMapping(temporaryMapping);

                QString command = QString("LOAD_THIS_MAPPING#%1").arg(jsonOfMapping);

                // Emit signal "Command asked to agent"
                Q_EMIT commandAskedToAgent(peerIdsList, command);
            }
        }
        // Model is OFF
        else {
            agentModel->setmustOverWriteMapping(true);
        }
    }
}


/**
 * @brief Import the mappings from the json byte content
 * @param byteArrayOfJson
 * @param fromPlatform
 */
void AgentsMappingController::importMappingFromJson(QByteArray byteArrayOfJson, bool fromPlatform)
{
    // Clear the previous mapping
    createNewMapping();

    // Initialize mapping lists from JSON file
    QList< mapping_agent_import_t* > listMappingImported = _jsonHelper->importMapping(byteArrayOfJson, fromPlatform);
    if(listMappingImported.count() > 0)
    {
        QList<ElementMappingM*> mappingElements;
        foreach (mapping_agent_import_t* importedMapping, listMappingImported)
        {
            DefinitionM* definition = importedMapping->definition;
            AgentMappingM* agentMapping = importedMapping->mapping;

            QList<AgentM*> agentModelList = _modelManager->getAgentModelsListFromName(importedMapping->name);
            if(agentModelList.count() == 0)
            {
                AgentM * newAgent = new AgentM(importedMapping->name);
                newAgent->setdefinition(definition);

                agentModelList.append(newAgent);
                Q_EMIT agentCreatedByMapping(newAgent);
            }

            if(agentModelList.count() > 0)
            {
                // Create a new Agent In Mapping
                _addAgentModelsToMappingAtPosition(importedMapping->name, agentModelList, importedMapping->position);

                AgentInMappingVM* agentInMapping = getAgentInMappingFromName(importedMapping->name);
                if(agentInMapping != NULL)
                {
                    // Add the link elements
                    mappingElements.append(agentMapping->mappingElements()->toList());

                    // Set agent mapping
                    if(agentMapping != NULL)
                    {
                        agentInMapping->settemporaryMapping(agentMapping);
                    }
                }
            }
        }

        // Add links
        if (mappingElements.count() > 0)
        {
            // Create all mapping links
            foreach (ElementMappingM* elementMapping, mappingElements)
            {
                onMapped(elementMapping);
            }
        }
    }
}

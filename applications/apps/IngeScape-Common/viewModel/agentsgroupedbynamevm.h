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
 *
 */

#ifndef AGENTSGROUPEDBYNAMEVM_H
#define AGENTSGROUPEDBYNAMEVM_H

#include <QObject>
#include <I2PropertyHelpers.h>
#include <model/agent/agentm.h>
#include <model/publishedvaluem.h>
#include <viewModel/agentsgroupedbydefinitionvm.h>
#include <viewModel/iop/inputvm.h>
#include <viewModel/iop/outputvm.h>
#include <viewModel/iop/parametervm.h>
#include <viewModel/mapping/mappingelementvm.h>


/**
 * @brief The AgentsGroupedByNameVM class defines a view model of group of agents who have the same name
 * Allows to manage several models of agents who have the same name
 */
class AgentsGroupedByNameVM : public QObject
{
    Q_OBJECT

    // Name of our agent(s)
    I2_QML_PROPERTY_READONLY(QString, name)

    // List of models of agents
    I2_QOBJECT_LISTMODEL(AgentM, models)

    // List of peer ids of models
    I2_CPP_NOSIGNAL_PROPERTY(QStringList, peerIdsList)

    // Flag indicating if our agent(s) is ON (vs OFF)
    I2_QML_PROPERTY_READONLY(bool, isON)

    // Flag indicating if our agent can be restarted (by an IngeScape launcher)
    I2_QML_PROPERTY_READONLY(bool, canBeRestarted)

    // Number of agents which are ON
    I2_QML_PROPERTY_READONLY(int, numberOfAgentsON)

    // Number of agents which are OFF
    I2_QML_PROPERTY_READONLY(int, numberOfAgentsOFF)

    // List of view models of inputs
    I2_QOBJECT_LISTMODEL(InputVM, inputsList)

    // List of view models of outputs
    I2_QOBJECT_LISTMODEL(OutputVM, outputsList)

    // List of view models of parameters
    I2_QOBJECT_LISTMODEL(ParameterVM, parametersList)

    // List of all groups (of agents) grouped by definition
    I2_QOBJECT_LISTMODEL(AgentsGroupedByDefinitionVM, allAgentsGroupsByDefinition)

    // List of all view models of mapping elements (which our agent is the input agent)
    I2_QOBJECT_LISTMODEL(MappingElementVM, allMappingElements)

    // Current mapping (real mapping without edition)
    I2_CPP_NOSIGNAL_PROPERTY(AgentMappingM*, currentMapping)

    // Flag indicate if the agent is in the current platform
    I2_QML_PROPERTY(bool, isAgentGroupedByNameInCurrentPlatform)

public:
    /**
     * @brief Constructor
     * @param agentName
     * @param parent
     */
    explicit AgentsGroupedByNameVM(QString agentName,
                                   QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~AgentsGroupedByNameVM();


    /**
     * @brief Clear our agent just before its deletion
     */
    void clearBeforeDeletion();

    /**
     * @brief Ask to start our agent
     */
    Q_INVOKABLE void askStartAgent();


    /**
     * @brief Ask to stop our agent
     */
    Q_INVOKABLE void askStopAgent();

    /**
     * @brief Add a new model of agent
     * @param model
     */
    void addNewAgentModel(AgentM* model);


    /**
     * @brief Remove an old model of agent
     * @param model
     */
    void removeOldAgentModel(AgentM* model);


    /**
     * @brief Update the current value of an I/O/P of our agent(s)
     * @param publishedValue
     */
    void updateCurrentValueOfIOP(PublishedValueM* publishedValue);


    /**
     * @brief Delete the view model of agents grouped by definition
     * And emit the signal "agentModelHasToBeDeleted" for each of its model of agent
     * @param agentsGroupedByDefinition
     */
    void deleteAgentsGroupedByDefinition(AgentsGroupedByDefinitionVM* agentsGroupedByDefinition);


    /**
     * @brief Remove a model of agent from its host
     * @param model
     */
    void removeAgentModelFromHost(AgentM* model);


    /**
     * @brief Delete agents OFF
     */
    Q_INVOKABLE void deleteAgentsOFF();


    /**
     * @brief Get the list of definitions with a specific name
     * @param definitionName
     * @return
     */
    QList<DefinitionM*> getDefinitionsWithName(QString definitionName);


    /**
     * @brief Open the definition(s)
     * If there are several "Agents Grouped by Definition", we have to open each definition
     */
    Q_INVOKABLE void openDefinition();


    /**
     * @brief Return the list of view models of input from an input name
     * @param inputName
     */
    QList<InputVM*> getInputsListFromName(QString inputName);


    /**
     * @brief Return the view model of input from an input id
     * @param inputId
     */
    InputVM* getInputFromId(QString inputId);


    /**
     * @brief Return the list of view models of output from an output name
     * @param outputName
     */
    QList<OutputVM*> getOutputsListFromName(QString outputName);


    /**
     * @brief Return the view model of output from an output id
     * @param outputId
     */
    OutputVM* getOutputFromId(QString outputId);


    /**
     * @brief Return the list of view models of parameter from a parameter name
     * @param parameterName
     */
    QList<ParameterVM*> getParametersListFromName(QString parameterName);


    /**
     * @brief Return the view model of parameter from a parameter id
     * @param parameterId
     */
    ParameterVM* getParameterFromId(QString parameterId);


    /**
     * @brief Return the view model of mapping element from a name
     * @param name
     * @return
     */
    MappingElementVM* getMappingElementVMfromName(QString name);


Q_SIGNALS:

    /**
     * @brief Signal emitted when our view model has become useless (no more model)
     */
    //void noMoreModelAndUseless();


    /**
     * @brief Signal emitted when our view model has become useless (no more agents grouped by definition)
     */
    void noMoreAgentsGroupedByDefinitionAndUseless();


    /**
     * @brief Signal emitted when a new view model of agents grouped by definition has been created
     * @param agentsGroupedByDefinition
     */
    void agentsGroupedByDefinitionHasBeenCreated(AgentsGroupedByDefinitionVM* agentsGroupedByDefinition);


    /**
     * @brief Signal emitted when a view model of agents grouped by definition will be deleted
     * @param agentsGroupedByDefinition
     */
    void agentsGroupedByDefinitionWillBeDeleted(AgentsGroupedByDefinitionVM* agentsGroupedByDefinition);


    /**
     * @brief Signal emitted when a model of agent "ON" has been added to our group
     * @param model
     */
    void agentModelONhasBeenAdded(AgentM* model);


    /**
     * @brief Signal emitted when a model of agent has to be deleted
     * @param model
     */
    void agentModelHasToBeDeleted(AgentM* model);


    /**
     * @brief Signal emitted when the definition(s) of our agent must be opened
     * @param definitionsList
     */
    void definitionsToOpen(QList<DefinitionM*> definitionsList);


    /**
     * @brief Signal emitted when some view models of inputs have been added to our agent(s grouped by name)
     * @param newInputs
     */
    void inputsHaveBeenAdded(QList<InputVM*> newInputs);


    /**
     * @brief Signal emitted when some view models of outputs have been added to our agent(s grouped by name)
     * @param newOutputs
     */
    void outputsHaveBeenAdded(QList<OutputVM*> newOutputs);


    /**
     * @brief Signal emitted when some view models of parameters have been added to our agent(s grouped by name)
     * @param newParameters
     */
    void parametersHaveBeenAdded(QList<ParameterVM*> newParameters);


    /**
     * @brief Signal emitted when some view models of inputs will be removed from our agent(s grouped by name)
     * @param oldInputs
     */
    void inputsWillBeRemoved(QList<InputVM*> oldInputs);


    /**
     * @brief Signal emitted when some view models of outputs will be removed from our agent(s grouped by name)
     * @param oldOutputs
     */
    void outputsWillBeRemoved(QList<OutputVM*> oldOutputs);


    /**
     * @brief Signal emitted when some view models of parameters will be removed from our agent(s grouped by name)
     * @param oldParameters
     */
    void parametersWillBeRemoved(QList<ParameterVM*> oldParameters);


    /**
     * @brief Signal emitted when some view models of mapping elements have been added to our agent(s grouped by name)
     * @param newMappingElements
     */
    void mappingElementsHaveBeenAdded(QList<MappingElementVM*> newMappingElements);


    /**
     * @brief Signal emitted when some view models of mapping elements will be removed from our agent(s grouped by name)
     * @param oldMappingElements
     */
    void mappingElementsWillBeRemoved(QList<MappingElementVM*> oldMappingElements);


public Q_SLOTS:


private Q_SLOTS:

    /**
     * @brief Slot called when the flag "is ON" of a model changed
     * @param isON
     */
    void _onIsONofModelChanged(bool isON);


    /**
     * @brief Slot called when the definition of a model changed (with previous and new values)
     * @param previousDefinition
     * @param newDefinition
     */
    void _onDefinitionOfModelChangedWithPreviousAndNewValues(DefinitionM* previousDefinition, DefinitionM* newDefinition);


    /**
     * @brief Slot called when the mapping of a model changed (with previous and new values)
     * @param previousMapping
     * @param newMapping
     */
    void _onMappingOfModelChangedWithPreviousAndNewValues(AgentMappingM* previousMapping, AgentMappingM* newMapping);


    /**
     * @brief Slot called when a view model of agents grouped by definition has become useless (no more model)
     */
    void _onUselessAgentsGroupedByDefinition();

    /**
     * @brief Slot called when the flag "can Be Restarted" of a model changed
     * @param canBeRestarted
     */
    void _onCanBeRestartedOfModelChanged(bool canBeRestarted);


private:

    /**
     * @brief Update with all models of agents
     */
    void _updateWithAllModels();


    /**
     * @brief Manage an agent model which is just defined (its definition evolve from NULL to a defined one)
     * @param model
     */
    void _manageJustDefinedAgent(AgentM* model);


    /**
     * @brief Check if we have to merge the model of agent with an existing view model of agents (grouped by definition) that have the same definition
     * @param model
     */
    void _checkHaveToMergeAgent(AgentM* model);


    /**
     * @brief Manage a new model of agent ON on a host for a group of agents with the same definition (can be NULL)
     * @param model
     * @param groupOfAgentsWithSameDefinition
     */
    void _manageNewModelOnHostForGroup(AgentM* model, AgentsGroupedByDefinitionVM* groupOfAgentsWithSameDefinition);


    /**
     * @brief Create a new view model of agents grouped by definition
     * @param definition
     * @param model
     */
    void _createAgentsGroupedByDefinition(DefinitionM* definition, AgentM* model);


    /**
     * @brief Manage the list of inputs of the added definition
     * @param definition
     */
    void _manageInputsOfAddedDefinition(DefinitionM* definition);


    /**
     * @brief Manage the list of outputs of the added definition
     * @param definition
     */
    void _manageOutputsOfAddedDefinition(DefinitionM* definition);


    /**
     * @brief Manage the list of parameters of the added definition
     * @param definition
     */
    void _manageParametersOfAddedDefinition(DefinitionM* definition);


    /**
     * @brief Manage the list of inputs of the removed definition
     * @param definition
     */
    void _manageInputsOfRemovedDefinition(DefinitionM* definition);


    /**
     * @brief Manage the list of outputs of the removed definition
     * @param definition
     */
    void _manageOutputsOfRemovedDefinition(DefinitionM* definition);


    /**
     * @brief Manage the list of parameters of the removed definition
     * @param definition
     */
    void _manageParametersOfRemovedDefinition(DefinitionM* definition);


    /**
     * @brief Update the flag "Is Defined in All Definitions" for each Input/Output/Parameter
     * @param numberOfDefinitions
     */
    void _updateIsDefinedInAllDefinitionsForEachIOP(int numberOfDefinitions);


    /**
     * @brief Manage a new model of input
     * @param input
     * @return Pair<haveToAdd, InputVM> flag indicating if we have to add the view model of input
     */
    QPair<bool, InputVM*> _manageNewInputModel(AgentIOPM* input);


    /**
     * @brief Manage a new model of output
     * @param output
     * @return Pair<haveToAdd, OutputVM> flag indicating if we have to add the view model of output
     */
    QPair<bool, OutputVM*> _manageNewOutputModel(OutputM* output);


    /**
     * @brief Manage a new model of parameter
     * @param parameter
     * @return Pair<haveToAdd, ParameterVM> flag indicating if we have to add the view model of parameter
     */
    QPair<bool, ParameterVM*> _manageNewParameterModel(AgentIOPM* parameter);


    /**
     * @brief Manage an old model of input (just before being deleted)
     * @param input
     * @return Pair<haveToRemove, InputVM> flag indicating if we have to remove the view model of input
     */
    QPair<bool, InputVM*> _manageOldInputModel(AgentIOPM* input);


    /**
     * @brief Manage an old model of output (just before being deleted)
     * @param output
     * @return Pair<haveToRemove, OutputVM> flag indicating if we have to remove the view model of output
     */
    QPair<bool, OutputVM*> _manageOldOutputModel(OutputM* output);


    /**
     * @brief Manage an old model of parameter (just before being deleted)
     * @param parameter
     * @return Pair<haveToRemove, ParameterVM> flag indicating if we have to remove the view model of parameter
     */
    QPair<bool, ParameterVM*> _manageOldParameterModel(AgentIOPM* parameter);


    /**
     * @brief Manage a new model of mapping element
     * @param mappingElement
     * @return Pair<haveToAdd, MappingElementVM> flag indicating if we have to add the view model of mapping element
     */
    QPair<bool, MappingElementVM*> _manageNewMappingElementModel(MappingElementM* mappingElement);


    /**
     * @brief Manage an old model of mapping element (just before being deleted)
     * @param mappingElement
     * @return Pair<haveToRemove, MappingElementVM> flag indicating if we have to remove the view model of mapping element
     */
    QPair<bool, MappingElementVM*> _manageOldMappingElementModel(MappingElementM* mappingElement);


private:

    // Flag indicating if our "agents grouped by name" can be deleted when its list of all agents grouped by definition is empty
    // The list "allAgentsGroupsByDefinition" can be TEMPORARY empty during the execution of "_onDefinitionOfModelChangedWithPreviousAndNewValues"
    bool _canBeDeleted_whenListOfAllAgentsGroupsByDefinition_isEmpty;

    // View model of agents grouped by definition NULL
    AgentsGroupedByDefinitionVM* _agentsGroupedByDefinitionNULL;

    // Hash table from a definition to a (view model of) agents grouped by definition
    QHash<DefinitionM*, AgentsGroupedByDefinitionVM*> _hashFromDefinitionToAgentsGroupedByDefinition;

    // Input name as key is not unique (value type can be different)
    // Hash table from an input name to a list of view models of inputs
    QHash<QString, QList<InputVM*>> _hashFromNameToInputsList;

    // Hash table from a (unique) input id to a view model of input
    QHash<QString, InputVM*> _hashFromIdToInput;

    // Output name as key is not unique (value type can be different)
    // Hash table from an output name to a list of view models of outputs
    QHash<QString, QList<OutputVM*>> _hashFromNameToOutputsList;

    // Hash table from a (unique) output id to a view model of output
    QHash<QString, OutputVM*> _hashFromIdToOutput;

    // Parameter name as key is not unique (value type can be different)
    // Hash table from a parameter name to a list of view models of parameters
    QHash<QString, QList<ParameterVM*>> _hashFromNameToParametersList;

    // Hash table from a (unique) parameter id to a view model of parameter
    QHash<QString, ParameterVM*> _hashFromIdToParameter;

    // Hash table from a name to a view model of mapping element
    QHash<QString, MappingElementVM*> _hashFromNameToMappingElement;

};

QML_DECLARE_TYPE(AgentsGroupedByNameVM)

#endif // AGENTSGROUPEDBYNAMEVM_H

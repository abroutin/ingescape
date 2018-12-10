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

#ifndef AGENTINMAPPINGVM_H
#define AGENTINMAPPINGVM_H

#include <QObject>
#include <QtQml>

#include <I2PropertyHelpers.h>

#include <viewModel/agentsgroupedbynamevm.h>
#include <viewModel/link/linkinputvm.h>
#include <viewModel/link/linkoutputvm.h>


/**
 * @brief The AgentInMappingVM class a view model of agent in the global mapping
 * Allows to manage agents and links graphically
 */
class AgentInMappingVM : public QObject
{
    Q_OBJECT

    // Name of our agent
    I2_QML_PROPERTY_READONLY(QString, name)

    // Agents grouped by name
    I2_QML_PROPERTY_READONLY(AgentsGroupedByNameVM*, agentsGroupedByName)

    // List of view models of link inputs
    I2_QOBJECT_LISTMODEL(LinkInputVM, linkInputsList)

    // List of view models of link outputs
    I2_QOBJECT_LISTMODEL(LinkOutputVM, linkOutputsList)

    // The position corresponds to the corner Top-Left of the box
    I2_QML_PROPERTY(QPointF, position)

    // Flag indicating if our agent is reduced
    I2_QML_PROPERTY(bool, isReduced)

    // Flag indicating if our agent is locked reduced (prevent to open the list of Inputs/Outputs)
    I2_QML_PROPERTY_READONLY(bool, isLockedReduced)

    // Group of value type of the reduced link inputs of our agent (= brin)
    I2_QML_PROPERTY_READONLY(AgentIOPValueTypeGroups::Value, reducedLinkInputsValueTypeGroup)

    // Group of value type of the reduced link outputs of our agent (= brin)
    I2_QML_PROPERTY_READONLY(AgentIOPValueTypeGroups::Value, reducedLinkOutputsValueTypeGroup)


    // FIXME TO REMOVE ?
    // Mapping currently edited (temporary until the user activate the mapping)
    //I2_CPP_NOSIGNAL_PROPERTY(AgentMappingM*, temporaryMapping)

    // List of (view models of) added mapping elements while the mapping was UN-activated
    //I2_QOBJECT_LISTMODEL(MappingElementVM, addedLinks_WhileMappingWasUNactivated)

    // List of (view models of) removed mapping elements while the mapping was UN-activated
    //I2_QOBJECT_LISTMODEL(MappingElementVM, removedLinks_WhileMappingWasUNactivated)

    // Flag indicating if our agent had links added while the mapping was UN-activated
    //I2_CPP_NOSIGNAL_PROPERTY(bool, hadLinksAdded_WhileMappingWasUNactivated)
    I2_QML_PROPERTY_READONLY(bool, hadLinksAdded_WhileMappingWasUNactivated)

    // Flag indicating if our agent had links removed while the mapping was UN-activated
    //I2_CPP_NOSIGNAL_PROPERTY(bool, hadLinksRemoved_WhileMappingWasUNactivated)
    I2_QML_PROPERTY_READONLY(bool, hadLinksRemoved_WhileMappingWasUNactivated)


public:
    /**
     * @brief Constructor
     * @param agentsGroupedByName Models of agents grouped by the same name
     * @param position Position of the top left corner
     * @param parent
     */
    explicit AgentInMappingVM(AgentsGroupedByNameVM* agentsGroupedByName,
                              QPointF position,
                              QObject* parent = nullptr);


    /**
     * @brief Destructor
     */
    ~AgentInMappingVM();


    /**
     * @brief Return the list of view models of link input from an input name
     * @param inputName
     */
    QList<LinkInputVM*> getLinkInputsListFromName(QString inputName);


    /**
     * @brief Return the view model of link input from an input id
     * @param inputId
     */
    LinkInputVM* getLinkInputFromId(QString inputId);


    /**
     * @brief Return the list of view models of link output from an output name
     * @param outputName
     */
    QList<LinkOutputVM*> getLinkOutputsListFromName(QString outputName);


    /**
     * @brief Return the view model of link output from an output id
     * @param outputId
     */
    LinkOutputVM* getLinkOutputFromId(QString outputId);


    /**
     * @brief FIXME REMOVE Add a temporary mapping element
     * This temporary mapping element will become a real link:
     * - when the user will activate the mapping
     * - or when our agent will evolve from OFF to ON
     * @param linkId
     * @param inputId
     * @param outputAgentName
     * @param outputId
     */
    //void addTemporaryMappingElement(QString linkId, QString inputId, QString outputAgentName, QString outputId);


    /**
     * @brief FIXME REMOVE Remove a temporary mapping element
     * This temporary link mapping element became a real link:
     * - when the user activated the mapping
     * - or when our agent evolved from OFF to ON
     * @param linkId
     */
    //void removeTemporaryMappingElement(QString linkId);

    void addLink_WhileMappingWasUNactivated(QString linkId, QString inputId, QString outputAgentName, QString outputId);

    void cancelAddLink_WhileMappingWasUNactivated(QString linkId);
    void cancelAllAddedLinks_WhileMappingWasUNactivated();

    void removeLink_WhileMappingWasUNactivated(QString linkId, MappingElementVM* mappingElement);

    void cancelRemoveLink_WhileMappingWasUNactivated(QString linkId);
    void cancelAllRemovedLinks_WhileMappingWasUNactivated();


    /**
     * @brief Get the view model of added mapping element (while the mapping was UN-activated) from a link id
     * @param linkId
     * @return
     */
    //MappingElementVM* getAddedMappingElementFromLinkId_WhileMappingWasUNactivated(QString linkId);

    /**
     * @brief Get the model of added mapping element (while the mapping was UN-activated) from a link id
     * @param linkId
     * @return
     */
    ElementMappingM* getAddedMappingElementFromLinkId_WhileMappingWasUNactivated(QString linkId);


    /**
     * @brief Get the view model of removed mapping element (while the mapping was UN-activated) from a link id
     * @param linkId
     * @return
     */
    MappingElementVM* getRemovedMappingElementFromLinkId_WhileMappingWasUNactivated(QString linkId);


    QList<QString> getAllAddedLinkIds_WhileMappingWasUNactivated();
    QList<ElementMappingM*> getAllAddedMappingElements_WhileMappingWasUNactivated();

    QList<QString> getAllRemovedLinkIds_WhileMappingWasUNactivated();
    QList<MappingElementVM*> getAllRemovedMappingElements_WhileMappingWasUNactivated();


Q_SIGNALS:

    /**
     * @brief Signal emitted when some view models of link inputs have been added to our agent in mapping
     * @param addedlinkInputs
     */
    void linkInputsListHaveBeenAdded(QList<LinkInputVM*> addedlinkInputs);


    /**
     * @brief Signal emitted when some view models of link outputs have been added to our agent in mapping
     * @param addedlinkOutputs
     */
    void linkOutputsListHaveBeenAdded(QList<LinkOutputVM*> addedlinkOutputs);


    /**
     * @brief Signal emitted when some view models of link inputs will be removed from our agent in mapping
     * @param removedLinkInputs
     */
    void linkInputsListWillBeRemoved(QList<LinkInputVM*> removedLinkInputs);


    /**
     * @brief Signal emitted when some view models of link outputs will be removed from our agent in mapping
     * @param removedLinkOutputs
     */
    void linkOutputsListWillBeRemoved(QList<LinkOutputVM*> removedLinkOutputs);


private Q_SLOTS:

    /**
     * @brief Slot called when some view models of inputs have been added to the agent(s grouped by name)
     * @param newInputs
     */
    void _onInputsHaveBeenAdded(QList<InputVM*> newInputs);


    /**
     * @brief Slot called when some view models of outputs have been added to the agent(s grouped by name)
     * @param newOutputs
     */
    void _onOutputsHaveBeenAdded(QList<OutputVM*> newOutputs);


    /**
     * @brief Slot called when some view models of inputs will be removed from the agent(s grouped by name)
     * @param oldInputs
     */
    void _onInputsWillBeRemoved(QList<InputVM*> oldInputs);


    /**
     * @brief Slot called when some view models of outputs will be removed from the agent(s grouped by name)
     * @param oldOutputs
     */
    void _onOutputsWillBeRemoved(QList<OutputVM*> oldOutputs);


    /**
     * @brief Slot when the list of models changed
     */
    //void _onModelsChanged();


    /**
     * @brief Slot called when the definition of a model changed (with previous and new values)
     * @param previousValue
     * @param newValue
     */
    //void _onDefinitionOfModelChangedWithPreviousAndNewValues(DefinitionM* previousValue, DefinitionM* newValue);


private:

    /**
     * @brief A model of agent has been added to our list
     * @param model
     */
    //void _agentModelAdded(AgentM* model);


    /**
     * @brief A model of agent has been removed from our list
     * @param model
     */
    //void _agentModelRemoved(AgentM* model);


    /**
     * @brief A model of input has been added
     * @param input
     * @return
     */
    //InputVM* _inputModelAdded(AgentIOPM* input);


    /**
     * @brief A model of input has been removed
     * @param input
     * @return
     */
    //InputVM* _inputModelRemoved(AgentIOPM* input);


    /**
     * @brief A model of output has been added
     * @param output
     * @return
     */
    //OutputVM* _outputModelAdded(OutputM* output);


    /**
     * @brief A model of output has been removed
     * @param output
     * @return
     */
    //OutputVM* _outputModelRemoved(OutputM* output);



    /**
     * @brief A model of parameter has been added
     * @param parameter
     * @return
     */
    //ParameterVM* _parameterModelAdded(AgentIOPM* parameter);


    /**
     * @brief A model of parameter has been removed
     * @param parameter
     * @return
     */
    //ParameterVM* _parameterModelRemoved(AgentIOPM* parameter);


    /**
     * @brief Update the group (of value type) of the reduced link inputs of our agent (= brin)
     */
    void _updateReducedLinkInputsValueTypeGroup();


    /**
     * @brief Update the group (of value type) of the reduced link outputs of our agent (= brin)
     */
    void _updateReducedLinkOutputsValueTypeGroup();


    /**
     * @brief Check the maximum number of Inputs/Outputs
     */
    void _checkMaxNumberOfIO();


private:

    // Input name as key is not unique (value type can be different)
    // Hash table from an input name to a list of view models of link inputs
    QHash<QString, QList<LinkInputVM*>> _hashFromNameToLinkInputsList;

    // Hash table from a (unique) input id to a view model of link input
    QHash<QString, LinkInputVM*> _hashFromIdToLinkInput;

    // Output name as key is not unique (value type can be different)
    // Hash table from an output name to a list of view models of link outputs
    QHash<QString, QList<LinkOutputVM*>> _hashFromNameToLinkOutputsList;

    // Hash table from a (unique) output id to a view model of link output
    QHash<QString, LinkOutputVM*> _hashFromIdToLinkOutput;


    // Hash table from "(unique) link id" to added link (model of mapping element) while the mapping was UN-activated
    QHash<QString, ElementMappingM*> _hashFromLinkIdToAddedMappingElement_WhileMappingWasUNactivated;

    // Hash table from "(unique) link id" to removed link (view model of mapping element) while the mapping was UN-activated
    QHash<QString, MappingElementVM*> _hashFromLinkIdToRemovedMappingElement_WhileMappingWasUNactivated;

};

QML_DECLARE_TYPE(AgentInMappingVM)

#endif // AGENTINMAPPINGVM_H

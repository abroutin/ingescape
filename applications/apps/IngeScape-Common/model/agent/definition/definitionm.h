/*
 *	IngeScape Common
 *
 *  Copyright © 2017-2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

#ifndef DEFINITIONM_H
#define DEFINITIONM_H

#include <QObject>
#include <QtQml>

#include <I2PropertyHelpers.h>

#include <model/agent/definition/agentiopm.h>
#include <model/agent/definition/outputm.h>
#include <model/agent/definition/callm.h>


/**
 * @brief The DefinitionM class defines a model of definition for an agent
 */
class DefinitionM : public QObject
{
    Q_OBJECT

    // Name of our agent definition
    I2_QML_PROPERTY(QString, name)

    // Version of our agent definition
    I2_QML_PROPERTY(QString, version)

    // Description of our agent definition
    I2_QML_PROPERTY(QString, description)

    // Flag indicating if our definition is a variant (same name, same version but I/O/P differents)
    I2_QML_PROPERTY(bool, isVariant)

    // List of inputs of our agent definition
    I2_QOBJECT_LISTMODEL(AgentIOPM, inputsList)

    // List of outputs of our agent definition
    I2_QOBJECT_LISTMODEL(OutputM, outputsList)

    // List of parameters of our agent definition
    I2_QOBJECT_LISTMODEL(AgentIOPM, parametersList)

    // List of calls of our agent definition
    I2_QOBJECT_LISTMODEL(CallM, callsList)

    // List of ids of inputs
    I2_CPP_NOSIGNAL_PROPERTY(QStringList, inputsIdsList)

    // List of ids of outputs
    I2_CPP_NOSIGNAL_PROPERTY(QStringList, outputsIdsList)

    // List of ids of parameters
    I2_CPP_NOSIGNAL_PROPERTY(QStringList, parametersIdsList)


public:
    /**
     * @brief Constructor
     * @param name
     * @param version
     * @param description
     * @param parent
     */
    explicit DefinitionM(QString name, QString version, QString description, QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~DefinitionM();


    /**
     * @brief Set the flag "is Muted Output" of an output of our agent definition
     * @param isMuted
     * @param outputName
     */
    void setisMutedOutput(bool isMuted, QString outputName);


    /**
     * @brief Get the flag "is Muted Output" of an output of our agent definition
     * @param outputName
     */
    bool getIsMutedOutput(QString outputName);


    /**
     * @brief Set the flag "is Muted" of all Outputs of our agent definition
     * @param isMuted
     */
    void setisMutedOfAllOutputs(bool isMuted);


    /**
     * @brief Get an Input with its name
     * @param inputName
     * @return
     */
    AgentIOPM* getInputWithName(QString inputName);


    /**
     * @brief Get an Output with its name
     * @param outputName
     * @return
     */
    OutputM* getOutputWithName(QString outputName);


    /**
     * @brief Get a Parameter with its name
     * @param parameterName
     * @return
     */
    AgentIOPM* getParameterWithName(QString parameterName);


    /**
     * @brief Accessor for the list of calls.
     * The returned value is a copy of the calls list cannot be modified from here.
     * @return
     */
    QList<CallM*> getCallsList() const;


    /**
     * @brief Update the current value of an I/O/P of our agent definition
     * @param iopType
     * @param iopName
     * @param value
     */
    void updateCurrentValueOfIOP(AgentIOPTypes::Value iopType, QString iopName, QVariant value);


    /**
     * @brief Make a copy of our definition
     * @return
     */
    DefinitionM* copy();


Q_SIGNALS:

    /**
     * @brief Signal emitted when a command must be sent on the network to an agent about one of its output
     * @param command
     * @param outputName
     */
    void commandAskedForOutput(QString command, QString outputName);


    /**
     * @brief Signal emitted when the flag "is Muted Output" of an output changed
     * @param isMutedOutput
     * @param outputName
     */
    void isMutedOutputChanged(bool isMutedOutput, QString outputName);


    /**
     * @brief Signal emitted when we have to open the values history of our agent definition
     */
    void openValuesHistoryOfAgent();


    /**
     * @brief Signal emitted when the corresponding window must be bring to front (in the windowing system)
     */
    void bringToFront();


public Q_SLOTS:


private Q_SLOTS:

    /**
     * @brief Slot when the list of inputs changed
     */
    void _onInputsListChanged();


    /**
     * @brief Slot when the list of outputs changed
     */
    void _onOutputsListChanged();


    /**
     * @brief Slot when the list of parameters changed
     */
    void _onParametersListChanged();


    /**
     * @brief Slot called when the flag "is Muted Output" of an output changed
     * @param isMutedOutput
     */
    void _onIsMutedOutputChanged(bool isMutedOutput);


private:
    // Previous list of inputs
    QList<AgentIOPM*> _previousInputsList;

    // Previous list of outputs
    QList<OutputM*> _previousOutputsList;

    // Previous list of parameters
    QList<AgentIOPM*> _previousParametersList;

    // Previous list of calls
    QList<CallM*> _previousCallsList;

    // Map from an input name to a model of input
    QHash<QString, AgentIOPM*> _mapFromInputNameToInput;

    // Map from an output name to a model of output
    QHash<QString, OutputM*> _mapFromOutputNameToOutput;

    // Map from a parameter name to a model of parameter
    QHash<QString, AgentIOPM*> _mapFromParameterNameToParameter;

};

/**
 * @brief Equality operator to compare two definition
 * @param left
 * @param right
 * @return
 */
bool operator==(const DefinitionM& left, const DefinitionM& right);

/**
 * @brief Difference operator to compare two definition
 * Simply the negation of the equality operator
 * @param left
 * @param right
 * @return
 */
bool operator!=(const DefinitionM& left, const DefinitionM& right);

QML_DECLARE_TYPE(DefinitionM)

#endif // DEFINITIONM_H
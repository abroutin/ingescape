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

#ifndef AGENTVM_H
#define AGENTVM_H

#include <QObject>
#include <QtQml>
#include <QColor>

#include <I2PropertyHelpers.h>

#include "model/agentm.h"
#include "model/definitionm.h"


/**
 * @brief The AgentVM class defines a view model of agent in the list
 * Allows to manage when several agents have exactly the same name and the same definition
 * Only Peer ID is different (and HostName can also be different)
 */
class AgentVM : public QObject
{
    Q_OBJECT

    // Name of our agent
    I2_QML_PROPERTY_CUSTOM_SETTER(QString, name)

    // List of models of agents
    I2_QOBJECT_LISTMODEL(AgentM, models)

    // Hostname(s) on the network of our agent(s)
    I2_QML_PROPERTY_READONLY(QString, hostnames)

    // Flag indicating if our agent never yet appeared on the network
    I2_QML_PROPERTY_READONLY(bool, neverAppearedOnNetwork)

    // Flag indicating if our agent is ON (vs OFF)
    I2_QML_PROPERTY_READONLY(bool, isON)

    // Flag indicating if our agent is muted
    I2_QML_PROPERTY_READONLY(bool, isMuted)

    // Flag indicating if our agent can be frozen
    I2_QML_PROPERTY_READONLY(bool, canBeFrozen)

    // Flag indicating if our agent is frozen
    I2_QML_PROPERTY_READONLY(bool, isFrozen)

    // Definition of our agent
    I2_QML_PROPERTY_READONLY_CUSTOM_SETTER(DefinitionM*, definition)

    // Number of clones
    I2_QML_PROPERTY_READONLY(int, clonesNumber)

    // Flag indicating if our agent can be restarted (by a INGESCAPE launcher)
    I2_QML_PROPERTY_READONLY(bool, canBeRestarted)

    // State of our agent
    I2_QML_PROPERTY(QString, state)

    // Flag indicating if our agent has its log in a stream
    I2_QML_PROPERTY_READONLY(bool, hasLogInStream)

    // Flag indicating if our agent has its log in a file
    I2_QML_PROPERTY_READONLY(bool, hasLogInFile)

    // Path of the log file of our agent
    I2_QML_PROPERTY_READONLY(QString, logFilePath)

    // Path of the definition file of our agent
    I2_QML_PROPERTY_READONLY(QString, definitionFilePath)

    // Path of the mapping file of our agent
    I2_QML_PROPERTY_READONLY(QString, mappingFilePath)

    // Flag indicating if the option "View Log Stream" is enabled
    I2_QML_PROPERTY_READONLY(bool, isEnabledViewLogStream)


public:
    /**
     * @brief Default constructor
     * @param model
     * @param parent
     */
    explicit AgentVM(AgentM* model, QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~AgentVM();


    /**
     * @brief Change the state of our agent
     */
    Q_INVOKABLE void changeState();


    /**
     * @brief Mute / UN-mute all outputs of our agent
     */
    Q_INVOKABLE void changeMuteAllOutputs();


    /**
     * @brief Freeze / UN-freeze our agent
     */
    Q_INVOKABLE void changeFreeze();


    /**
     * @brief Load a (new) definition
     */
    Q_INVOKABLE void loadDefinition();


    /**
     * @brief Load a (new) mapping
     */
    Q_INVOKABLE void loadMapping();


    /**
     * @brief Download the current definition
     */
    Q_INVOKABLE void downloadDefinition();


    /**
     * @brief Download the current mapping
     */
    Q_INVOKABLE void downloadMapping();


    /**
     * @brief Change the flag "(has) Log in Stream"
     */
    Q_INVOKABLE void changeLogInStream();


    /**
     * @brief Change the flag "(has) Log in File"
     */
    Q_INVOKABLE void changeLogInFile();


    /**
     * @brief Save the "files paths" of our agent
     * @param definitionFilePath
     * @param mappingFilePath
     * @param logFilePath
     */
    Q_INVOKABLE void saveFilesPaths(QString definitionFilePath, QString mappingFilePath, QString logFilePath);


    /**
     * @brief Save the definition of our agent to its path
     */
    Q_INVOKABLE void saveDefinitionToPath();


    /**
     * @brief Save the mapping of our agent to its path
     */
    Q_INVOKABLE void saveMappingToPath();


    /**
     * @brief Open the "Log Stream" of our agent
     */
    Q_INVOKABLE void openLogStream();


Q_SIGNALS:

    /**
     * @brief Signal emitted when the definition changed (with previous and new values)
     * @param previousValue
     * @param newValue
     */
    void definitionChangedWithPreviousAndNewValues(DefinitionM* previousValue, DefinitionM* newValue);


    /**
     * @brief Signal emitted when a command must be sent on the network to a launcher
     * @param command
     * @param hostname
     * @param commandLine
     */
    void commandAskedToLauncher(QString command, QString hostname, QString commandLine);


    /**
     * @brief Signal emitted when a command must be sent on the network to an agent
     * @param peerIdsList
     * @param command
     */
    void commandAskedToAgent(QStringList peerIdsList, QString command);


    /**
     * @brief Signal emitted when a command must be sent on the network to an agent about one of its output
     * @param peerIdsList
     * @param command
     * @param outputName
     */
    void commandAskedToAgentAboutOutput(QStringList peerIdsList, QString command, QString outputName);


    /**
     * @brief Signal emitted when we have to open the values history of our agent
     * @param agentName
     */
    void openValuesHistoryOfAgent(QString agentName);


    /**
     * @brief Signal emitted when a different definition is detected on a model of agent
     * (compared to the definition of our view model)
     * @param agent
     */
    void differentDefinitionDetectedOnModelOfAgent(AgentM* agent);


    /**
     * @brief Signal emitted when we have to load an agent definition from a JSON file (path)
     * @param peerIdsList
     * @param definitionFilePath
     */
    void loadAgentDefinitionFromPath(QStringList peerIdsList, QString definitionFilePath);


    /**
     * @brief Signal emitted when we have to load an agent mapping from a JSON file (path)
     * @param mappingFilePath
     */
    void loadAgentMappingFromPath(QStringList peerIdsList, QString mappingFilePath);


    /**
     * @brief Signal emitted when we have to download an agent definition to a JSON file (path)
     * @param agentDefinition
     * @param definitionFilePath
     */
    void downloadAgentDefinitionToPath(DefinitionM* agentDefinition, QString definitionFilePath);


    /**
     * @brief Signal emitted when we have to download an agent mapping to a JSON file (path)
     * @param agentMapping
     * @param mappingFilePath
     */
    void downloadAgentMappingToPath(AgentMappingM* agentMapping, QString mappingFilePath);


    /**
     * @brief Signal emitted when we have to open the "Log Stream" of a list of agents
     * @param models
     */
    void openLogStreamOfAgents(QList<AgentM*> models);


private Q_SLOTS:

    /**
     * @brief Slot when the list of models changed
     */
    void _onModelsChanged();


    /**
     * @brief Slot when the flag "is ON" of a model changed
     * @param isON
     */
    void _onIsONofModelChanged(bool isON);


    /**
     * @brief Slot when the flag "can Be Restarted" of a model changed
     * @param canBeRestarted
     */
    void _onCanBeRestartedOfModelChanged(bool canBeRestarted);


    /**
     * @brief Slot when the flag "is Muted" of a model changed
     * @param isMuted
     */
    void _onIsMutedOfModelChanged(bool isMuted);


    /**
     * @brief Slot when the flag "is Frozen" of a model changed
     * @param isMuted
     */
    void _onIsFrozenOfModelChanged(bool isFrozen);


    /**
     * @brief Slot when the definition of a model changed
     * @param definition
     */
    void _onDefinitionOfModelChanged(DefinitionM* definition);


    /**
     * @brief Slot when the state of a model changed
     * @param state
     */
    void _onStateOfModelChanged(QString state);


    /**
     * @brief Slot called when the flag "has Log in Stream" of a model changed
     * @param hasLogInStream
     */
    void _onHasLogInStreamOfModelChanged(bool hasLogInStream);


    /**
     * @brief Slot called when the flag "has Log in File" of a model changed
     * @param hasLogInFile
     */
    void _onHasLogInFileOfModelChanged(bool hasLogInFile);


    /**
     * @brief Slot called when the path of "Log File" of a model changed
     * @param logFilePath
     */
    void _onLogFilePathOfModelChanged(QString logFilePath);


    /**
     * @brief Slot called when the path of "Definition File" of a model changed
     * @param definitionFilePath
     */
    void _onDefinitionFilePathOfModelChanged(QString definitionFilePath);


    /**
     * @brief Slot called when the path of "Mapping File" of a model changed
     * @param mappingFilePath
     */
    void _onMappingFilePathOfModelChanged(QString mappingFilePath);


    /**
     * @brief Slot when a command must be sent on the network to an agent about one of its output
     * @param command
     * @param outputName
     */
    void _onCommandAskedToAgentAboutOutput(QString command, QString outputName);


    /**
     * @brief Slot when we have to open the values history of our agent
     */
    void _onOpenValuesHistoryOfAgent();


private:
    /**
     * @brief Update with all models of agents
     */
    void _updateWithAllModels();


    /**
     * @brief Update the flag "is ON" in function of flags of models
     */
    void _updateIsON();


    /**
     * @brief Update the flag "can Be Restarted" in function of flags of models
     */
    void _updateCanBeRestarted();


    /**
     * @brief Update the flag "is Muted" in function of flags of models
     */
    void _updateIsMuted();


    /**
     * @brief Update the flag "is Frozen" in function of flags of models
     */
    void _updateIsFrozen();


    /**
     * @brief Update the flag "has Log in Stram" in function of flags of models
     */
    void _updateHasLogInStream();


    /**
     * @brief Update the flag "is Enabled View Log Stram" in function of flags of models
     */
    void _updateIsEnabledViewLogStream();


    /**
     * @brief Update the flag "has Log in File" in function of flags of models
     */
    void _updateHasLogInFile();


    /**
     * @brief Update with the definition of the first model
     */
    void _updateWithDefinitionOfFirstModel();


    /**
     * @brief Update with the state of the first model
     */
    void _updateWithStateOfFirstModel();


    /**
     * @brief Update with the log file path of the first model
     */
    void _updateWithLogFilePathOfFirstModel();


    /**
     * @brief Update with the definition file path of the first model
     */
    void _updateWithDefinitionFilePathOfFirstModel();


    /**
     * @brief Update with the mapping file path of the first model
     */
    void _updateWithMappingFilePathOfFirstModel();


private:
    // Previous list of models of agents
    QList<AgentM*> _previousAgentsList;

    // List of peer ids of our models
    QStringList _peerIdsList;

};

QML_DECLARE_TYPE(AgentVM)

#endif // AGENTVM_H

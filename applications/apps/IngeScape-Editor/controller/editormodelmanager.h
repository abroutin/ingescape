/*
 *	IngeScape Editor
 *
 *  Copyright © 2017-2020 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *      Chloé Roumieu      <roumieu@ingenuity.io>
 */

#ifndef EDITORMODELMANAGER_H
#define EDITORMODELMANAGER_H

#include <QObject>
//#include <QtQml>
#include <I2PropertyHelpers.h>
#include <controller/ingescapemodelmanager.h>


/**
 * @brief The EditorModelManager class defines the manager for the data model of IngeScape
 */
class EditorModelManager : public QObject
{
    Q_OBJECT

    // List of opened definitions
    I2_QOBJECT_LISTMODEL(DefinitionM, openedDefinitions)


public:

    /**
     * @brief Accessor to the singleton instance
     * @return
     */
    //static EditorModelManager* instance();


    /**
     * @brief Method used to provide a singleton to QML
     * @param engine
     * @param scriptEngine
     * @return
     */
     //static QObject* qmlSingleton(QQmlEngine* engine, QJSEngine* scriptEngine);


    /**
     * @brief Constructor
     * @param parent
     */
    explicit EditorModelManager(QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~EditorModelManager();


    /**
     * @brief Export the agents into JSON
     * @return array of all agents (grouped by name)
     */
    QJsonArray exportAgentsToJSON();


    /**
     * @brief Export the agents list to selected file
     */
    Q_INVOKABLE void exportAgentsListToSelectedFile();


    /**
     * @brief Open a definition
     * If there are variants of this definition, we open each variant
     * @param definition
     */
    Q_INVOKABLE void openDefinition(DefinitionM* definition);


    /**
     * @brief Duplicate the agent with a new command line
     * @param agent
     * @param newCommandLine
     */
    Q_INVOKABLE void duplicateAgentWithNewCommandLine(AgentM* agent, QString newCommandLine);


Q_SIGNALS:


public Q_SLOTS:

    /**
     * @brief Slot called when a new view model of agents grouped by name has been created
     * @param agentsGroupedByName
     */
    void onAgentsGroupedByNameHasBeenCreated(AgentsGroupedByNameVM* agentsGroupedByName);


    // Slot called when the flag "is Muted" from an agent/agent output has been updated
    void onisMutedFromAgentUpdated(QString peerId, QString agentUid, bool isMuted);
    void onIsMutedFromOutputOfAgentUpdated(QString peerId, QString agentUid, bool isMuted, QString outputName);

    // Slot called when the flag "is Frozen" from an agent has been updated
    void onIsFrozenFromAgentUpdated(QString peerId, QString agentUid, bool isFrozen);

    // Slot called when the state of an agent has been updated
    void onAgentStateChanged(QString peerId, QString agentUid, QString stateName);

    // Slot called when we receive the flag "Log In Stream" for an agent
    void onAgentHasLogInStream(QString peerId, QString agentUid, bool hasLogInStream);

    // Slot called when we receive the flag "Log In File" for an agent
    void onAgentHasLogInFile(QString peerId, QString agentUid, bool hasLogInFile);

    // Slot called when we receive the file path of an agent log
    void onAgentLogFilePath(QString peerId, QString agentUid, QString logFilePath);

    // Slot called when we receive the file path of an agent Definition
    void onAgentDefinitionFilePath(QString peerId, QString agentUid, QString definitionFilePath);

    // Slot called when we receive the file path of an agent Mapping
    void onAgentMappingFilePath(QString peerId, QString agentUid, QString mappingFilePath);


private Q_SLOTS:

    /**
     * @brief Slot called when the definition(s) of an agent (agents grouped by name) must be opened
     * @param definitionsList
     */
    void _onDefinitionsToOpen(QList<DefinitionM*> definitionsList);


private:

    /**
     * @brief Open a list of definitions (if the definition is already opened, we bring it to front)
     * @param definitionsToOpen
     */
    void _openDefinitions(QList<DefinitionM*> definitionsToOpen);


private:


};

QML_DECLARE_TYPE(EditorModelManager)

#endif // EDITORMODELMANAGER_H

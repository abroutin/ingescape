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
 *      Bruno Lemenicier <lemenicier@ingenuity.io>
 *      Alexandre Lemort <lemort@ingenuity.io>
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

#ifndef HOSTVM_H
#define HOSTVM_H

#include <QObject>

#include <I2PropertyHelpers.h>

#include <model/hostm.h>
#include <model/agentm.h>
#include <model/agentexecutiononhostm.h>


/**
 * @brief The HostVM class defines a view model of host in the supervision list
 */
class HostVM : public QObject
{
    Q_OBJECT

    // Name of our host
    I2_QML_PROPERTY_READONLY(QString, name)

    // Model of our host
    I2_QML_PROPERTY_READONLY(HostM*, modelM)

    // List of associated Agents
    I2_QOBJECT_LISTMODEL(AgentM, listOfAgents)

    // Flag indicating if our agent can be frozen
    I2_QML_PROPERTY_READONLY(bool, canProvideStream)

    // Falg indicating if the host is streaming or not
    I2_QML_PROPERTY(bool, isStreaming)

    // List of agent execution on our host
    I2_QOBJECT_LISTMODEL(AgentExecutionOnHostM, listOfAgentExecution)


public:

    /**
     * @brief Constructor
     * @param parent
     */
    explicit HostVM(HostM* model, QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~HostVM();


    /**
     * @brief Change the state of our host
     */
    Q_INVOKABLE void changeState();


    /**
     * @brief Get the same agent execution on our host
     * @param agentName
     * @param commandLine
     * @return
     */
    AgentExecutionOnHostM* getSameAgentExecutionOnHost(QString agentName, QString commandLine);


Q_SIGNALS:

    /**
     * @brief Signal emitted when a command must be sent on the network to a launcher
     * @param command
     * @param hostname
     * @param commandLine
     */
    void commandAskedToHost(QString command, QString hostname, QString commandLine);

};

QML_DECLARE_TYPE(HostVM)

#endif // HOSTVM_H

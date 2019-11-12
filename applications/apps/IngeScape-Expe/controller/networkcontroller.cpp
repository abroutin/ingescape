/*
 *	IngeScape Expe
 *
 *  Copyright © 2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

#include "networkcontroller.h"


static const QString prefix_LoadPlatformFile = "LOAD_PLATFORM_FROM_PATH";
static const QString prefix_TimeLineState = "TIMELINE_STATE=";


//--------------------------------------------------------------
//
// Network Controller
//
//--------------------------------------------------------------


/**
 * @brief Constructor
 * @param parent
 */
NetworkController::NetworkController(QObject *parent) : IngeScapeNetworkController(parent)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    // Add  header to declare ourselves as expe
    igs_busAddServiceDescription("isExpe", "1");

    // We don't see itself
    setnumberOfExpes(1);
}


/**
 * @brief Destructor
 */
NetworkController::~NetworkController()
{
    // Mother class is automatically called
    //IngeScapeNetworkController::~IngeScapeNetworkController();
}


/**
 * @brief Manage a "Whispered" message
 * @param peerId
 * @param peerName
 * @param zMessage
 */
void NetworkController::manageWhisperedMessage(QString peerId, QString peerName, zmsg_t* zMessage)
{
    QString message = zmsg_popstr(zMessage);

    // An agent DEFINITION has been received
    if (message.startsWith(prefix_Definition))
    {
        QString definitionJSON = message.remove(0, prefix_Definition.length());

        Q_EMIT definitionReceived(peerId, peerName, definitionJSON);
    }
    // An agent MAPPING has been received
    else if (message.startsWith(prefix_Mapping))
    {
        QString mappingJSON = message.remove(0, prefix_Mapping.length());

        Q_EMIT mappingReceived(peerId, peerName, mappingJSON);
    }
    // Status of command "LOAD PLATFORM FROM PATH"
    else if (message.startsWith(prefix_LoadPlatformFile))
    {
        // Starts with the prefix, followed by parameters
        // Ends with white space followed by "STATUS=" and a digit
        QString pattern = QString("^%1=(.*)\\sSTATUS=(\\d)$").arg(prefix_LoadPlatformFile);
        QRegularExpression regExp(pattern);
        QRegularExpressionMatch regExpMatch = regExp.match(message);

        if (regExpMatch.hasMatch())
        {
            //QString matched = regExpMatch.captured(0);
            QString commandParameters = regExpMatch.captured(1);
            QString strStatus = regExpMatch.captured(2);

            bool successConvertStringToInt = false;
            int status = strStatus.toInt(&successConvertStringToInt);

            if (successConvertStringToInt)
            {
                // Emit the signal "Status Received about LoadPlatformFile"
                Q_EMIT statusReceivedAbout_LoadPlatformFile(static_cast<bool>(status), commandParameters);
            }
        }
    }
    // The state of the TimeLine updated (in Editor app)
    else if (message.startsWith(prefix_TimeLineState))
    {
        QString parameters = message.remove(0, prefix_TimeLineState.length());

        Q_EMIT timeLineStateUpdated(parameters);
    }
    // Unknown
    else {
        qDebug() << "Not yet managed WHISPERED message '" << message << "' for agent" << peerName << "(" << peerId << ")";
    }
}


/**
 * @brief Send a command and parameters to the editor
 * @param peerIdOfEditor
 * @param commandAndParameters
 */
void NetworkController::sendCommandToEditor(QString peerIdOfEditor, QString commandAndParameters)
{
    if (!peerIdOfEditor.isEmpty() && !commandAndParameters.isEmpty())
    {
        // Send the command (and parameters) to the peer id of the editor
        int success = igs_busSendStringToAgent(peerIdOfEditor.toStdString().c_str(), "%s", commandAndParameters.toStdString().c_str());

        qInfo() << "Send command (and parameters)" << commandAndParameters << "to editor" << peerIdOfEditor << "with success ?" << success;
    }
}
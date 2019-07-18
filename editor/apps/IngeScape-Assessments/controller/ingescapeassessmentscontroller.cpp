/*
 *	IngeScape Assessments
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

#include "ingescapeassessmentscontroller.h"

#include <QApplication>

#include <controller/assessmentsmodelmanager.h>
#include <misc/ingescapeutils.h>
#include <settings/ingescapesettings.h>


/**
 * @brief Constructor
 * @param parent
 */
IngeScapeAssessmentsController::IngeScapeAssessmentsController(QObject *parent) : QObject(parent),
    _networkDevice(""),
    _ipAddress(""),
    _port(0),
    _errorMessageWhenConnectionFailed(""),
    _snapshotDirectory(""),
    _networkC(nullptr),
    _experimentationsListC(nullptr),
    _experimentationC(nullptr),
    _subjectsC(nullptr),
    _tasksC(nullptr),
    _terminationSignalWatcher(nullptr),
    _jsonHelper(nullptr)
  //_platformDirectoryPath("")
{
    qInfo() << "New IngeScape Assessments Controller";

    //
    // Snapshots directory
    //
    QString snapshotsDirectoryPath = IngeScapeUtils::getSnapshotsPath();
    QDir snapshotsDirectory(snapshotsDirectoryPath);
    if (snapshotsDirectory.exists())
    {
        _snapshotDirectory = snapshotsDirectoryPath;
    }
    else
    {
        qCritical() << "ERROR: could not create directory at '" << snapshotsDirectoryPath << "' !";
    }


    //
    // Settings
    //
    IngeScapeSettings& settings = IngeScapeSettings::Instance();

    // Settings about the "Network"
    settings.beginGroup("network");
    _networkDevice = settings.value("networkDevice", QVariant("")).toString();
    _ipAddress = settings.value("ipAddress", QVariant("")).toString();
    _port = settings.value("port", QVariant(0)).toUInt();
    qInfo() << "Network Device:" << _networkDevice << "-- IP address:" << _ipAddress << "-- Port" << QString::number(_port);
    settings.endGroup();


    // Root directory
    QString rootPath = IngeScapeUtils::getRootPath();
    QDir rootDir(rootPath);
    if (!rootDir.exists()) {
        qCritical() << "ERROR: could not create directory at '" << rootPath << "' !";
    }


    // Directory for platform files
    /*QString platformPath = IngeScapeUtils::getPlatformsPath();

    QDir platformDir(platformPath);
    if (!platformDir.exists()) {
        qCritical() << "ERROR: could not create directory at '" << platformPath << "' !";
    }
    else
    {
        _platformDirectoryPath = platformPath;
    }*/


    // Create the helper to manage JSON files
    _jsonHelper = new JsonHelper(this);


    //
    // Create sub-controllers
    //

    // Create the manager for the data model of our IngeScape Assessments application
    AssessmentsModelManager::initInstance(_jsonHelper, rootPath, this);

    // Create the controller for network communications
    _networkC = new NetworkController(this);

    // Create the controller to manage the list of experimentations
    _experimentationsListC = new ExperimentationsListController(this);

    // Create the controller to manage the current experimentation
    _experimentationC = new ExperimentationController(_jsonHelper, this);

    // Create the controller to manage the subjects of the current experimentation
    _subjectsC = new SubjectsController(this);

    // Create the controller to manage the tasks of the current experimentation
    _tasksC = new TasksController(this);


    // Connect to signals from the experimentation controller to the rest of the controllers
    connect(_experimentationC, &ExperimentationController::currentExperimentationChanged,
            this, &IngeScapeAssessmentsController::_onCurrentExperimentationChanged);

    // Connect to signals from the network controller
    /*connect(_networkC, &NetworkController::agentEntered, _modelManager, &AssessmentsModelManager::onAgentEntered);
    connect(_networkC, &NetworkController::agentExited, _modelManager, &AssessmentsModelManager::onAgentExited);*/


    // Update the list of available network devices
    _networkC->updateAvailableNetworkDevices();

    // There is only one available network device, we use it !
    if (_networkC->availableNetworkDevices().count() == 1) {
        _networkDevice = _networkC->availableNetworkDevices().at(0);
    }

    // Start our INGESCAPE agent with a network device (or an IP address) and a port
    bool isStarted = _networkC->start(_networkDevice, _ipAddress, _port);

    if (isStarted && (AssessmentsModelManager::Instance() != nullptr))
    {
        // Initialize platform from online mapping
        AssessmentsModelManager::Instance()->setisMappingConnected(true);
    }
    else {
        seterrorMessageWhenConnectionFailed(tr("Failed to connect with network device %1 on port %2").arg(_networkDevice, QString::number(_port)));
    }


    //
    // Subscribe to system signals to interceipt interruption and termination signals
    //
    _terminationSignalWatcher = new TerminationSignalWatcher(this);
    connect(_terminationSignalWatcher, &TerminationSignalWatcher::terminationSignal,
            [=] () {
        qDebug() << "\n\n\n CATCH Termination Signal \n\n\n";

        if (QApplication::instance() != nullptr)
        {
            QApplication::instance()->quit();
        }
    });


    // Sleep to display our loading screen
    //QThread::msleep(2000);
}


/**
* @brief Destructor
*/
IngeScapeAssessmentsController::~IngeScapeAssessmentsController()
{
    //
    // Clean-up our TerminationSignalWatcher first
    //
    if (_terminationSignalWatcher != nullptr)
    {
        disconnect(_terminationSignalWatcher, nullptr);
        delete _terminationSignalWatcher;
        _terminationSignalWatcher = nullptr;
    }


    //
    // Clean-up sub-controllers
    //

    if (_experimentationsListC != nullptr)
    {
        disconnect(_experimentationsListC);

        ExperimentationsListController* temp = _experimentationsListC;
        setexperimentationsListC(nullptr);
        delete temp;
        temp = nullptr;
    }

    if (_experimentationC != nullptr)
    {
        disconnect(_experimentationC);

        ExperimentationController* temp = _experimentationC;
        setexperimentationC(nullptr);
        delete temp;
        temp = nullptr;
    }

    if (_subjectsC != nullptr)
    {
        disconnect(_subjectsC);

        SubjectsController* temp = _subjectsC;
        setsubjectsC(nullptr);
        delete temp;
        temp = nullptr;
    }

    if (_tasksC != nullptr)
    {
        disconnect(_tasksC);

        TasksController* temp = _tasksC;
        settasksC(nullptr);
        delete temp;
        temp = nullptr;
    }

    AssessmentsModelManager::destroyInstance();

    if (_networkC != nullptr)
    {
        disconnect(_networkC);

        NetworkController* temp = _networkC;
        setnetworkC(nullptr);
        delete temp;
        temp = nullptr;
    }

    // Delete json helper
    if (_jsonHelper != nullptr)
    {
        delete _jsonHelper;
        _jsonHelper = nullptr;
    }

    qInfo() << "Delete IngeScape Assessments Controller";
}


/**
* @brief Method used to provide a singleton to QML
* @param engine
* @param scriptEngine
* @return
*/
QObject* IngeScapeAssessmentsController::qmlSingleton(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    // NOTE: A QObject singleton type instance returned from a singleton type provider is owned by the QML engine.
    // For this reason, the singleton type provider function should not be implemented as a singleton factory.
    return new IngeScapeAssessmentsController();
}


/**
 * @brief Actions to perform before the application closing
 */
void IngeScapeAssessmentsController::processBeforeClosing()
{
    // TODO...
}


/**
 * @brief Method used to force the creation of our singleton from QML
 */
void IngeScapeAssessmentsController::forceCreation()
{
    qDebug() << "Force the creation of our singleton from QML";
}


/**
 * @brief Slot called when the current experimentation changed
 * @param value
 */
void IngeScapeAssessmentsController::_onCurrentExperimentationChanged(ExperimentationM* currentExperimentation)
{
    if (currentExperimentation != nullptr)
    {
        qDebug() << "Current Experimentation = " << currentExperimentation->name();

        if (_subjectsC != nullptr) {
            _subjectsC->setcurrentExperimentation(currentExperimentation);
        }

        if (_tasksC != nullptr) {
            _tasksC->setcurrentExperimentation(currentExperimentation);
        }

    }
    else {
        qDebug() << "Current Experimentation is NULL !";
    }
}

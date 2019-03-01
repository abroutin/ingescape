/*
 *	IngeScape Measuring
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

#include <QApplication>
#include <QtQml>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QtGlobal>
#include <QDate>

#include <I2Quick.h>

#include <controller/ingescapemeasuringcontroller.h>

//
// IngeScape Common
//
#include <settings/ingescapesettings.h>
#include <misc/ingescapeutils.h>


// FIXME: Why this include is not needed in the IngeScape Editor ?
extern "C" {
#include <ingescape.h>
}


/**
 * @brief Handler for "Log Message"
 * @param type
 * @param context
 * @param message
 */
void LogMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    fprintf(stdout, "%s\n", message.toLatin1().constData());
    fflush(stdout);

    // "context.function" can be null
    QString function = "Undefined function";
    if (context.function != nullptr) {
        function = QString(context.function);
    }

    // Get the current log level
    igs_logLevel_t logLevel = igs_getLogLevel();

    switch (type)
    {
    case QtDebugMsg:
    {
        // Allows to prevent to log in the file useless logs (with type TRACE or DEBUG)
        if (logLevel <= IGS_LOG_DEBUG) {
            igs_log(IGS_LOG_DEBUG, function.toStdString().c_str(), message.toStdString().c_str(), "%s");
        }
        break;
    }
    case QtInfoMsg:
    {
        igs_log(IGS_LOG_INFO, function.toStdString().c_str(), message.toStdString().c_str(), "%s");
        break;
    }
    case QtWarningMsg:
    {
        igs_log(IGS_LOG_WARN, function.toStdString().c_str(), message.toStdString().c_str(), "%s");
        break;
    }
    case QtCriticalMsg:
    {
        igs_log(IGS_LOG_ERROR, function.toStdString().c_str(), message.toStdString().c_str(), "%s");
        break;
    }
    case QtFatalMsg:
    {
        igs_log(IGS_LOG_FATAL, function.toStdString().c_str(), message.toStdString().c_str(), "%s");
        break;
    }
    default:
        // NO LOG IT (QtSystemMsg)
        break;
    }
}


/**
 * @brief Register our C++ types and extensions in the QML system
 */
void registerCustomQmlTypes()
{

    //---------------
    //
    // Enums
    //
    //---------------
    qmlRegisterSingletonType<CharacteristicValueTypes>("INGESCAPE", 1, 0, "CharacteristicValueTypes", &CharacteristicValueTypes::qmlSingleton);
    qmlRegisterSingletonType<IndependentVariableValueTypes>("INGESCAPE", 1, 0, "IndependentVariableValueTypes", &IndependentVariableValueTypes::qmlSingleton);


    //----------------
    //
    // Controllers
    //
    //----------------
    // Singleton used as primary access point to our controllers
    qmlRegisterSingletonType<IngeScapeMeasuringController>("INGESCAPE", 1, 0, "IngeScapeMeasuringC", &IngeScapeMeasuringController::qmlSingleton);

    // sub-controllers
    qmlRegisterUncreatableType<IngeScapeModelManager>("INGESCAPE", 1, 0, "IngeScapeModelManager", "Internal Class");
    //qmlRegisterUncreatableType<AgentsMappingController>("INGESCAPE", 1, 0, "AgentsMappingController", "Internal Class");


    //----------------
    //
    // Sort and Filter
    //
    //----------------
    //qmlRegisterUncreatableType<LogsSortFilter>("INGESCAPE", 1, 0, "LogsSortFilter", "Internal class");
    //qmlRegisterUncreatableType<ValuesHistorySortFilter>("INGESCAPE", 1, 0, "ValuesHistorySortFilter", "Internal class");


    //----------------
    //
    // Misc.
    //
    //----------------
    //qmlRegisterType<CollapsibleColumn>("INGESCAPE", 1, 0, "CollapsibleColumn");
    //qmlRegisterType<QQuickWindowBlockTouches>("INGESCAPE", 1, 0, "WindowBlockTouches");


    //----------------
    //
    // Models
    //
    //----------------
    qmlRegisterUncreatableType<CharacteristicEnumM>("INGESCAPE", 1, 0, "CharacteristicEnumM", "Internal class");
    qmlRegisterUncreatableType<CharacteristicM>("INGESCAPE", 1, 0, "CharacteristicM", "Internal class");
    qmlRegisterUncreatableType<DependentVariableM>("INGESCAPE", 1, 0, "DependentVariableM", "Internal class");
    qmlRegisterUncreatableType<ExperimentationM>("INGESCAPE", 1, 0, "ExperimentationM", "Internal class");
    qmlRegisterUncreatableType<IndependentVariableEnumM>("INGESCAPE", 1, 0, "IndependentVariableEnumM", "Internal class");
    qmlRegisterUncreatableType<IndependentVariableM>("INGESCAPE", 1, 0, "IndependentVariableM", "Internal class");
    qmlRegisterUncreatableType<RecordM>("INGESCAPE", 1, 0, "RecordM", "Internal class");
    qmlRegisterUncreatableType<SubjectM>("INGESCAPE", 1, 0, "SubjectM", "Internal class");
    qmlRegisterUncreatableType<TaskM>("INGESCAPE", 1, 0, "TaskM", "Internal class");


    //---------------
    //
    // View Models
    //
    //---------------
    //qmlRegisterUncreatableType<ActionConditionVM>("INGESCAPE", 1, 0, "ActionConditionVM", "Internal class");
    //qmlRegisterUncreatableType<ActionEffectVM>("INGESCAPE", 1, 0, "ActionEffectVM", "Internal class");


    //------------------
    //
    // QML components
    //
    //------------------
    // - Singleton used to enable consistent app styling through predefined colors, fonts, etc.
    qmlRegisterSingletonType(QUrl("qrc:/qml/theme/IngeScapeTheme.qml"), "INGESCAPE", 1, 0, "IngeScapeTheme");

    // - Scrollview style
    //qmlRegisterType(QUrl("qrc:/qml/theme/IngeScapeScrollViewStyle.qml"), "INGESCAPE", 1, 0, "IngeScapeScrollViewStyle");

    // - Combobox style
    //qmlRegisterType(QUrl("qrc:/qml/theme/IngeScapeComboboxStyle.qml"), "INGESCAPE", 1, 0, "IngeScapeComboboxStyle");

    // - Combobox for ItemModel with tooltip delegate
    //qmlRegisterType(QUrl("qrc:/qml/scenario/IngeScapeComboboxItemModel.qml"), "INGESCAPE", 1, 0, "IngeScapeComboboxItemModel");

    // - Combobox for Agents IOP model with a custom tooltip delegate to show the color circles
    //qmlRegisterType(QUrl("qrc:/qml/scenario/IngeScapeComboboxItemModelAgentsIOP.qml"), "INGESCAPE", 1, 0, "IngeScapeComboboxItemModelAgentsIOP");

    // - Combobox delegate with tooltip
    //qmlRegisterType(QUrl("qrc:/qml/scenario/IngeScapeToolTipComboboxDelegate.qml"), "INGESCAPE", 1, 0, "IngeScapeToolTipComboboxDelegate");

    // - Combobox delegate for Agents IOP comboboxes to show the color circles
    //qmlRegisterType(QUrl("qrc:/qml/scenario/IngeScapeToolTipComboboxDelegateAgentsIOP.qml"), "INGESCAPE", 1, 0, "IngeScapeToolTipComboboxDelegateAgentsIOP");

}


/**
 * @brief Entry point of our application program
 * @param argc Argument count
 * @param argv List of arguments
 * @return
 */
int main(int argc, char *argv[])
{
    //------------------------------
    //
    // Configure Qt
    //
    //------------------------------

    // Automatic high-DPI scaling
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);


    //------------------------------
    //
    // Configure our application
    //
    //------------------------------
    QApplication app(argc, argv);
    app.setOrganizationName("Ingenuity i/o");
    app.setOrganizationDomain("ingenuity.io");
    app.setApplicationName("IngeScape-Measuring");
    app.setApplicationVersion("0.1.0.0");

    // - behavior when our last window is closed
    app.setQuitOnLastWindowClosed(true);


    //------------------------------
    //
    // Root directory
    //
    //------------------------------

    // Get (and create if needed) the root path of our application ([DocumentsLocation]/IngeScape/)
    QString rootDirectoryPath = IngeScapeUtils::getRootPath();
    if (!rootDirectoryPath.isEmpty())
    {
        //------------------------------
        //
        // Logs
        //
        //------------------------------

        QString logFilePath = QString("%1log-IngeScape-Measuring.csv").arg(rootDirectoryPath);

        // Set the log level from which logs are printed in the console
        //igs_setLogLevel(IGS_LOG_DEBUG);
        igs_setLogLevel(IGS_LOG_INFO);

        igs_setLogPath(logFilePath.toStdString().c_str());
        igs_setLogInFile(true);

#ifdef QT_DEBUG
        // Use a custom message format
        qSetMessagePattern("%{time hh:mm:ss.zzz} %{type} %{if-category}%{category} %{endif}file://%{file}:%{line} %{message}");
#else
        // Replace the default message handler with our own logger
        qInstallMessageHandler(LogMessageHandler);
#endif

        qInfo() << "Application" << app.applicationName() << "is running with version" << app.applicationVersion();


        //------------------------------
        //
        // Settings
        //
        //------------------------------

        // Get (and create if needed) the settings path of our application ([DocumentsLocation]/IngeScape/Settings/)
        QString settingsDirectoryPath = IngeScapeUtils::getSettingsPath();
        QString settingsFilePath = QString("%1IngeScape-Measuring.ini").arg(settingsDirectoryPath);

        QFile settingsFile(settingsFilePath);
        if (!settingsFile.exists())
        {
            // Copy our .ini file
            if (QFile::copy(":/settings/IngeScape-Measuring.ini", settingsFilePath))
            {
                // Update permissions for our .ini file
                QFileDevice::Permissions filePermissions =  QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::WriteGroup;
                if (!QFile::setPermissions(settingsFilePath, filePermissions))
                {
                    qWarning() << "ERROR during set permissions to file" << settingsFilePath;
                }
            }
            else
            {
                qCritical() << "ERROR during copy of settings into" << settingsFilePath;
            }
        }


        // Récupération des différentes configurations
        if (settingsFile.exists()) {
            // Load our settings
            IngeScapeSettings &settings = IngeScapeSettings::Instance(settingsFilePath);

            qDebug() << "Settings" << settings.fileName();
        }
    }
    else
    {
        qFatal("ERROR: There is no special directory 'Documents' !");
    }


    //------------------------------
    //
    // Configure our QML engine
    //
    //------------------------------
    QQmlApplicationEngine engine;


    //
    // Register and configure I2Quick
    //
    // - register QML extensions
    I2Quick::registerTypes();
    I2Quick::initializeEngine(&engine);

    // - Display error images when SVG items are missing (source: "image://I2svg/.....")
    I2SVGImageProvider::showErrorsAsImage = true;

    //
    // Register our custom QML types
    //
    registerCustomQmlTypes();


    //
    // Defines context properties
    //
    QQmlContext* qmlContext = engine.rootContext();
    if (qmlContext != nullptr)
    {
        // Build date
        qmlContext->setContextProperty("BUILD_DATE", QLocale(QLocale::C).toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy")));

        // Qt version against which the application is compiled
        qmlContext->setContextProperty("QT_BUILD_VERSION", QString(QT_VERSION_STR));

        // Runtime version of Qt. This may be a different version than the version the application was compiled against
        qmlContext->setContextProperty("QT_RUNTIME_VERSION", QString(qVersion()));
    }


    //
    // Load our main QML file
    //
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));


    //------------------------------
    //
    // Mainloop
    //
    //------------------------------

    qInfo() << "Starting" << app.applicationName() << app.applicationVersion() << "with processID" << QApplication::applicationPid();
    int exitReturnCode = app.exec();
    qInfo() << "Quitting application with return code" << exitReturnCode;
}
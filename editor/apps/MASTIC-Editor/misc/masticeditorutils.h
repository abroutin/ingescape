
/*
 *	MasticEditorUtils
 *
 *  Copyright (c) 2016-2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *
 */

#ifndef MASTICEDITORUTILS_H
#define MASTICEDITORUTILS_H

#include <QObject>
#include <QtQml>
#include <QQmlEngine>
#include <QJSEngine>

#include "I2PropertyHelpers.h"



/**
 * @brief The MasticEditorUtils class defines the main controller of our application
 */
class MasticEditorUtils: public QObject
{
    Q_OBJECT


public:

    /**
     * @brief Default constructor
     * @param parent
     */
    explicit MasticEditorUtils(QObject *parent = 0);


    /**
      * @brief Destructor
      */
    ~MasticEditorUtils();

    /**
      * @brief Create a directory if it does not exist
      * @param directoryPath
      */
    static void createDirectoryIfNotExist(QString directoryPath);

    /**
     * @brief Get (and create if needed) the root path of our application
     * "[DocumentsLocation]/MASTIC/"
     * @return
     */
    static QString getRootPath();

    /**
     * @brief Get (and create if needed) the settings path of our application
     * "[DocumentsLocation]/MASTIC/settings/"
     * @return
     */
    static QString getSettingsPath();

    /**
     * @brief Get (and create if needed) the data path of our application
     * "[DocumentsLocation]/MASTIC/data/"
     * @return
     */
    static QString getDataPath();

    /**
     * @brief Get (and create if needed) the snapshots path of our application
     * "[DocumentsLocation]/MASTIC/snapshots/"
     * @return
     */
    static QString getSnapshotsPath();

    /**
     * @brief Get (and create if needed) the agents definitions path of our application
     * "[DocumentsLocation]/MASTIC/AgentsDefinitions/"
     * @return
     */
    static QString getAgentsDefinitionsPath();

Q_SIGNALS:


public Q_SLOTS:


protected:



};

QML_DECLARE_TYPE(MasticEditorUtils)

#endif // MASTICEDITORUTILS_H

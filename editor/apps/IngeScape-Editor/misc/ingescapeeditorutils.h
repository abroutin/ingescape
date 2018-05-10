/*
 *	IngeScape Editor
 *
 *  Copyright (c) 2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Deliencourt <deliencourt@ingenuity.io>
 *      Alexandre Lemort    <lemort@ingenuity.io>
 *
 */

#ifndef INGESCAPEEDITORUTILS_H
#define INGESCAPEEDITORUTILS_H

#include <QObject>
#include <QtQml>




/**
 * @brief The IngeScapeEditorUtils class defines the ingescape editor utils
 */
class IngeScapeEditorUtils: public QObject
{
    Q_OBJECT


public:

    /**
     * @brief Default constructor
     * @param parent
     */
    explicit IngeScapeEditorUtils(QObject *parent = 0);


    /**
      * @brief Destructor
      */
    ~IngeScapeEditorUtils();


    /**
      * @brief Create a directory if it does not exist
      * @param directoryPath
      */
    static void createDirectoryIfNotExist(QString directoryPath);


    /**
     * @brief Get (and create if needed) the root path of our application
     * "[DocumentsLocation]/INGESCAPE/"
     * @return
     */
    static QString getRootPath();


    /**
     * @brief Get (and create if needed) the settings path of our application
     * "[DocumentsLocation]/INGESCAPE/settings/"
     * @return
     */
    static QString getSettingsPath();


    /**
     * @brief Get (and create if needed) the data path of our application
     * "[DocumentsLocation]/INGESCAPE/data/"
     * @return
     */
    static QString getDataPath();


    /**
     * @brief Get (and create if needed) the snapshots path of our application
     * "[DocumentsLocation]/INGESCAPE/snapshots/"
     * @return
     */
    static QString getSnapshotsPath();


    /**
     * @brief Get (and create if needed) the path with files about agents list
     * "[DocumentsLocation]/INGESCAPE/AgentsList/"
     * @return
     */
    static QString getAgentsListPath();


    /**
     * @brief Get (and create if needed) the path with files about agents mappings
     * "[DocumentsLocation]/INGESCAPE/AgentsMappings/"
     * @return
     */
    static QString getAgentsMappingsPath();

    /**
     * @brief Get (and create if needed) the path with files about scenarios
     * "[DocumentsLocation]/INGESCAPE/Scenarios/"
     * @return
     */
    static QString getScenariosPath();

    /**
     * @brief Get (and create if needed) the path with files about platforms
     * "[DocumentsLocation]/INGESCAPE/Platforms/"
     * @return
     */
    static QString getPlatformsPath();


protected:
    /**
     * @brief Get (and create if needed) the fullpath of a given sub-directory
     * @param subDirectory
     * @return
     */
    static QString _getSubDirectoryPath(QString subDirectory);
};

QML_DECLARE_TYPE(IngeScapeEditorUtils)

#endif // INGESCAPEEDITORUTILS_H
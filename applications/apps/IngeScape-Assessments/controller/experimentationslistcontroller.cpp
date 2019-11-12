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

#include "experimentationslistcontroller.h"

#include <controller/assessmentsmodelmanager.h>

/**
 * @brief Constructor
 * @param parent
 */
ExperimentationsListController::ExperimentationsListController(QObject *parent) : QObject(parent),
    _defaultGroupOther(nullptr),
    _newGroup(nullptr),
    _experimentationsTotalNumber(0)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    qInfo() << "New Experimentations List Controller";

    // Groups are sorted on their name (alphabetical order)
    _allExperimentationsGroups.setSortProperty("name");
    _allExperimentationsGroupsWithoutOthers.setSortProperty("name");


    //
    // Create the default group "Other"
    //
    _defaultGroupOther = new ExperimentationsGroupVM("Others", nullptr);

    _allExperimentationsGroups.append(_defaultGroupOther);

    _hashFromNameToExperimentationsGroup.insert(_defaultGroupOther->name(), _defaultGroupOther);


    // Create the (fake) group "New"
    _newGroup = new ExperimentationsGroupVM(tr("New Group"), nullptr);


    // Get the model manager
    AssessmentsModelManager* modelManager = AssessmentsModelManager::Instance();

    // If we are connected to the database
    if ((modelManager != nullptr) && modelManager->isConnectedToDatabase())
    {
        // Init the list of experimentations
        _initExperimentationsList();
    }
}


/**
 * @brief Destructor
 */
ExperimentationsListController::~ExperimentationsListController()
{
    qInfo() << "Delete Experimentations List Controller";

    _allExperimentationsGroups.clear();
    //_allExperimentationsGroups.deleteAllItems();

    _allExperimentationsGroupsWithoutOthers.clear();

    if (_defaultGroupOther != nullptr)
    {
        ExperimentationsGroupVM* temp = _defaultGroupOther;
        setdefaultGroupOther(nullptr);
        delete temp;
    }

    if (_newGroup != nullptr)
    {
        ExperimentationsGroupVM* temp = _newGroup;
        setnewGroup(nullptr);
        delete temp;
    }
}


/**
 * @brief Update when we are just connected to a database
 */
void ExperimentationsListController::updateWhenConnectedDatabase()
{
    // Init the list of experimentations
    _initExperimentationsList();
}


/**
 * @brief update when we are just DIS-connected from a database
 */
void ExperimentationsListController::updateWhenDISconnectedDatabase()
{
    // FIXME TODO: Clear the list of experimentations ?
    //_clearExperimentationsList();
}


/**
 * @brief Create a new experimentation in new group (to create)
 * @param experimentationName
 * @param newExperimentationsGroupName
 */

void ExperimentationsListController::createNewExperimentationInNewGroup(QString experimentationName, QString newExperimentationsGroupName)
{
    if (!experimentationName.isEmpty() && !newExperimentationsGroupName.isEmpty())
    {
        ExperimentationsGroupVM* experimentationsGroup = _getExperimentationsGroupFromName(newExperimentationsGroupName);
        if (experimentationsGroup == nullptr)
        {
            // Create a new experimentations group
            experimentationsGroup = _createNewExperimentationsGroup(newExperimentationsGroupName);
        }

        if (experimentationsGroup != nullptr)
        {
            // Create a new experimentation in this new group
            createNewExperimentationInGroup(experimentationName, experimentationsGroup);
        }
    }
}


/**
 * @brief Create a new experimentation in a group
 * @param experimentationName
 * @param experimentationsGroup
 */
void ExperimentationsListController::createNewExperimentationInGroup(QString experimentationName, ExperimentationsGroupVM* experimentationsGroup)
{
    if (!experimentationName.isEmpty() && (experimentationsGroup != nullptr)
            && (AssessmentsModelManager::Instance() != nullptr))
    {
        ExperimentationM* experimentation = new ExperimentationM(AssessmentsModelManager::genCassUuid(),
                                                                 experimentationName,
                                                                 experimentationsGroup->name(),
                                                                 QDateTime::currentDateTime(),
                                                                 nullptr);

        if ((experimentation != nullptr) && AssessmentsModelManager::insert(*experimentation))
        {
            // Add to the group
            experimentationsGroup->experimentations()->append(experimentation);

            // Update the total number of experimentations
            setexperimentationsTotalNumber(_experimentationsTotalNumber + 1);
        }
    }
    else {
        qWarning() << "Cannot create new experimentation because name is empty (" << experimentationName << ") or group is null !";
    }
}


/**
 * @brief Return true if the user can create an experimentations group with the name
 * Check if the name is not empty and if a group with the same name does not already exist
 * @param experimentationsGroupName
 * @return
 */
bool ExperimentationsListController::canCreateExperimentationsGroupWithName(QString experimentationsGroupName)
{
    return !experimentationsGroupName.isEmpty() && !_hashFromNameToExperimentationsGroup.contains(experimentationsGroupName);
}


/**
 * @brief Delete an experimentation of a group
 * @param experimentation
 * @param experimentationsGroup
 */
void ExperimentationsListController::deleteExperimentationOfGroup(ExperimentationM* experimentation, ExperimentationsGroupVM* experimentationsGroup)
{
    if ((experimentation != nullptr) && (experimentationsGroup != nullptr))
    {
        qInfo() << "Delete the experimentation" << experimentation->name() << "of the group" << experimentationsGroup->name();

        // Remove from the group
        experimentationsGroup->experimentations()->remove(experimentation);

        // Update the total number of experimentations
        setexperimentationsTotalNumber(_experimentationsTotalNumber - 1);

        // Remove from DB
        ExperimentationM::deleteExperimentationFromCassandra(*experimentation);

        // Free memory
        delete experimentation;

        // The group is empty (and is not the default group "Other")
        if ((experimentationsGroup->experimentations()->count() == 0) && (experimentationsGroup != _defaultGroupOther))
        {
            // Remove the experimentations group
            _removeExperimentationsGroup(experimentationsGroup);
        }
    }
}


/**
 * @brief Get the group of experimentations from a name
 * @param experimentationsGroupName
 * @return
 */
ExperimentationsGroupVM* ExperimentationsListController::_getExperimentationsGroupFromName(QString experimentationsGroupName)
{
    return _hashFromNameToExperimentationsGroup.value(experimentationsGroupName, nullptr);
}


/**
 * @brief Creates a new experimentation group and returns it
 * @param experimentationsGroup
 * @param newExperimentationsGroupName
 */
ExperimentationsGroupVM* ExperimentationsListController::_createNewExperimentationsGroup(const QString& newExperimentationsGroupName)
{
    ExperimentationsGroupVM* experimentationsGroup = new ExperimentationsGroupVM(newExperimentationsGroupName, nullptr);
    if (experimentationsGroup != nullptr)
    {
        _allExperimentationsGroups.append(experimentationsGroup);
        _allExperimentationsGroupsWithoutOthers.append(experimentationsGroup);
        _hashFromNameToExperimentationsGroup.insert(newExperimentationsGroupName, experimentationsGroup);
    }
    return experimentationsGroup;
}


/**
 * @brief Remove an experimentations group
 * @param experimentationsGroup
 */
void ExperimentationsListController::_removeExperimentationsGroup(ExperimentationsGroupVM* experimentationsGroup)
{
    if (experimentationsGroup != nullptr)
    {
        _allExperimentationsGroups.remove(experimentationsGroup);
        _allExperimentationsGroupsWithoutOthers.remove(experimentationsGroup);
        _hashFromNameToExperimentationsGroup.remove(experimentationsGroup->name());

        delete experimentationsGroup;
    }
}


/**
 * @brief Init the list of experimentations
 */
void ExperimentationsListController::_initExperimentationsList()
{
    if (AssessmentsModelManager::Instance()->isConnectedToDatabase())
    {
        // Create the query
        QList<ExperimentationM*> experimentationsList = AssessmentsModelManager::select<ExperimentationM>({}); // #NoFilter
        for (ExperimentationM* experimentation : experimentationsList)
        {
            if (experimentation != nullptr)
            {
                ExperimentationsGroupVM* experimentationsGroup = _getExperimentationsGroupFromName(experimentation->groupName());
                if (experimentationsGroup == nullptr)
                {
                    // Create the Experimentations Group VM
                    experimentationsGroup = _createNewExperimentationsGroup(experimentation->groupName());
                }

                if (experimentationsGroup != nullptr)
                {
                    // Add to the group
                    experimentationsGroup->experimentations()->append(experimentation);

                    // Update the total number of experimentations
                    setexperimentationsTotalNumber(_experimentationsTotalNumber + 1);
                }
            }
        }
    }
}
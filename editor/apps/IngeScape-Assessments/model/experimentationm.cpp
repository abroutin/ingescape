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

#include "experimentationm.h"
#include <misc/ingescapeutils.h>

#include "controller/assessmentsmodelmanager.h"

// Experimentation table name
const QString ExperimentationM::table = "ingescape.experimentation";

/**
 * @brief Constructor
 * @param name
 * @param creationDate
 * @param parent
 */
ExperimentationM::ExperimentationM(CassUuid cassUuid,
                                   QString name,
                                   QString groupeName,
                                   QDateTime creationDate,
                                   QObject *parent) : QObject(parent),
    _name(name),
    _groupName(groupeName),
    _creationDate(creationDate),
    _cassUuid(cassUuid)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    qInfo() << "New Model of Experimentation" << _name << "created" << _creationDate.toString("dd/MM/yy hh:mm:ss") << "(" << AssessmentsModelManager::cassUuidToQString(_cassUuid) << ")";

    // Task instances are sorted on their start date/time (chronological order)
    _allTaskInstances.setSortProperty("startDateTime");
}


/**
 * @brief Destructor
 */
ExperimentationM::~ExperimentationM()
{
    qInfo() << "Delete Model of Experimentation" << _name << "created" << _creationDate.toString("dd/MM/yy hh:mm:ss") << "(" << AssessmentsModelManager::cassUuidToQString(_cassUuid) << ")";

    clearData();
}


/**
 * @brief Clear the attribute lists
 */
void ExperimentationM::clearData()
{
    // Delete all task instances of our experimentation
    _allTaskInstances.deleteAllItems();

    // Delete all characteristics of our experimentation
    _hashFromUIDtoCharacteristic.clear();
    _allCharacteristics.deleteAllItems();

    // Delete all subjects of our experimentation
    _allSubjects.deleteAllItems();

    // Delete all tasks of our experimentation
    _allTasks.deleteAllItems();
}


void ExperimentationM::setname(QString value)
{
    if (_name != value)
    {
        _name = value;

        QString queryStr = "UPDATE " + ExperimentationM::table + " SET name = ? WHERE id = ?;";
        CassStatement* cassStatement = cass_statement_new(queryStr.toStdString().c_str(), 2);
        cass_statement_bind_string(cassStatement, 0, value.toStdString().c_str());
        cass_statement_bind_uuid  (cassStatement, 1, _cassUuid);
        // Execute the query or bound statement
        CassFuture* cassFuture = cass_session_execute(AssessmentsModelManager::Instance()->getCassSession(), cassStatement);
        CassError cassError = cass_future_error_code(cassFuture);
        if (cassError == CASS_OK)
        {
            qInfo() << "Experimentation renamed successfully";
        }
        else {
            qCritical() << "Could not update the name of the experimentation" << cass_error_desc(cassError);
        }

        Q_EMIT nameChanged(value);
    }
}


/**
 * @brief Get the unique identifier in Cassandra Data Base
 * @return
 */
CassUuid ExperimentationM::getCassUuid() const
{
    return _cassUuid;
}


/**
 * @brief Add a characteristic to our experimentation
 * @param characteristic
 */
void ExperimentationM::addCharacteristic(CharacteristicM* characteristic)
{
    if ((characteristic != nullptr) && !_hashFromUIDtoCharacteristic.contains(characteristic->getCassUuid()))
    {
        // Add to the hash
        _hashFromUIDtoCharacteristic.insert(characteristic->getCassUuid(), characteristic);

        // Add to the list
        _allCharacteristics.append(characteristic);
    }
}


/**
 * @brief Remove a characteristic from our experimentation
 * @param characteristic
 */
void ExperimentationM::removeCharacteristic(CharacteristicM* characteristic)
{
    if ((characteristic != nullptr) && _hashFromUIDtoCharacteristic.contains(characteristic->getCassUuid()))
    {
        // Remove from the hash
        _hashFromUIDtoCharacteristic.remove(characteristic->getCassUuid());

        // Remove from the list
        _allCharacteristics.remove(characteristic);
    }
}


/**
 * @brief Add a subject to our experimentation
 * @param subject
 */
void ExperimentationM::addSubject(SubjectM* subject)
{
    if (subject != nullptr)
    {
        // Add to the list
        _allSubjects.append(subject);
    }
}


/**
 * @brief Remove a subject from our experimentation
 * @param subject
 */
void ExperimentationM::removeSubject(SubjectM* subject)
{
    if (subject != nullptr)
    {
        // Remove from the list
        _allSubjects.remove(subject);

        // Remove related TaskInstances
        removeTaskInstanceRelatedToSubject(subject);
    }
}


/**
 * @brief Add a task to our experimentation
 * @param task
 */
void ExperimentationM::addTask(TaskM* task)
{
    if (task != nullptr)
    {
        // Add to the list
        _allTasks.append(task);
    }
}


/**
 * @brief Remove a task from our experimentation
 * @param task
 */
void ExperimentationM::removeTask(TaskM* task)
{
    if (task != nullptr)
    {
        // Remove from the list
        _allTasks.remove(task);

        // Remove related TaskInstances
        removeTaskInstanceRelatedToTask(task);
    }
}


/**
 * @brief Add a task instance to our experimentation
 * @param taskInstance
 */
void ExperimentationM::addTaskInstance(TaskInstanceM* taskInstance)
{
    if (taskInstance != nullptr)
    {
        // Add to the list
        _allTaskInstances.append(taskInstance);
    }
}


/**
 * @brief Remove a task instance from our experimentation
 * @param taskInstance
 */
void ExperimentationM::removeTaskInstance(TaskInstanceM* taskInstance)
{
    if (taskInstance != nullptr)
    {
        // Remove from the list
        _allTaskInstances.remove(taskInstance);
    }
}


/**
 * @brief Remove task instances related to the given subject
 * @param subject
 */
void ExperimentationM::removeTaskInstanceRelatedToSubject(SubjectM* subject)
{
    const QList<TaskInstanceM*> taskInstanceList = _allTaskInstances.toList();
    auto taskInstanceIt = taskInstanceList.begin();
    while (taskInstanceIt != taskInstanceList.end())
    {
        // Looking for a task instance related to the given subject
        taskInstanceIt = std::find_if(taskInstanceIt, taskInstanceList.end(), [subject](TaskInstanceM* taskInstance){
                return (taskInstance != nullptr) && (taskInstance->subject() == subject);
        });

        if (taskInstanceIt != taskInstanceList.end())
        {
            // We found a task instance with given subject
            removeTaskInstance(*taskInstanceIt);
            ++taskInstanceIt;
        }
    }
}


/**
 * @brief Remove task instances related to the given task
 * @param task
 */
void ExperimentationM::removeTaskInstanceRelatedToTask(TaskM* task)
{
    const QList<TaskInstanceM*> taskInstanceList = _allTaskInstances.toList();
    auto taskInstanceIt = taskInstanceList.begin();
    while (taskInstanceIt != taskInstanceList.end())
    {
        // Looking for a task instance related to the given subject
        taskInstanceIt = std::find_if(taskInstanceIt, taskInstanceList.end(), [task](TaskInstanceM* taskInstance){
                return (taskInstance != nullptr) && (taskInstance->task() == task);
        });

        if (taskInstanceIt != taskInstanceList.end())
        {
            // We found a task instance with given subject
            removeTaskInstance(*taskInstanceIt);
            ++taskInstanceIt;
        }
    }
}


/**
 * @brief Get a characteristic from its UUID
 * @param cassUuid
 * @return
 */
CharacteristicM* ExperimentationM::getCharacteristicFromUID(const CassUuid& cassUuid)
{
    return _hashFromUIDtoCharacteristic.value(cassUuid, nullptr);
}

/**
 * @brief Get a task from its UUID
 * @param cassUuid
 * @return
 */
SubjectM* ExperimentationM::getSubjectFromUID(const CassUuid& cassUuid)
{
    auto subjectIt = std::find_if(_allSubjects.begin(), _allSubjects.end(), [cassUuid](SubjectM* subject) { return (subject != nullptr) && (subject->getCassUuid() == cassUuid); });
    return (subjectIt != _allSubjects.end()) ? *subjectIt : nullptr;
}


/**
 * @brief Get a task from its UUID
 * @param cassUuid
 * @return
 */
TaskM* ExperimentationM::getTaskFromUID(const CassUuid& cassUuid)
{
    auto taskIt = std::find_if(_allTasks.begin(), _allTasks.end(), [cassUuid](TaskM* task) { return (task != nullptr) && (task->getCassUuid() == cassUuid); });
    return (taskIt != _allTasks.end()) ? *taskIt : nullptr;
}


/**
 * @brief Static factory method to create an experiment from a CassandraDB record
 * @param row
 * @return
 */
ExperimentationM* ExperimentationM::createExperimentationFromCassandraRow(const CassRow* row)
{
    ExperimentationM* experimentation = nullptr;

    if (row != nullptr)
    {
        CassUuid experimentationUid;
        cass_value_get_uuid(cass_row_get_column_by_name(row, "id"), &experimentationUid);

        QString experimentationName = AssessmentsModelManager::getStringValueFromColumnName(row, "name");

        QString groupName = AssessmentsModelManager::getStringValueFromColumnName(row, "group_name");

        cass_uint32_t creationDate;
        cass_value_get_uint32(cass_row_get_column_by_name(row, "creation_date"), &creationDate);

        cass_int64_t creationTime;
        cass_value_get_int64(cass_row_get_column_by_name(row, "creation_time"), &creationTime);

        time_t secCreationDateTime = cass_date_time_to_epoch(creationDate, creationTime);
        QDateTime creationDateTime;
        creationDateTime.setSecsSinceEpoch(secCreationDateTime);

        experimentation = new ExperimentationM(experimentationUid, experimentationName, groupName, creationDateTime, nullptr);
    }

    return experimentation;
}

/**
 * @brief Delete the given experimentation from Cassandra DB
 * Also deletes its associated taks, subjects, characteristics and characteristics values
 * @param experimentation
 */
void ExperimentationM::deleteExperimentationFromCassandra(const ExperimentationM& experimentation)
{
    // Delete experimentations associations
    _deleteAllTasksForExperimentation(experimentation);
    _deleteAllSubjectsForExperimentation(experimentation);
    _deleteAllCharacteristicsForExperimentation(experimentation);
    _deleteAllCharacteristicsValuesForExperimentation(experimentation);

    // Delete actual experimentation
    QString queryStr = "DELETE FROM " + ExperimentationM::table + " WHERE id = ?;";
    CassStatement* cassStatement = cass_statement_new(queryStr.toStdString().c_str(), 1);
    cass_statement_bind_uuid(cassStatement, 0, experimentation.getCassUuid());

    // Execute the query or bound statement
    CassFuture* cassFuture = cass_session_execute(AssessmentsModelManager::Instance()->getCassSession(), cassStatement);
    CassError cassError = cass_future_error_code(cassFuture);
    if (cassError == CASS_OK)
    {
        qInfo() << "Experimentation" << experimentation.name() << "has been successfully deleted from the DB";
    }
    else {
        qCritical() << "Could not delete the experimentation" << experimentation.name() << "from the DB:" << cass_error_desc(cassError);
    }

    // Clean-up cassandra objects
    cass_future_free(cassFuture);
    cass_statement_free(cassStatement);
}

/**
 * @brief Delete all tasks associated with the given experimentation
 * @param experimentation
 */
void ExperimentationM::_deleteAllTasksForExperimentation(const ExperimentationM& experimentation)
{
    QString queryStr = "DELETE FROM " + TaskM::table + " WHERE id_experimentation = ?;";
    CassStatement* cassStatement = cass_statement_new(queryStr.toStdString().c_str(), 1);
    cass_statement_bind_uuid(cassStatement, 0, experimentation.getCassUuid());

    // Execute the query or bound statement
    CassFuture* cassFuture = cass_session_execute(AssessmentsModelManager::Instance()->getCassSession(), cassStatement);
    CassError cassError = cass_future_error_code(cassFuture);
    if (cassError == CASS_OK)
    {
        qInfo() << "Tasks from" << experimentation.name() << "has been successfully deleted from the DB";
    }
    else {
        qCritical() << "Could not delete the tasks from" << experimentation.name() << "from the DB:" << cass_error_desc(cassError);
    }

    // Clean-up cassandra objects
    cass_future_free(cassFuture);
    cass_statement_free(cassStatement);
}

/**
 * @brief Delete all subjects associated with the given experimentation
 * @param experimentation
 */
void ExperimentationM::_deleteAllSubjectsForExperimentation(const ExperimentationM& experimentation)
{
    QString queryStr = "DELETE FROM " + CharacteristicM::table + " WHERE id_experimentation = ?;";
    CassStatement* cassStatement = cass_statement_new(queryStr.toStdString().c_str(), 1);
    cass_statement_bind_uuid(cassStatement, 0, experimentation.getCassUuid());

    // Execute the query or bound statement
    CassFuture* cassFuture = cass_session_execute(AssessmentsModelManager::Instance()->getCassSession(), cassStatement);
    CassError cassError = cass_future_error_code(cassFuture);
    if (cassError == CASS_OK)
    {
        qInfo() << "Subjects from" << experimentation.name() << "has been successfully deleted from the DB";
    }
    else {
        qCritical() << "Could not delete the subjects from" << experimentation.name() << "from the DB:" << cass_error_desc(cassError);
    }

    // Clean-up cassandra objects
    cass_future_free(cassFuture);
    cass_statement_free(cassStatement);
}

/**
 * @brief Delete all characteristics associated with the given experimentation
 * @param experimentation
 */
void ExperimentationM::_deleteAllCharacteristicsForExperimentation(const ExperimentationM& experimentation)
{
    QString queryStr = "DELETE FROM " + CharacteristicM::table + " WHERE id_experimentation = ?;";
    CassStatement* cassStatement = cass_statement_new(queryStr.toStdString().c_str(), 1);
    cass_statement_bind_uuid(cassStatement, 0, experimentation.getCassUuid());

    // Execute the query or bound statement
    CassFuture* cassFuture = cass_session_execute(AssessmentsModelManager::Instance()->getCassSession(), cassStatement);
    CassError cassError = cass_future_error_code(cassFuture);
    if (cassError == CASS_OK)
    {
        qInfo() << "Characteristics from" << experimentation.name() << "has been successfully deleted from the DB";
    }
    else {
        qCritical() << "Could not delete the characteristics from" << experimentation.name() << "from the DB:" << cass_error_desc(cassError);
    }

    // Clean-up cassandra objects
    cass_future_free(cassFuture);
    cass_statement_free(cassStatement);
}

/**
 * @brief Delete all tasks characteristics values with the given experimentation
 * @param experimentation
 */
void ExperimentationM::_deleteAllCharacteristicsValuesForExperimentation(const ExperimentationM& experimentation)
{
    QString queryStr = "DELETE FROM " + CharacteristicValueM::table + " WHERE id_experimentation = ?;";
    CassStatement* cassStatement = cass_statement_new(queryStr.toStdString().c_str(), 1);
    cass_statement_bind_uuid(cassStatement, 0, experimentation.getCassUuid());

    // Execute the query or bound statement
    CassFuture* cassFuture = cass_session_execute(AssessmentsModelManager::Instance()->getCassSession(), cassStatement);
    CassError cassError = cass_future_error_code(cassFuture);
    if (cassError == CASS_OK)
    {
        qInfo() << "Characteristics values from" << experimentation.name() << "has been successfully deleted from the DB";
    }
    else {
        qCritical() << "Could not delete the characteristics values from" << experimentation.name() << "from the DB:" << cass_error_desc(cassError);
    }

    // Clean-up cassandra objects
    cass_future_free(cassFuture);
    cass_statement_free(cassStatement);
}


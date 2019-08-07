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

#ifndef TASK_INSTANCE_M_H
#define TASK_INSTANCE_M_H

#include <QObject>
#include <I2PropertyHelpers.h>

#include <model/subject/subjectm.h>
#include <model/task/taskm.h>
#include <model/assessmentsenums.h>


/**
 * @brief The TaskInstanceM class defines a model of task instance
 */
class TaskInstanceM : public QObject
{
    Q_OBJECT

    // Unique identifier of our record
    I2_QML_PROPERTY(QString, uid)

    // Name of our record
    I2_QML_PROPERTY(QString, name)

    // Subject of our record
    I2_QML_PROPERTY_DELETE_PROOF(SubjectM*, subject)

    // Task of our record
    I2_QML_PROPERTY_DELETE_PROOF(TaskM*, task)

    // Start date and time of our record
    I2_QML_PROPERTY(QDateTime, startDateTime)

    // End date and time of our record
    I2_QML_PROPERTY_CUSTOM_SETTER(QDateTime, endDateTime)

    // Duration of our record
    I2_QML_PROPERTY_QTime(duration)
    //I2_QML_PROPERTY(QDateTime, duration)

    // Values of the independent variables of the task
    // "Qml Property Map" allows to set key-value pairs that can be used in QML bindings
    I2_QML_PROPERTY_READONLY(QQmlPropertyMap*, mapIndependentVariableValues)

    // Hash table from a (unique) id of independent variable to the independent variable value
    //I2_QOBJECT_HASHMODEL(QVariant, hashFromIndependentVariableIdToValue)

    // DependentVariableValues (TODO ?): Les valeurs des VD (sorties d’agents)
    // sont stockées avec le temps correspondant au changement de la valeur d’une sortie


public:

    /**
     * @brief Constructor
     * @param uid
     * @param name
     * @param subject
     * @param task
     * @param startDateTime
     * @param parent
     */
    explicit TaskInstanceM(CassUuid experimentationUuid,
                          CassUuid cassUuid,
                          QString name,
                          SubjectM* subject,
                          TaskM* task,
                          QDateTime startDateTime,
                          QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~TaskInstanceM();

    /**
     * @brief TaskInstance table name
     */
    static const QString table;

    /**
     * @brief Accessor for the Cassandra UUID of this entry
     * @return
     */
    CassUuid getCassUuid() const { return _cassUuid; }

    /**
     * @brief Static factory method to create a task instance from a CassandraDB record
     * @param row
     * @return
     */
    static TaskInstanceM* createTaskInstanceFromCassandraRow(const CassRow* row, SubjectM* subject, TaskM* task);

    /**
     * @brief Delete the given task instance from Cassandra DB
     * @param experimentation
     */
    static void deleteTaskInstanceFromCassandra(const TaskInstanceM& taskInstance);

private Q_SLOTS:
    void _onIndependentVariableValueChanged(const QString& key, const QVariant& value);

private:
    /**
     * @brief For debug purpose: Print the value of all independent variables
     */
    void _printIndependentVariableValues();

private:
    QHash<QString, IndependentVariableM*> _mapIndependentVarByName;
    CassUuid _experimentationCassUuid;

    CassUuid _cassUuid;


};

QML_DECLARE_TYPE(TaskInstanceM)

#endif // TASK_INSTANCE_M_H
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

#ifndef TASKM_H
#define TASKM_H

#include <QObject>
#include "I2PropertyHelpers.h"

#include <model/task/dependentvariablem.h>
#include <model/task/independentvariablem.h>


/**
 * @brief The TaskM class defines a model of task
 */
class TaskM : public QObject
{
    Q_OBJECT

    // Name of our task
    I2_QML_PROPERTY(QString, name)

    // URL of the IngeScape platform (JSON file)
    I2_CPP_PROPERTY_CUSTOM_SETTER(QUrl, platformFileUrl)

    // Name of the IngeScape platform (JSON file)
    I2_QML_PROPERTY_READONLY(QString, platformFileName)

    // List of independent variables
    I2_QOBJECT_LISTMODEL(IndependentVariableM, independentVariables)

    // List of dependent variables
    I2_QOBJECT_LISTMODEL(DependentVariableM, dependentVariables)


public:
    /**
     * @brief Constructor
     * @param name
     * @param parent
     */
    explicit TaskM(QString name,
                   QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~TaskM();


    /**
     * @brief Return true if the user can create an independent variable with the name
     * Check if the name is not empty and if a independent variable with the same name does not already exist
     * @param independentVariableName
     * @return
     */
    Q_INVOKABLE bool canCreateIndependentVariableWithName(QString independentVariableName);


Q_SIGNALS:


public Q_SLOTS:


private:

    /**
     * @brief Update the list of dependent variables
     */
    void _updateDependentVariables();


};

QML_DECLARE_TYPE(TaskM)

#endif // TASKM_H

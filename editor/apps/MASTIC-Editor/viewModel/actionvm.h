/*
 *	MASTIC Editor
 *
 *  Copyright © 2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

#ifndef ACTIONVM_H
#define ACTIONVM_H

#include <QObject>
#include <QtQml>
#include <QQmlEngine>
#include <QJSEngine>

#include "I2PropertyHelpers.h"

#include "model/scenario/actionm.h"
#include "viewModel/actionexecutionvm.h"


/**
 * @brief The ActionVM class defines a view model of action
 */
class ActionVM: public QObject
{
    Q_OBJECT

    // Action model
    I2_QML_PROPERTY_READONLY_CUSTOM_SETTER(ActionM*, actionModel)

    // Start time in milliseconds
    I2_QML_PROPERTY(int, startTime)

    // Start date time in string format
    I2_QML_PROPERTY_CUSTOM_SETTER(QString, startTimeString)

    // Color
    I2_QML_PROPERTY(QColor, color)

    // Line number in timeline
    I2_QML_PROPERTY(int, lineInTimeLine)

    // Is valid flag
    I2_QML_PROPERTY(bool, isValid)

    // List of executions of our action
    I2_QOBJECT_LISTMODEL(ActionExecutionVM, executionsList)

    // Current execution (waiting to execute)
    I2_QML_PROPERTY(ActionExecutionVM*, currentExecution)


    // End time in milliseconds (evaluated from the action type)
    // CUSTOM : startime + validation duration
    // FOREVER : -1
    // IMMEDIATE : startime
    I2_QML_PROPERTY(int, endTime)

public:

    /**
     * @brief Constructor
     * @param model
     * @param startTime
     * @param parent
     */
    explicit ActionVM(ActionM* model,
                      int startTime,
                      QObject *parent = 0);


    /**
      * @brief Destructor
      */
    ~ActionVM();


    /**
     * @brief Copy from another action view model
     * @param action VM to copy
     */
    void copyFrom(ActionVM* actionVM);


    /**
     * @brief Notify our action that its effects has been executed
     * @param currentTimeInMilliSeconds
     */
    void effectsExecuted(int currentTimeInMilliSeconds);


    /**
     * @brief Notify our action that its reverse effects has been executed
     * @param currentTimeInMilliSeconds
     */
    void reverseEffectsExecuted(int currentTimeInMilliSeconds);


    /**
     * @brief Delay the current execution of our action
     * @param currentTimeInMilliSeconds
     */
    void delayCurrentExecution(int currentTimeInMilliSeconds);


Q_SIGNALS:


private Q_SLOTS:

    /**
     * @brief Slot when the flag "is valid" changed in the model of action
     * @param isValid flag "is valid"
     */
    void _onIsValidChangedInModel(bool isValid);


    /**
     * @brief Slot on the validity duration change
     * @param validity duration
     */
    void _onValidityDurationChange();


private:
    /**
     * @brief Compute the endTime according to the action model and its type
     */
    void _computeEndTime();


    /**
     * @brief Create a new (view model of) action execution
     * @param startTime relative to our view model of action
     */
    void _createActionExecution(int startTime);


};

QML_DECLARE_TYPE(ActionVM)

#endif // ACTIONVM_H

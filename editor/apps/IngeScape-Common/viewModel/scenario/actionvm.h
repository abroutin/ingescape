/*
 *	IngeScape Editor
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

#include <model/scenario/actionm.h>
#include <viewModel/scenario/actionexecutionvm.h>

#define NB_MILLI_SECONDS_IN_ONE_HOUR 3600000
#define NB_MILLI_SECONDS_IN_ONE_MINUTE 60000


/**
 * @brief The ActionVM class defines a view model of action
 */
class ActionVM: public QObject
{
    Q_OBJECT

    // Model of our view model of action
    I2_QML_PROPERTY_READONLY_CUSTOM_SETTER(ActionM*, modelM)

    // Start time in milliseconds
    I2_QML_PROPERTY(int, startTime)

    // Start date time in string format
    I2_QML_PROPERTY_CUSTOM_SETTER(QString, startTimeString)

    // Color
    I2_QML_PROPERTY(QColor, color)

    // Line number in timeline
    I2_QML_PROPERTY(int, lineInTimeLine)

    // Flag indicating if all conditions of our action are valid
    I2_QML_PROPERTY(bool, areAllConditionsValid)

    // List of executions of our action
    I2_QOBJECT_LISTMODEL(ActionExecutionVM, executionsList)

    // Current execution (waiting to execute)
    I2_QML_PROPERTY(ActionExecutionVM*, currentExecution)

    // End time in milliseconds (evaluated from the action type)
    // CUSTOM : startime + validation duration
    // FOREVER : -1
    // IMMEDIATE : startime
    I2_QML_PROPERTY(int, endTime)

    // Timer to wait for action revert
    I2_CPP_PROPERTY(QTimer*, timerToReverse)

    // Timer to wait for action rearm
    I2_CPP_PROPERTY(QTimer*, timerToRearm)


public:

    /**
     * @brief Constructor
     * @param model
     * @param startTime
     * @param parent
     */
    explicit ActionVM(ActionM* model,
                      int startTime,
                      QObject *parent = nullptr);


    /**
      * @brief Destructor
      */
    ~ActionVM();


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
     * @brief Rearm our action
     * @param currentTimeInMilliSeconds
     */
    void rearmCurrentActionExecution(int currentTimeInMilliSeconds);


    /**
     * @brief Delay the current execution of our action
     * @param currentTimeInMilliSeconds
     */
    void delayCurrentExecution(int currentTimeInMilliSeconds);


    /**
      * @brief Initialize the action view model at a specific time
      * @param time when to initialize the action VM
      */
    void resetDataFrom(int time);


Q_SIGNALS:

    /**
     * @brief Signal emitted when the action has finished its reverse period
     * @param action execution
     */
    void revertAction(ActionExecutionVM* actionExecution);


    /**
     * @brief Signal emitted when the action has finished its rearm period
     */
    void rearmAction();


private Q_SLOTS:

    /**
     * @brief Slot when the flag "is valid" changed in the model of action
     * @param isValid
     */
    void _onIsValidChangedInModel(bool isValid);


    /**
     * @brief Slot called when the validity duration changed and when the type of the validity duration changed
     */
    void _onValidityDurationChanged();


    /**
     * @brief Called when our timer time out to handle the action reversion
     */
    void _onTimeout_ReserseAction();


    /**
     * @brief Called when our timer time out to handle the action rearm
     */
    void _onTimeout_RearmAction();


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
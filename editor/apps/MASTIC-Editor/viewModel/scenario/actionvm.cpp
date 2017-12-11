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

#include "actionvm.h"

#include <QDebug>

/**
 * @brief Constructor
 * @param model
 * @param startTime
 * @param parent
 */
ActionVM::ActionVM(ActionM* model,
                   int startTime,
                   QObject *parent) : QObject(parent),
    _modelM(NULL),
    _startTime(startTime),
    _startTimeString("00:00:00.000"),
    _lineInTimeLine(-1),
    _areConditionsValid(false),
    _currentExecution(NULL),
    _endTime(startTime)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    if (model != NULL)
    {
        if (startTime >= 0)
        {
            int hours = startTime / 3600000;
            int minutes = (startTime - hours*3600000)/60000 ;
            int seconds = (startTime - hours*3600000 - minutes*60000) / 1000;
            int milliseconds = startTime%1000;

            _startTimeString = QString::number(hours).rightJustified(2, '0') + ":" + QString::number(minutes).rightJustified(2, '0') + ":" + QString::number(seconds).rightJustified(2, '0') + "." + QString::number(milliseconds).leftJustified(3, '0');
        }

        // Update valid flag
        setareConditionsValid(model->isValid());

        // Set the action model
        setmodelM(model);

        // Create the first (view model of) action execution
        _createActionExecution(0);
    }

    //
    // Init the revert timer
    //
    _timerToReverse = new QTimer();
    _timerToReverse->setSingleShot(true);
    connect(_timerToReverse, &QTimer::timeout, this, &ActionVM::_onTimeout_ReserseAction);

}


/**
 * @brief Destructor
 */
ActionVM::~ActionVM()
{
    // Reset current execution
    setcurrentExecution(NULL);

    // Delete all executions
    _executionsList.deleteAllItems();

    // Reset model of action
    setmodelM(NULL);

    // Remove timer
    if(_timerToReverse != NULL)
    {
        delete _timerToReverse;
        _timerToReverse = NULL;
    }
}


/**
 * @brief Copy from another action view model
 * @param action VM to copy
 */
void ActionVM::copyFrom(ActionVM* actionVM)
{
    if(actionVM != NULL)
    {
        ActionM* originalModel = actionVM->modelM();

        // Copy the model
        if(originalModel != NULL)
        {
            ActionM* model = new ActionM(originalModel->name());
            model->copyFrom(originalModel);

            setmodelM(model);
        }
        else {
            setmodelM(NULL);
        }

        // Copy the view model attributes
        setcolor(actionVM->color());
        setstartTime(actionVM->startTime());
        setendTime(actionVM->endTime());
        setlineInTimeLine(actionVM->lineInTimeLine());
        setstartTimeString(actionVM->startTimeString());
        setareConditionsValid(actionVM->areConditionsValid());
    }
}


/**
 * @brief Setter for property "Start Time String"
 * @param value
 */
void ActionVM::setstartTimeString(QString value)
{
    if (_startTimeString != value)
    {
        _startTimeString = value;

        if (!_startTimeString.isEmpty())
        {
            bool successSeconds = false;
            bool successMinutes = false;
            bool successHours = false;
            bool successMilliSeconds = false;

            int hours = 0;
            int minutes = 0;
            int seconds = 0;
            int milliSeconds = 0;

            // Split start time HH:MM:SS
            QStringList splittedTime = _startTimeString.split(':');
            if (splittedTime.count() == 3)
            {
                hours = splittedTime.at(0).toInt(&successHours);
                minutes = splittedTime.at(1).toInt(&successMinutes);

                // Try to split SS.zzz
                QStringList splittedTimeSeconds = splittedTime.at(2).split('.');
                if (splittedTimeSeconds.count() == 2)
                {
                    seconds = splittedTimeSeconds.at(0).toInt(&successSeconds);
                    milliSeconds = splittedTimeSeconds.at(1).leftJustified(3, '0').toInt(&successMilliSeconds);
                }
                else {
                    seconds = splittedTime.at(2).toInt(&successSeconds);
                }
            }

            if(successHours && successMinutes && successSeconds)
            {
                int totalMilliseconds = hours*3600000 + minutes*60000 + seconds*1000;
                if(successMilliSeconds)
                {
                    totalMilliseconds += milliSeconds;
                    setstartTime(totalMilliseconds);
                    qDebug() << "Start Time =" << _startTime;
                }
                else {
                    setstartTime(-1);
                    if (_modelM != NULL) {
                        qCritical() << "Wrong 'Start Time':" << _startTimeString << "for action" << _modelM->name();
                    }
                }
            }
            else {
                setstartTime(-1);
                if (_modelM != NULL) {
                    qCritical() << "Wrong 'Start Time':" << _startTimeString << "for action" << _modelM->name();
                }
            }
        }
        else {
            setstartTime(-1);
            if (_modelM != NULL) {
                qCritical() << "Wrong 'Start Time':" << _startTimeString << "for action" << _modelM->name();
            }
        }

        // Compute the new endtime
        _computeEndTime();

        Q_EMIT startTimeStringChanged(value);
    }
}


/**
 * @brief Custom setter for property "modelM"
 * @param value
 */
void ActionVM::setmodelM(ActionM* value)
{
    if (_modelM != value)
    {
        if(_modelM != NULL)
        {
            disconnect(_modelM, &ActionM::isValidChanged, this, &ActionVM::_onIsValidChangedInModel);

            // Endtime reevaluation disconnection
            disconnect(_modelM, &ActionM::validityDurationChanged, this, &ActionVM::_onValidityDurationChange);
            disconnect(_modelM, &ActionM::validityDurationTypeChanged, this, &ActionVM::_onValidityDurationChange);
        }

        _modelM = value;

        if(_modelM != NULL)
        {
            connect(_modelM, &ActionM::isValidChanged, this, &ActionVM::_onIsValidChangedInModel);

            // Endtime reevaluation connection
            connect(_modelM, &ActionM::validityDurationChanged, this, &ActionVM::_onValidityDurationChange);
            connect(_modelM, &ActionM::validityDurationTypeChanged, this, &ActionVM::_onValidityDurationChange);
        }

        // Compute the new endtime
        _computeEndTime();

        Q_EMIT modelMChanged(value);
    }
}


/**
 * @brief Notify our action that its effects has been executed
 * @param currentTimeInMilliSeconds
 */
void ActionVM::effectsExecuted(int currentTimeInMilliSeconds)
{
    if (_currentExecution != NULL)
    {
        _currentExecution->setisExecuted(true);

        // Shall revert
        if (_modelM->shallRevertAfterTime() && _modelM->revertAfterTime() > 0)
        {
            // Launch timer for revert
            _timerToReverse->start(_modelM->revertAfterTime());
        }
        else if (_modelM->shallRevertWhenValidityIsOver())
        {
            // Launch timer for revert
            _timerToReverse->start(_endTime - currentTimeInMilliSeconds);
        }
        // No reverse
        else
        {
            setcurrentExecution(NULL);

            // Shall rearm
            if (_modelM->shallRearm())
            {
                // Create a new (view model of) action execution
                _createActionExecution(currentTimeInMilliSeconds - _startTime);
            }
        }
    }
}


/**
 * @brief Notify our action that its reverse effects has been executed
 * @param currentTimeInMilliSeconds
 */
void ActionVM::reverseEffectsExecuted(int currentTimeInMilliSeconds)
{
    if (_currentExecution != NULL)
    {
        setcurrentExecution(NULL);

        // Shall rearm
        if (_modelM->shallRearm() && (currentTimeInMilliSeconds < _endTime || _endTime == -1))
        {
            // Create a new (view model of) action execution
            _createActionExecution(currentTimeInMilliSeconds - _startTime);
        }
    }
}


/**
 * @brief Delay the current execution of our action
 * @param currentTimeInMilliSeconds
 */
void ActionVM::delayCurrentExecution(int currentTimeInMilliSeconds)
{
    if (_currentExecution != NULL)
    {
        // Add 1 ms
        _currentExecution->setexecutionTime(currentTimeInMilliSeconds - _startTime + 1);

        if ((_modelM != NULL) && _modelM->shallRevert() && _modelM->shallRevertAfterTime())
        {
            // FIXME TODO: check reverseTime is not after validityDuration

            if (_modelM->revertAfterTime() > -1) {
                _currentExecution->setreverseTime(currentTimeInMilliSeconds - _startTime + _modelM->revertAfterTime());
            }
            else {
                qWarning() << "Action" << _modelM->name() << "Shall revert after time but 'revert after time' is not defined";
            }
        }
    }
}


/**
 * @brief Slot when the flag "is valid" changed in the model of action
 * @param isValid flag "is valid"
 */
void ActionVM::_onIsValidChangedInModel(bool areConditionsValid)
{
    setareConditionsValid(areConditionsValid);
}


/**
 * @brief Slot on the validity duration change
 * @param validity duration
 */
void ActionVM::_onValidityDurationChange()
{
    _computeEndTime();
}


/**
 * @brief Compute the endTime according to the action model and its type
 */
void ActionVM::_computeEndTime()
{
    int endTime = _startTime;
    if(_modelM != NULL)
    {
        int itemDurationTime = 0;
        if(_modelM->validityDurationType() == ValidationDurationType::FOREVER)
        {
            endTime = -1;
        }
        else if(_modelM->validityDurationType() == ValidationDurationType::CUSTOM)
        {
            itemDurationTime = _modelM->validityDuration();
        }

        // Compare with the time before revert if selected
        if(_modelM->shallRevertAfterTime() && _modelM->revertAfterTime() > itemDurationTime)
        {
            itemDurationTime = _modelM->revertAfterTime();
        }

        // If not forever, add duration
        if(endTime != -1)
        {
            endTime += itemDurationTime;
        }
    }

    // Set the new value
    setendTime(endTime);
}


/**
 * @brief Create a new (view model of) action execution
 * @param relative to our view model of action
 */
void ActionVM::_createActionExecution(int startTime)
{
    if (_modelM != NULL)
    {
        // No revert by default: reverse time = start time
        int reverseTime = startTime;

        if (_modelM->shallRevert())
        {
            // Shall revert when validity is over
            if (_modelM->shallRevertWhenValidityIsOver())
            {
                if (_modelM->validityDuration() > -1) {
                    reverseTime = startTime + _modelM->validityDuration();
                }
                else {
                    qWarning() << "Action" << _modelM->name() << "Shall revert when validity is over but 'validity duration' is not defined";
                }
            }
            // Shall revert after time
            else if (_modelM->shallRevertAfterTime())
            {
                if (_modelM->revertAfterTime() > -1) {
                    reverseTime = startTime + _modelM->revertAfterTime();
                }
                else {
                    qWarning() << "Action" << _modelM->name() << "Shall revert after time but 'revert after time' is not defined";
                }
            }
        }

        // Create a new (view model of) action execution
        ActionExecutionVM* actionExecution = actionExecution = new ActionExecutionVM(_modelM->shallRevert(), startTime, reverseTime, this);

        // Add to the list
        _executionsList.append(actionExecution);

        // Set as current
        setcurrentExecution(actionExecution);
    }
}

/**
 * @brief Called when our timer time out to handle the action reversion
 */
void ActionVM::_onTimeout_ReserseAction()
{
    if(_currentExecution != NULL)
    {
        // Emit the signal to send the action reversion
        Q_EMIT revertAction(_currentExecution);
    }
}


/**
  * @brief Initialize the action view model at a specific time
  * @param time when to initialize the action VM
  */
void ActionVM::resetDataFrom(int time)
{
    // Update the conditions validation flag
    if(_modelM->isConnected())
    {
        setareConditionsValid(modelM()->isValid());
    } else {
        setareConditionsValid(false);
    }

    // Reset the current action execution
    setcurrentExecution(NULL);

    // Get the relative time to the action view model
    int relativeTime = time - _startTime;

    // Check the actions executions
    foreach (ActionExecutionVM* actionExecution, _executionsList.toList())
    {
        // The action execution is in the future
        if(actionExecution->executionTime() >= relativeTime ||
                (actionExecution->shallRevert()
                                 && ((actionExecution->reverseTime() >= relativeTime))))
        {
            _executionsList.remove(actionExecution);
        }
    }

    // Create the first action execution
    if(time <= _startTime)
    {
        // Create the first (view model of) action execution
        _createActionExecution(0);
    } else {
        // Create one default action execution in the current validation duration
        _createActionExecution(relativeTime);
    }
}
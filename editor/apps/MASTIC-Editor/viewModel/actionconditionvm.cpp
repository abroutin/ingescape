/*
 *	ActionConditionVM
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

#include "actionconditionvm.h"


#include <QDebug>

/**
 * @brief Action condition type
 * @param value
 * @return
 */
QString ActionConditionType::enumToString(int value)
{
    QString string = "Action condition type";

    switch (value) {
    case ActionConditionType::VALUE:
        string = "Value";
        break;

    case ActionConditionType::AGENT:
        string = "Agent";
        break;

    default:
        break;
    }

    return string;
}

//--------------------------------------------------------------
//
// ActionConditionVM
//
//--------------------------------------------------------------


/**
 * @brief Default constructor
 * @param parent
 */
ActionConditionVM::ActionConditionVM(QObject *parent) : QObject(parent),
    _condition(NULL),
    _conditionType(ActionConditionType::AGENT)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}


/**
 * @brief Destructor
 */
ActionConditionVM::~ActionConditionVM()
{
    if(_condition != NULL)
    {
        ActionConditionM* tmp = _condition;
        setcondition(NULL);
        delete tmp;
        tmp = NULL;
    }
}

/**
 * @brief Custom setter on the condition type
 */
void ActionConditionVM::setconditionType(ActionConditionType::Value value)
{
    if(_conditionType != value)
    {
        _conditionType = value;

        // configure the new type
        _configureToType(value);

        emit conditionTypeChanged(value);
    }
}

/**
 * @brief Configure action condition VM into a specific type
 */
void ActionConditionVM::_configureToType(ActionConditionType::Value value)
{
    AgentInMappingVM* agent = NULL;

    // Delete the old condition if exists
    if(_condition != NULL)
    {
        // Save the agent
        agent = _condition->agentModel();

        ActionConditionM* tmp = _condition;
        setcondition(NULL);
        delete tmp;
        tmp = NULL;
    }

    // Create the new type condition
    switch (value)
    {
        case ActionConditionType::AGENT :
        {
            setcondition(new ActionConditionM());
            _condition->setagentModel(agent);
            break;
        }

        case ActionConditionType::VALUE :
        {
            setcondition(new IOPValueConditionM());
            _condition->setagentModel(agent);
            break;
        }

        default:
            break;
    }
}


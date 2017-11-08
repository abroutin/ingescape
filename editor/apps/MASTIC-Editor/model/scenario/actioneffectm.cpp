/*
 *	ActionEffectM
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

#include "actioneffectm.h"


#include <QDebug>

/**
 * @brief Effect type for an action
 * @param value
 * @return
 */
QString ActionEffectType::enumToString(int value)
{
    QString string = "Effect type";

    switch (value) {
    case ActionEffectType::ENABLE:
        string = "Enable";
        break;

    case ActionEffectType::DISABLE:
        string = "Disable";
        break;

    case ActionEffectType::ON:
        string = "On";
        break;

    case ActionEffectType::OFF:
        string = "Off";
        break;

    default:
        break;
    }

    return string;
}

//--------------------------------------------------------------
//
// ActionEffectM
//
//--------------------------------------------------------------


/**
 * @brief Default constructor
 * @param parent
 */
ActionEffectM::ActionEffectM(QObject *parent) : QObject(parent),
    _agentModel(NULL),
    _effect(ActionEffectType::ON)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

}


/**
 * @brief Destructor
 */
ActionEffectM::~ActionEffectM()
{
    // Reset agent model to null
    setagentModel(NULL);
}

/**
* @brief Copy from another effect model
* @param effect to copy
*/
void ActionEffectM::copyFrom(ActionEffectM* effect)
{
    if(effect != NULL)
    {
        setagentModel(effect->agentModel());
        seteffect(effect->effect());
    }
}



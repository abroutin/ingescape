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
 *      Alexandre Lemort   <lemort@ingenuity.io>
 *
 */

#include "inputvm.h"

InputVM::InputVM(QString agentName,
                 AgentIOPM* modelM,
                 QObject *parent) : PointMapVM(agentName,
                                               NULL,
                                               parent),
    _modelM(NULL)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    setmodelM(modelM);

    if (_modelM != NULL) {
        qInfo() << "New Input VM" << _nameAgent << "." << _modelM->name();
    }
}


/**
 * @brief Destructor
 */
InputVM::~InputVM()
{
    if (_modelM != NULL) {
        qInfo() << "Delete Input VM" << _nameAgent << "." << _modelM->name();
    }

    setmodelM(NULL);
}

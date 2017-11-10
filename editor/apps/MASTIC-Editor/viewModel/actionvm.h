/*
 *	ActionVM
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

#ifndef ACTIONVM_H
#define ACTIONVM_H

#include <QObject>
#include <QtQml>
#include <QQmlEngine>
#include <QJSEngine>

#include "I2PropertyHelpers.h"

#include "model/scenario/actionm.h"


/**
 * @brief The ActionVM class defines the main controller of our application
 */
class ActionVM: public QObject
{
    Q_OBJECT

    // Action model
    I2_QML_PROPERTY_READONLY_DELETE_PROOF(ActionM*, actionModel)

    // Start time in seconds
    I2_CPP_PROPERTY(int, startTime)

    // Start date time in string format
    I2_QML_PROPERTY_CUSTOM_SETTER(QString, startTimeString)

    // Color
    I2_QML_PROPERTY(QColor, color)

    // Line number in timeline
    I2_QML_PROPERTY(int, lineInTimeLine)

public:

    /**
     * @brief Default constructor
     * @param parent
     */
    explicit ActionVM(ActionM* actionModel, int startTime, QObject *parent = 0);

    /**
      * @brief Destructor
      */
    ~ActionVM();

    /**
     * @brief Copy from another action view model
     * @param action VM to copy
     */
    void copyFrom(ActionVM* actionVM);

Q_SIGNALS:


public Q_SLOTS:


protected:



};

QML_DECLARE_TYPE(ActionVM)

#endif // ACTIONVM_H

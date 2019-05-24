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

#ifndef INDEPENDENTVARIABLEM_H
#define INDEPENDENTVARIABLEM_H

#include <QObject>
#include <I2PropertyHelpers.h>
#include <model/assessmentsenums.h>


/**
 * @brief The IndependentVariableM class defines a model of independent variable
 */
class IndependentVariableM : public QObject
{
    Q_OBJECT

    // Name of our independent variable
    I2_QML_PROPERTY(QString, name)

    // Description of our independent variable
    I2_QML_PROPERTY(QString, description)

    // Unit of our independent variable
    //I2_QML_PROPERTY(QString, unit)

    // Type of our independent variable value
    I2_QML_PROPERTY(IndependentVariableValueTypes::Value, valueType)

    // List of possible values if the value type is "INDEPENDENT_VARIABLE_ENUM"
    I2_QML_PROPERTY(QStringList, enumValues)


public:
    /**
     * @brief Constructor
     * @param name
     * @param description
     * @param valueType
     * @param parent
     */
    explicit IndependentVariableM(QString name,
                                  QString description,
                                  IndependentVariableValueTypes::Value valueType,
                                  QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~IndependentVariableM();


Q_SIGNALS:


public Q_SLOTS:


private:


};

QML_DECLARE_TYPE(IndependentVariableM)

#endif // INDEPENDENTVARIABLEM_H
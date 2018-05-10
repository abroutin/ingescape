/*
 *  IngeScape - QML binding
 *
 *  Copyright (c) 2018 Ingenuity i/o. All rights reserved.
 *
 *  See license terms for the rights and conditions
 *  defined by copyright holders.
 *
 *
 *  Contributors:
 *      Alexandre Lemort <lemort@ingenuity.io>
 *
 */

#include "ingescapequickabstractiopbinding.h"

#include <QDebug>

#include "ingescapequickbindingsingleton.h"
#include "IngeScapeQuick.h"




//
// List of properties excluded from introspection
//
QList<QString> IngeScapeQuickAbstractIOPBinding::_propertiesExcludedFromIntrospection = QList<QString>()
        << "parent"
        << "objectName"
         ;


//
// List of signals excluded from introspection
//
QList<QString> IngeScapeQuickAbstractIOPBinding::_signalHandlersExcludedFromIntrospection = QList<QString>()
        << "onDestroyed"
        << "onImplicitWidthChanged2"
        << "onImplicitHeightChanged2"
           ;



//
// List of supported types by IOP type
//
// - List of supported types for IngeScapeIopType.INTEGER
QList<QMetaType::Type> IngeScapeQuickAbstractIOPBinding::_supportedTypesForIngeScapeIopTypeInteger = QList<QMetaType::Type>()
        << QMetaType::Int
        << QMetaType::UInt
        << QMetaType::Long
        << QMetaType::LongLong
        << QMetaType::Short
        << QMetaType::ULong
        << QMetaType::ULongLong
        << QMetaType::UShort
         ;

// - List of supported types for IngeScapeIopType.DOUBLE
QList<QMetaType::Type> IngeScapeQuickAbstractIOPBinding::_supportedTypesForIngeScapeIopTypeDouble = QList<QMetaType::Type>()
        << QMetaType::Double
        << QMetaType::Float
         ;




//-------------------------------------------------------------------
//
//
//  IngeScapeQuickAbstractIOPBinding
//
//
//-------------------------------------------------------------------


/**
 * @brief Default constructor
 * @param parent
 * @param qmlPropertiesMustBeWritable
 */
IngeScapeQuickAbstractIOPBinding::IngeScapeQuickAbstractIOPBinding(QObject *parent, bool qmlPropertiesMustBeWritable)
    : QObject(parent),
    // QML properties
    _target(NULL),
    _when(true),
    _removeOnUpdatesAndDestruction(true),
    // Protected properties
    _isCompleted(false),
    _isUsedAsQQmlPropertyValueSource(false),
    _qmlPropertiesMustBeWritable(qmlPropertiesMustBeWritable)
{
}



/**
 * @brief Destructor
 */
IngeScapeQuickAbstractIOPBinding::~IngeScapeQuickAbstractIOPBinding()
{
    // Clear internal data
    clear();

    // Reset our isCompleted flag to be able to use setters
    _isCompleted = false;

    // Reset our target
    if (_target != NULL)
    {
        settarget(NULL);
    }
}



//-------------------------------------------------------------------
//
// Custom setters
//
//-------------------------------------------------------------------


/**
 * @brief Set our target
 * @param value
 */
void IngeScapeQuickAbstractIOPBinding::settarget(QObject *value)
{
    if (_target != value)
    {
        // Unsubscribe to our previous target if needed
        if (_target != NULL)
        {
            disconnect(_target, &QObject::destroyed, this, &IngeScapeQuickAbstractIOPBinding::_ontargetDestroyed);
        }

        // Save our new value
        _target = value;

        // Subscribe to our new target if needed
        if (_target != NULL)
        {
            connect(_target, &QObject::destroyed, this, &IngeScapeQuickAbstractIOPBinding::_ontargetDestroyed);
        }

        // Check if we can use this value
        if (!_isUsedAsQQmlPropertyValueSource)
        {
            // Update our component
            update();
        }
        else
        {
            qmlWarning(this) << "target can not be set when our item is used as a property value source";
        }

        // Notify change
        Q_EMIT targetChanged(value);
    }
}



/**
 * @brief Set our list of properties
 * @param value
 */
void IngeScapeQuickAbstractIOPBinding::setproperties(QString value)
{
    if (_properties != value)
    {
        // Save our new value
        _properties = value;

        // Check if we can use this value
        if (!_isUsedAsQQmlPropertyValueSource)
        {
            // Update our component
            update();
        }
        else
        {
            qmlWarning(this) << "'properties' can not be set when our item is used as a property value source (invalid value: '"
                              << _properties << "' )";
        }

        // Notify change
        Q_EMIT propertiesChanged(value);
    }
}



/**
 * @brief Flag indicating if our binding is active or not
 * @param value
 */
void IngeScapeQuickAbstractIOPBinding::setwhen(bool value)
{
    if (_when != value)
    {
        // Save our new value
        _when = value;

        // Connect or disconnect to IngeScapeQuick
        connectOrDisconnectToIngeScapeQuick();

        // Notify change
        Q_EMIT whenChanged(value);
    }
}



/**
 * @brief Set if we need to remove inpiuts/outputs on updates and destruction
 * @param value
 */
void IngeScapeQuickAbstractIOPBinding::setremoveOnUpdatesAndDestruction(bool value)
{
    if (_removeOnUpdatesAndDestruction != value)
    {
        // Save our new value
        _removeOnUpdatesAndDestruction = value;

        // Notify change
        Q_EMIT removeOnUpdatesAndDestructionChanged(value);
    }
}



//-------------------------------------------------------------------
//
// Protected slots
//
//-------------------------------------------------------------------


/**
 * @brief Called when the object associated to our target property is destroyed
 * @param sender
 */
void IngeScapeQuickAbstractIOPBinding::_ontargetDestroyed(QObject *sender)
{
    Q_UNUSED(sender)

    // Reset our target
    _target = NULL;

    // Update our component
    update();

    // Notify that our target is destroyed
    Q_EMIT targetChanged(NULL);
}




//-------------------------------------------------------------------
//
// Protected methods
//
//-------------------------------------------------------------------



/**
 * @brief QQmlPropertyValueSource API: This method will be called by the QML engine when assigning a value source
 *        with the following syntax: IngeScapeInputBinding on property { } or IngeScapeOutputBinding on property { }
 *
 * @param property
 */
void IngeScapeQuickAbstractIOPBinding::setTarget(const QQmlProperty &property)
{
    // Our component is used as a property value source
    _isUsedAsQQmlPropertyValueSource = true;

    // Check if we have a property
    if (property.isProperty())
    {
        // Check if we need a writable property
        if (!_qmlPropertiesMustBeWritable || property.isWritable())
        {
            // Check if the type of our property is supported
            if (checkIfPropertyIsSupported(property))
            {
                // Save this property
                _propertyValueSourceTarget = property;
            }
            else
            {
                // Reset value
                _propertyValueSourceTarget = QQmlProperty();

                qmlWarning(this) << "property '" << property.name() << "' has type '"
                                 << prettyPropertyTypeName(property)
                                 << "' that is not supported by IngeScapeQuick";
            }
        }
        else
        {
            // Reset value
            _propertyValueSourceTarget = QQmlProperty();

            qmlWarning(this) << "can only be associated to a writable property - " << property.name() << " is read-only";
        }
    }
    else
    {
        // Reset value
        _propertyValueSourceTarget = QQmlProperty();

        qmlWarning(this) << "can only be associated to a property. " << property.name() << " is not a property";
    }

    // Update our component
    update();
}



/**
 * @brief QQmlParserStatus API: Invoked after class creation, but before any properties have been set
 */
void IngeScapeQuickAbstractIOPBinding::classBegin()
{
}


/**
 * @brief QQmlParserStatus API: Invoked after the root component that caused this instantiation has completed construction.
 *        At this point all static values and binding values have been assigned to the class.
 */
void IngeScapeQuickAbstractIOPBinding::componentComplete()
{
    // Check if everything is ok
    if (_isUsedAsQQmlPropertyValueSource)
    {
        // Check if "properties" is empty
        if (!_properties.isEmpty())
        {
            qmlWarning(this) << "'properties' can not be set when our item is used as a property value source (invalid value: '"
                             << _properties << "' )";
        }

        // Check if "target" is NULL
        if (_target != NULL)
        {
            qmlWarning(this) << "target can ot be set when our item is used as a property value source";
        }
    }

    // Our component is completed
    _isCompleted = true;


    // Update internal data
    update();
}



/**
 * @brief Get the pretty type name of a given object
 * @param object
 * @return
 */
QString IngeScapeQuickAbstractIOPBinding::prettyObjectTypeName(QObject* object)
{
   QString result;

   if (object != NULL)
   {
       if (object->metaObject() != NULL)
       {
           result = object->metaObject()->className();
       }
       else if (!object->objectName().isEmpty())
       {
           result = QString("objectName=%1").arg(object->objectName());
       }
       else
       {
           result = "UNKNOWN_CLASS";
       }
   }

   return result;
}



/**
 * @brief Check if a given property is supported by IngeScape
 * @param qmlProperty
 * @return
 */
bool IngeScapeQuickAbstractIOPBinding::checkIfPropertyIsSupported(const QQmlProperty &qmlProperty)
{
   bool result = false;

   // Check if we have a property
   if (qmlProperty.isProperty())
   {
       // Check its type category
       if (qmlProperty.propertyTypeCategory() == QQmlProperty::Normal)
       {
           // Read value to check if we can convert its type to a supported type
           QVariant value = qmlProperty.read();
           if (
               value.canConvert<int>()
               ||
               value.canConvert<QString>()
               ||
               value.canConvert<double>()
               ||
               value.canConvert<bool>()
               )
           {
               result = true;
           }
           // Else: not yet implemented or not supported
       }
       // Else: not yet implemented or not supported (QQmlProperty::Object, QQmlProperty::List)
   }
   // Else: invalid property or signal => nothing to do

   return result;
}



/**
 * @brief Get the pretty name of a given property
 * @param property
 * @return
 */
QString IngeScapeQuickAbstractIOPBinding::prettyPropertyTypeName(const QQmlProperty &property)
{
    QString result;

    // Check if we have a QVariant i.e. a generic container type
    if (property.propertyType() == QMetaType::QVariant)
    {
        // Read our value to get our real type
        QVariant value = property.read();
        if (value.type() == QVariant::Invalid)
        {
            result = QString("QVariant::Invalid");
        }
        else
        {
            result = QString("QVariant::%1").arg(value.typeName());
        }
    }
    else
    {
        result = property.propertyTypeName();
    }

    return result;
}



/**
 * @brief Get the IngeScapeIopType of a given property
 * @param property
 * @return
 *
 * @remarks we assume that checkIfPropertyIsSupported has been called before using this method
 */
IngeScapeIopType::Value IngeScapeQuickAbstractIOPBinding::getIngeScapeIOPTypeForProperty(const QQmlProperty &property)
{
   IngeScapeIopType::Value result = IngeScapeIopType::INVALID;

   // Ensure that we have a property
   if (property.type() == QQmlProperty::Property)
   {
       QMetaType::Type propertyType = static_cast<QMetaType::Type>(property.propertyType());

       if (propertyType == QMetaType::Bool)
       {
           result = IngeScapeIopType::BOOLEAN;
       }
       else if (_supportedTypesForIngeScapeIopTypeDouble.contains(propertyType))
       {
           result = IngeScapeIopType::DOUBLE;
       }
       else if (_supportedTypesForIngeScapeIopTypeInteger.contains(propertyType))
       {
           result = IngeScapeIopType::INTEGER;
       }
       else
       {
           // Read value to check if we can convert its type to QString
           QVariant value = property.read();
           if (value.canConvert<QString>())
           {
               result = IngeScapeIopType::STRING;
           }
       }
   }
   // Else: It is not a valid property

   return result;
}



/**
 * @brief Update our component
 *
 * @remarks this function will call _updateInternalData()
 */
void IngeScapeQuickAbstractIOPBinding::update()
{
    // Ensure that our component is completed
    if (_isCompleted)
    {
        // Clean-up previous data
        clear();


        //
        // Check if our item is used as as property value source or a standard QML item
        //
        if (_isUsedAsQQmlPropertyValueSource)
        {
            //
            // Property value source
            //

            // Add our QML property to our hashtable if needed
            if (_propertyValueSourceTarget.isValid())
            {
                _qmlPropertiesByName.insert( _propertyValueSourceTarget.name(), _propertyValueSourceTarget);
            }
        }
        else
        {
            //
            // Standard QML item
            //

            // Check if we have a target and a list of properties
            QString properties = _properties.trimmed();
            if ((_target != NULL) && !properties.isEmpty())
            {
                IngeScapeQuickBindingSingleton* ingescapeQuickBindingSingleton = IngeScapeQuickBindingSingleton::instance();

                // Check if its a keyword of IngeScapeQuickBindingSingleton
                if ((ingescapeQuickBindingSingleton != NULL) && ingescapeQuickBindingSingleton->isKeyword(properties))
                {
                    if (properties == ingescapeQuickBindingSingleton->AllProperties())
                    {
                        _buildListOfQmlPropertiesByIntrospection(_target, "");
                    }
                    else if (properties == ingescapeQuickBindingSingleton->None())
                    {
                        // No property required => nothing to do
                    }
                    else
                    {
                        qmlWarning(this) << "invalid value 'IngeScapeBinding." << ingescapeQuickBindingSingleton->getKeyword(properties)
                                          << "' for 'properties'";
                    }
                }
                else
                {
                    //
                    // Specific set of properties
                    //

                    // Parse our list of properties
                    QStringList listOfPropertyNames = properties.split(QLatin1Char(','));
                    int numberOfPropertyNames = listOfPropertyNames.count();
                    for (int index = 0; index < numberOfPropertyNames; index++)
                    {
                        // Clean-up the name of the current property
                        QString propertyName = listOfPropertyNames.at(index).trimmed();

                        // Create a QML property
                        QQmlProperty qmlProperty = QQmlProperty(_target, propertyName);

                        // Check if this property exists
                        if (qmlProperty.isValid() && qmlProperty.isProperty())
                        {
                            // Check if we need a writable property
                            if (!_qmlPropertiesMustBeWritable || qmlProperty.isWritable())
                            {
                                // Check if the type of our property is supported
                                if (checkIfPropertyIsSupported(qmlProperty))
                                {
                                    // Save property
                                    _qmlPropertiesByName.insert(propertyName, qmlProperty);
                                }
                                else
                                {
                                    qmlWarning(this) << "property '" << propertyName << "' on "
                                                     << prettyObjectTypeName(_target)
                                                     << " has type '" << prettyPropertyTypeName(qmlProperty)
                                                     << "' that is not supported by IngeScapeQuick";

                                }
                            }
                            else
                            {
                                qmlWarning(this) << "can only be associated to a writable property - property '" << propertyName << "' on "
                                                 << prettyObjectTypeName(_target) << " is read-only";
                            }
                        }
                        else
                        {
                            qmlWarning(this) << "property '" << propertyName << "' does not exist on "
                                             << prettyObjectTypeName(_target);
                        }
                    }
                }
            }
            // Else: no target or no properties => nothing to do
        }


        //
        // Do something with our list of QML properties
        //
        _updateInternalData();


        //
        // Connect to IngeScapeQuick if needed
        //
        if (_when)
        {
            _connectToIngeScapeQuick();
        }
    }
    // Else: our component is not completed yet, we do nothing to avoid useless computations
}



/**
 * @brief Clear our component
 *
 * @remarks this function will call _clearInternalData();
 */
void IngeScapeQuickAbstractIOPBinding::clear()
{
    // Unsubscribe to IngeScapeQuick
    _disconnectToIngeScapeQuick();

    // Clear additional data
    _clearInternalData();

    // Clear our own data
    _qmlPropertiesByName.clear();
}



/**
 * @brief Manage connect/disconnect calls to associate our item to IngeScapeQuick
 */
void IngeScapeQuickAbstractIOPBinding::connectOrDisconnectToIngeScapeQuick()
{
    // Check if component is completed
    if (_isCompleted)
    {
        if (_when)
        {
            _connectToIngeScapeQuick();
        }
        else
        {
            _disconnectToIngeScapeQuick();
        }
    }
    // Else: our component is not completed, we do nothing to avoid useless computations
}



/**
 * @brief Connect to IngeScapeQuick
 */
void IngeScapeQuickAbstractIOPBinding::_connectToIngeScapeQuick()
{
}



/**
 * @brief Disconnect to IngeScapeQuick
 */
void IngeScapeQuickAbstractIOPBinding::_disconnectToIngeScapeQuick()
{
}



/**
 * @brief Clear internal data
 */
void IngeScapeQuickAbstractIOPBinding::_clearInternalData()
{
}



/**
 * @brief Update internal data
 */
void IngeScapeQuickAbstractIOPBinding::_updateInternalData()
{
}



/**
 * @brief Build our list of QML properties by introspection
 * @param object
 * @param prefix
 */
void IngeScapeQuickAbstractIOPBinding::_buildListOfQmlPropertiesByIntrospection(QObject* object, QString prefix)
{
    // Check if we have an object
    if (object != NULL)
    {
        // Get meta object of our target
        const QMetaObject* metaObject = object->metaObject();
        if (metaObject != NULL)
        {
            // Get the number of properties
            int numberOfProperties = metaObject->propertyCount();
            for (int index = 0; index < numberOfProperties; index++)
            {
                // Get meta property
                QMetaProperty metaProperty = metaObject->property(index);
                QString propertyName(metaProperty.name());

                // Check if this property is excluded or not
                if (!_propertiesExcludedFromIntrospection.contains(propertyName))
                {
                    // Create a QML property
                    QQmlProperty qmlProperty = QQmlProperty(object, propertyName);

                    // Check if we can use this property (first approach)
                    bool writableRequirements = (!_qmlPropertiesMustBeWritable || qmlProperty.isWritable());
                    bool typeIsSupported = checkIfPropertyIsSupported(qmlProperty);
                    if (writableRequirements && typeIsSupported)
                    {
                        // Save property
                        _qmlPropertiesByName.insert(prefix + propertyName, qmlProperty);
                    }
                    else
                    {
                        // Either our property is not writable (and we require wreitable properties) o
                        // or this type is not supported

                        /*
                        if (!typeIsSupported)
                        {
                            qmlWarning(this) << "property '" << propertyName << "' on "
                                             << prettyObjectTypeName(_target)
                                             << " has type '" << prettyPropertyTypeName(qmlProperty)
                                             << "' that is not supported by IngeScapeQuick";
                        }
                        */


                        // Check if this property references a non-writable object
                        if ((qmlProperty.propertyTypeCategory() == QQmlProperty::Object) && !qmlProperty.isWritable())
                        {
                            QVariant qmlValue = qmlProperty.read();
                            QObject* objectValue = qmlValue.value<QObject*>();
                            if (objectValue != NULL)
                            {
                                QString newPrefix = (prefix.isEmpty() ? QString("%1.").arg(propertyName) : QString("%1.%2.").arg(prefix, propertyName));

                                _buildListOfQmlPropertiesByIntrospection(objectValue, newPrefix);
                            }
                        }

                    }
                }
                // Else: property is excluded to avoid side-effects
            }
            // End  for (int index = 0; index < numberOfProperties; index++)
        }
        // Else: should not happen because a QObject always has a meta object
    }
    // Else: NULL reference => nothing to do
}



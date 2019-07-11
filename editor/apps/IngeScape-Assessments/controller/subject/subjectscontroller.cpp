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

#include "subjectscontroller.h"


/**
 * @brief Constructor
 * @param modelManager
 * @param parent
 */
SubjectsController::SubjectsController(AssessmentsModelManager* modelManager,
                                       QObject *parent) : QObject(parent),
    _currentExperimentation(nullptr),
    _modelManager(modelManager)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    qInfo() << "New Subjects Controller";

    // Fill without type "UNKNOWN"
    _allCharacteristicValueTypes.fillWithAllEnumValues();
    _allCharacteristicValueTypes.removeEnumValue(CharacteristicValueTypes::UNKNOWN);

}


/**
 * @brief Destructor
 */
SubjectsController::~SubjectsController()
{
    qInfo() << "Delete Subjects Controller";

    // Reset the model of the current experimentation
    if (_currentExperimentation != nullptr)
    {
        setcurrentExperimentation(nullptr);
    }

    /*if (_modelManager != nullptr)
    {
        //disconnect(_modelManager, nullptr, this, nullptr);

        _modelManager = nullptr;
    }*/
}


/**
 * @brief Setter for property "Current Experimentation"
 * @param value
 */
void SubjectsController::setcurrentExperimentation(ExperimentationM *value)
{
    if (_currentExperimentation != value)
    {
        _currentExperimentation = value;

        // Manage changes
        _onCurrentExperimentationChanged(_currentExperimentation);

        Q_EMIT currentExperimentationChanged(value);
    }
}


/**
 * @brief Return true if the user can create a characteristic with the name
 * Check if the name is not empty and if a characteristic with the same name does not already exist
 * @param characteristicName
 * @return
 */
bool SubjectsController::canCreateCharacteristicWithName(QString characteristicName)
{
    if (!characteristicName.isEmpty() && (_currentExperimentation != nullptr))
    {
        for (CharacteristicM* characteristic : _currentExperimentation->allCharacteristics()->toList())
        {
            if ((characteristic != nullptr) && (characteristic->name() == characteristicName))
            {
                return false;
            }
        }
        return true;
    }
    else {
        return false;
    }
}


/**
 * @brief Create a new characteristic
 * @param characteristicName
 * @param nCharacteristicValueType
 */
void SubjectsController::createNewCharacteristic(QString characteristicName, int nCharacteristicValueType)
{
    if (!characteristicName.isEmpty() && (nCharacteristicValueType > -1) && (_currentExperimentation != nullptr))
    {
        // FIXME TODO createNewCharacteristic

        /*CharacteristicValueTypes::Value characteristicValueType = static_cast<CharacteristicValueTypes::Value>(nCharacteristicValueType);

        //qInfo() << "Create new characteristic" << characteristicName << "of type" << CharacteristicValueTypes::staticEnumToString(characteristicValueType);

        // Create the new characteristic
        CharacteristicM* characteristic = new CharacteristicM(characteristicName, characteristicValueType);

        // Add the characteristic to the current experimentation
        _currentExperimentation->addCharacteristic(characteristic);*/
    }
}


/**
 * @brief Create a new characteristic of type enum
 * @param characteristicName
 * @param enumValues
 */
void SubjectsController::createNewCharacteristicEnum(QString characteristicName, QStringList enumValues)
{
    if (!characteristicName.isEmpty() && !enumValues.isEmpty() && (_currentExperimentation != nullptr))
    {
        // FIXME TODO createNewCharacteristicEnum

        //qInfo() << "Create new characteristic" << characteristicName << "of type" << CharacteristicValueTypes::staticEnumToString(CharacteristicValueTypes::CHARACTERISTIC_ENUM) << "with values:" << enumValues;

        /*// Create the new characteristic
        CharacteristicM* characteristic = new CharacteristicM(characteristicName, CharacteristicValueTypes::CHARACTERISTIC_ENUM);
        characteristic->setenumValues(enumValues);

        // Add the characteristic to the current experimentation
        _currentExperimentation->addCharacteristic(characteristic);*/
    }
}


/**
 * @brief Delete a characteristic
 * @param characteristic
 */
void SubjectsController::deleteCharacteristic(CharacteristicM* characteristic)
{
    if ((characteristic != nullptr) && (_currentExperimentation != nullptr))
    {
        // Remove the characteristic from the current experimentation
        _currentExperimentation->removeCharacteristic(characteristic);

        // Free memory
        delete characteristic;
    }
}


/**
 * @brief Create a new subject
 */
void SubjectsController::createNewSubject()
{
    if (_currentExperimentation != nullptr)
    {
        QDateTime now = QDateTime::currentDateTime();

        QString subjectUID = now.toString("S-yyMMdd-hhmmss-zzz");

        // Create a new subject
        SubjectM* subject = new SubjectM(subjectUID, nullptr);

        // For each existing characteristic
        for (CharacteristicM* characteristic : _currentExperimentation->allCharacteristics()->toList())
        {
            if (characteristic != nullptr)
            {
                subject->addCharacteristic(characteristic);
            }
        }

        // Add the subject to the current experimentation
        _currentExperimentation->addSubject(subject);
    }
}


/**
 * @brief Delete a subject
 * @param subject
 */
void SubjectsController::deleteSubject(SubjectM* subject)
{
    if ((subject != nullptr) && (_currentExperimentation != nullptr))
    {
        // Remove the subject from the current experimentation
        _currentExperimentation->removeSubject(subject);

        // Free memory
        delete subject;
    }
}


/**
 * @brief Slot called when the current experimentation changed
 * @param currentExperimentation
 */
void SubjectsController::_onCurrentExperimentationChanged(ExperimentationM* currentExperimentation)
{
    if ((currentExperimentation != nullptr) && (_modelManager != nullptr))
    {
        qDebug() << "Subjects Controller: on Current Experimentation changed" << currentExperimentation->name();

        // FIXME TODO: load data about this experimentation (subjects and characteristics)

        CassUuid uidExperimentation = currentExperimentation->getCassUuid();

        // Create the query
        QString queryGetCharacteristics = QString("SELECT * FROM ingescape.characteristic WHERE id_experimentation = ?;");

        // Creates the new query statement
        CassStatement* cassStatementGetCharacteristics = cass_statement_new(queryGetCharacteristics.toStdString().c_str(), 1);
        cass_statement_bind_uuid(cassStatementGetCharacteristics, 0, uidExperimentation);

        // Execute the query or bound statement
        CassFuture* cassFutureGetCharacteristics = cass_session_execute(_modelManager->getCassSession(), cassStatementGetCharacteristics);

        CassError cassErrorGetCharacteristics = cass_future_error_code(cassFutureGetCharacteristics);
        if (cassErrorGetCharacteristics == CASS_OK)
        {
            qDebug() << "Get all characteristics succeeded";

            // Retrieve result set and iterate over the rows
            const CassResult* cassResult = cass_future_get_result(cassFutureGetCharacteristics);

            if (cassResult != nullptr)
            {
                size_t characteristicsNumber = cass_result_row_count(cassResult);
                if (characteristicsNumber == 0)
                {
                    qDebug() << "There is NO characteristic...create the special characteristic 'id'";

                    CassUuid characteristicId;
                    cass_uuid_gen_time(_modelManager->getCassUuidGen(), &characteristicId);

                    QString characteristicName = "Id";
                    CharacteristicValueTypes::Value characteristicValueType = CharacteristicValueTypes::TEXT;

                    // Create the query
                    QString queryInsertCharacteristic = QString("INSERT INTO ingescape.characteristic (id, id_experimentation, name, value_type, enum_values) VALUES (?, ?, ?, ?, ?);");

                    //id timeuuid, id_experimentation timeuuid, name text, value_type tinyint, enum_values text

                    // Creates the new query statement
                    CassStatement* cassStatementInsertCharacteristic = cass_statement_new(queryInsertCharacteristic.toStdString().c_str(), 5);
                    cass_statement_bind_uuid(cassStatementInsertCharacteristic, 0, characteristicId);
                    cass_statement_bind_uuid(cassStatementInsertCharacteristic, 1, uidExperimentation);
                    cass_statement_bind_string(cassStatementInsertCharacteristic, 2, characteristicName.toStdString().c_str());
                    cass_statement_bind_int8(cassStatementInsertCharacteristic, 3, characteristicValueType);
                    cass_statement_bind_string(cassStatementInsertCharacteristic, 4, "");

                    // Execute the query or bound statement
                    CassFuture* cassFutureInsertCharacteristic = cass_session_execute(_modelManager->getCassSession(), cassStatementInsertCharacteristic);

                    CassError cassErrorInsertCharacteristic = cass_future_error_code(cassFutureInsertCharacteristic);
                    if (cassErrorInsertCharacteristic == CASS_OK)
                    {
                        qInfo() << "CharacteristicM" << characteristicName << "inserted into the DataBase";

                        // Create the new characteristic
                        CharacteristicM* characteristic = new CharacteristicM(characteristicId, characteristicName, characteristicValueType, true, nullptr);

                        // Add the characteristic to the current experimentation
                        _currentExperimentation->addCharacteristic(characteristic);
                    }
                    else {
                        qCritical() << "Could not insert the experimentation" << characteristicName << "into the DataBase:" << cass_error_desc(cassErrorInsertCharacteristic);
                    }

                    cass_statement_free(cassStatementInsertCharacteristic);
                    cass_future_free(cassFutureInsertCharacteristic);
                }
                else
                {
                    qDebug() << "There are" << characteristicsNumber << "characteristics";

                    CassIterator* cassIterator = cass_iterator_from_result(cassResult);

                    while(cass_iterator_next(cassIterator))
                    {
                        //const CassRow* row = cass_iterator_get_row(cassIterator);

                    }

                    cass_iterator_free(cassIterator);
                }
            }
        }
        else {
            qCritical() << "Could not get all characteristics from the DataBase:" << cass_error_desc(cassErrorGetCharacteristics);
        }
    }
}


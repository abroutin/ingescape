/*
 *	IngeScape Measuring
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

import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import I2Quick 1.0

import INGESCAPE 1.0


I2PopupBase {
    id: rootPopup

    height: 500
    width: 500

    anchors.centerIn: parent

    isModal: true
    dismissOnOutsideTap: false
    keepRelativePositionToInitialParent: false


    //--------------------------------------------------------
    //
    //
    // Properties
    //
    //
    //--------------------------------------------------------

    property SubjectsController controller: null;

    property ExperimentationM experimentation: null;

    //property CharacteristicValueTypes selectedType: null;
    property int selectedType: -1;

    property var enumTexts: [];


    //--------------------------------
    //
    //
    // Signals
    //
    //
    //--------------------------------

    //
    //signal cancelTODO();


    //--------------------------------
    //
    //
    // Slots
    //
    //
    //--------------------------------

    onOpened: {

    }



    //--------------------------------
    //
    //
    // Functions
    //
    //
    //--------------------------------

    //
    // Reset all user inputs and close the popup
    //
    function resetInputsAndClosePopup() {
        //console.log("QML: Reset all user inputs and close popup");

        // Reset all user inputs
        txtCharacteristicName.text = "";
        rootPopup.selectedType = -1;
        radioExistingEnum.checked = true;
        spinBoxValuesNumber.value = 2;
        enumTexts = [];

        // Close the popup
        rootPopup.close();
    }


    //--------------------------------
    //
    // Content
    //
    //--------------------------------

    Rectangle {

        anchors {
            fill: parent
        }
        radius: 5
        border {
            width: 2
            color: IngeScapeTheme.editorsBackgroundBorderColor
        }
        color: IngeScapeTheme.editorsBackgroundColor


        Text {
            id: title

            anchors {
                left: parent.left
                top: parent.top
                leftMargin: 20
                topMargin: 20
            }
            height: 25

            //horizontalAlignment: Text.AlignHCenter

            text: qsTr("New characteristic:")

            color: IngeScapeTheme.whiteColor
            font {
                family: IngeScapeTheme.textFontFamily
                pixelSize: 16
            }
        }

        Row {
            id: rowName

            anchors {
                top: title.bottom
                topMargin: 50
                left: parent.left
                leftMargin: 10
            }

            spacing: 10

            Text {
                text: qsTr("Name:")
                height: 30

                color: IngeScapeTheme.whiteColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    weight : Font.Medium
                    pixelSize : 16
                }
            }

            TextField {
                id: txtCharacteristicName

                height: 30
                width: 250 // parent.width

                //verticalAlignment: TextInput.AlignVCenter
                text: ""

                style: I2TextFieldStyle {
                    backgroundColor: IngeScapeTheme.darkBlueGreyColor
                    borderColor: IngeScapeTheme.whiteColor
                    borderErrorColor: IngeScapeTheme.redColor
                    radiusTextBox: 1
                    borderWidth: 0;
                    borderWidthActive: 1
                    textIdleColor: IngeScapeTheme.whiteColor;
                    textDisabledColor: IngeScapeTheme.darkGreyColor

                    padding.left: 3
                    padding.right: 3

                    font {
                        pixelSize:15
                        family: IngeScapeTheme.textFontFamily
                    }
                }

                //Binding {
                //    target: txtLogFilePath
                //    property: "text"
                //    value: rootItem.agent.logFilePath
                //}

                /*onTextChanged: {
                    console.log("onTextChanged " + txtCharacteristicName.text);
                }*/
            }
        }

        Item {
            anchors {
                top: rowName.bottom
                topMargin: 20
                left: parent.left
                leftMargin: 10
                right: parent.right
                rightMargin: 10
            }

            Text {
                id: txtTypesTitle

                anchors {
                    left: parent.left
                    top: parent.top
                }

                text: qsTr("Type:")

                color: "white"
            }

            Column {
                id: columnTypes

                anchors {
                    top: parent.top
                    left: txtTypesTitle.right
                    leftMargin: 10
                    //right: parent.right
                }
                width: 150

                spacing: 10

                ExclusiveGroup {
                    id: exclusiveGroupTypes
                }

                Repeater {
                    model: controller ? controller.allCharacteristicValueTypes : null

                    delegate: RadioButton {
                        id: radioCharacteristicValueType

                        text: model.name

                        exclusiveGroup: exclusiveGroupTypes

                        checked: ((rootPopup.selectedType > -1) && (rootPopup.selectedType === model.value))

                        onCheckedChanged: {
                            if (checked) {
                                console.log("Select Characteristic Value Type: " + model.name + " (" + model.value + ")");

                                rootPopup.selectedType = model.value;
                            }
                        }

                        Binding {
                            target: radioCharacteristicValueType
                            property: "checked"
                            value: ((rootPopup.selectedType > -1) && (rootPopup.selectedType === model.value))
                        }
                    }
                }
            }


            // FIXME TODO
            /*Loader {
                id: loaderEnum
            }*/

            //
            // We cannot use a TabView because ids in its sub tree is not known
            //
            Rectangle {
                anchors {
                    top: parent.top
                    left: columnTypes.right
                    right: parent.right
                }
                height: 250

                // Selected type is "Enum"
                visible: (rootPopup.selectedType === CharacteristicValueTypes.CHARACTERISTIC_ENUM)

                color: "transparent"
                border {
                    color: "white"
                    width: 1
                }

                Row {
                    id: tabs

                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 10

                    ExclusiveGroup {
                        id: tabsExclusiveGroup
                    }

                    RadioButton {
                        id: radioExistingEnum

                        text: "Existing Enum"

                        exclusiveGroup: tabsExclusiveGroup
                    }

                    RadioButton {
                        id: radioNewEnum

                        text: "New Enum"

                        exclusiveGroup: tabsExclusiveGroup
                    }
                }

                Item {
                    id: contentNewEnum

                    anchors {
                        top: tabs.bottom
                        topMargin: 10
                        left: parent.left
                        leftMargin: 5
                    }

                    visible: radioNewEnum.checked

                    Row {
                        id: headerNewEnum

                        spacing: 10

                        Text {
                            text: "Number of values:"
                        }

                        SpinBox {
                            id: spinBoxValuesNumber

                            value: 2
                        }
                    }

                    Column {
                        anchors {
                            top: headerNewEnum.bottom
                            topMargin: 5
                        }

                        Repeater {
                            model: spinBoxValuesNumber.value

                            delegate: TextField {
                                id: enumText

                                width: 200

                                text: rootPopup.enumTexts[index] ? rootPopup.enumTexts[index] : ""

                                Component.onCompleted: {
                                    // If this index is not defined, initialize it with empty string
                                    if (typeof rootPopup.enumTexts[index] === 'undefined') {
                                        rootPopup.enumTexts[index] = "";
                                    }
                                }

                                onTextChanged: {
                                    //console.log(index + ": text changed to " + enumText.text);

                                    // Update the strings array for this index
                                    rootPopup.enumTexts[index] = enumText.text;
                                }
                            }
                        }
                    }
                }

            }
        }


        Row {
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom : parent.bottom
                bottomMargin: 16
            }
            spacing : 15

            Button {
                id: cancelButton

                property var boundingBox: IngeScapeTheme.svgFileINGESCAPE.boundsOnElement("button");

                anchors {
                    verticalCenter: parent.verticalCenter
                }

                height: boundingBox.height
                width: boundingBox.width

                activeFocusOnPress: true
                text: "Cancel"

                style: I2SvgButtonStyle {
                    fileCache: IngeScapeTheme.svgFileINGESCAPE

                    pressedID: releasedID + "-pressed"
                    releasedID: "button"
                    disabledID: releasedID + "-disabled"

                    font {
                        family: IngeScapeTheme.textFontFamily
                        weight : Font.Medium
                        pixelSize : 16
                    }
                    labelColorPressed: IngeScapeTheme.blackColor
                    labelColorReleased: IngeScapeTheme.whiteColor
                    labelColorDisabled: IngeScapeTheme.whiteColor

                }

                onClicked: {
                    //console.log("QML: cancel");

                    // Reset all user inputs and close the popup
                    rootPopup.resetInputsAndClosePopup();
                }
            }

            Button {
                id: okButton

                property var boundingBox: IngeScapeTheme.svgFileINGESCAPE.boundsOnElement("button");

                anchors {
                    verticalCenter: parent.verticalCenter
                }

                height: boundingBox.height
                width: boundingBox.width

                activeFocusOnPress: true
                text: "OK"

                /*enabled: ((txtCharacteristicName.text.length > 0) && controller && controller.newGroup
                          && ( (rootPopup.selectedExperimentationsGroup !== controller.newGroup)
                               || controller.canCreateExperimentationsGroupWithName(txtNewExperimentationsGroupName.text) ) )*/
                enabled: ((txtCharacteristicName.text.length > 0) && (rootPopup.selectedType > -1))

                style: I2SvgButtonStyle {
                    fileCache: IngeScapeTheme.svgFileINGESCAPE

                    pressedID: releasedID + "-pressed"
                    releasedID: "button"
                    disabledID: releasedID + "-disabled"

                    font {
                        family: IngeScapeTheme.textFontFamily
                        weight : Font.Medium
                        pixelSize : 16
                    }
                    labelColorPressed: IngeScapeTheme.blackColor
                    labelColorReleased: IngeScapeTheme.whiteColor
                    labelColorDisabled: IngeScapeTheme.greyColor

                }

                onClicked: {
                    console.log("QML: create new Characteristic " + txtCharacteristicName.text + " of type " + rootPopup.selectedType);

                    if (controller)
                    {
                        // Selected type is ENUM
                        if (rootPopup.selectedType === CharacteristicValueTypes.CHARACTERISTIC_ENUM)
                        {
                            // Use only the N first elements of the array (the array may be longer than the number of displayed TextFields
                            // if the user decreases the value of the spin box after edition the last TextField)
                            // Where N = spinBoxValuesNumber.value (the value of the spin box)
                            var displayedEnumTexts = rootPopup.enumTexts.slice(0, spinBoxValuesNumber.value);

                            var index = 0;
                            var isEmptyValue = false;

                            displayedEnumTexts.forEach(function(element) {
                                if (element === "") {
                                    isEmptyValue = true;
                                    console.log("value at " + index + " is empty, edit it !");
                                }
                                index++;
                              });

                            console.log("QML: Enum with " + spinBoxValuesNumber.value + " strings: " + displayedEnumTexts);

                            if (isEmptyValue === false)
                            {
                                rootPopup.controller.createNewCharacteristicEnum(txtCharacteristicName.text, displayedEnumTexts);

                                // Reset all user inputs and close the popup
                                rootPopup.resetInputsAndClosePopup();
                            }
                            else
                            {
                                console.warn("Some values of the enum are empty, edit them !");
                            }
                        }
                        // Selected type is NOT ENUM
                        else
                        {
                            rootPopup.controller.createNewCharacteristic(txtCharacteristicName.text, rootPopup.selectedType);

                            // Reset all user inputs and close the popup
                            rootPopup.resetInputsAndClosePopup();
                        }
                    }
                }
            }
        }

    }

}

/*
 *	IngeScape Editor
 *
 *  Copyright © 2017-2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

import QtQuick 2.8
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import I2Quick 1.0

import INGESCAPE 1.0

I2PopupBase {
    id: rootPopup

    width: 600
    height: Math.min(containerPopup.height, 750)

    dismissOnOutsideTap: false

    // Our controller
    property LicensesController licensesController: null;

    // Flag indicating if we prevent the user to continue to use our Application
    // We display only the button "Quit"
    property bool allowsOnlyQuit: false


    //--------------------------------------------------------
    //
    //
    // Functions
    //
    //
    //--------------------------------------------------------

    //
    // function allowing to validate the form
    //
    function validate() {
        console.log("QML: function validate()");

        if (rootPopup.licensesController)
        {
            rootPopup.licensesController.updateLicensesPath(txtLicensesPath.text);
        }

        // Close our popup
        rootPopup.close();
    }


    //--------------------------------------------------------
    //
    //
    // Callbacks
    //
    //
    //--------------------------------------------------------

    onOpened: {
        if (rootPopup.licensesController) {
            txtLicensesPath.text = rootPopup.licensesController.licensesPath;
        }

        // Set the focus to catch keyboard press on Return/Escape
        rootPopup.focus = true;
    }

    Keys.onEscapePressed: {
        //console.log("QML: Escape Pressed");

        rootPopup.close();
    }

    Keys.onReturnPressed: {
        //console.log("QML: Return Pressed");

        rootPopup.validate();
    }


    //--------------------------------------------------------
    //
    //
    // Content
    //
    //
    //--------------------------------------------------------

    Rectangle {
        anchors.fill: parent

        color: IngeScapeTheme.veryDarkGreyColor

        border {
            width: 2
            color: IngeScapeTheme.editorsBackgroundBorderColor
        }

        radius: 5
    }

    Item {
        id: containerPopup

        anchors {
            top: parent.top
            right: parent.right
            left: parent.left
            leftMargin: 18
            rightMargin: 18
        }

        height: childrenRect.height + childrenRect.y

        Text {
            id: title

            anchors {
                top: parent.top
                topMargin: 25
                left: parent.left
            }

            text: qsTr("Licenses")

            color: IngeScapeTheme.whiteColor
            font {
                family: IngeScapeTheme.textFontFamily
                weight : Font.Medium
                pixelSize : 23
            }
        }

        Item {
            id: directoryPathItem
            anchors {
                left: parent.left
                right: parent.right
                top: title.bottom
                topMargin: 18
            }

            height: childrenRect.height

            Text {
                text: qsTr("Directory path")

                color: IngeScapeTheme.whiteColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    weight : Font.Medium
                    pixelSize : 16
                }
            }

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                }

                height: childrenRect.height

                TextField {
                    id: txtLicensesPath

                    anchors {
                        left: parent.left
                        right: btnSelectLicencesDirectory.left
                        rightMargin: 10
                    }
                    height: btnSelectLicencesDirectory.height
                    verticalAlignment: TextInput.AlignVCenter

                    text: "" //rootPopup.controller.licensesPath

                    enabled: false

                    style: I2TextFieldStyle {
                        backgroundColor: IngeScapeTheme.darkBlueGreyColor
                        backgroundDisabledColor: IngeScapeTheme.darkBlueGreyColor

                        borderColor: IngeScapeTheme.whiteColor
                        borderDisabledColor: IngeScapeTheme.whiteColor

                        borderErrorColor: IngeScapeTheme.redColor

                        radiusTextBox: 1
                        borderWidth: 0;
                        borderWidthActive: 1

                        textIdleColor: IngeScapeTheme.whiteColor
                        textDisabledColor: IngeScapeTheme.whiteColor

                        padding.left: 6
                        padding.right: 3

                        font {
                            pixelSize:15
                            family: IngeScapeTheme.textFontFamily
                        }
                    }
                }

                Button {
                    id: btnSelectLicencesDirectory

                    property var boundingBox: IngeScapeTheme.svgFileIngeScape.boundsOnElement("button");

                    anchors {
                        right: parent.right
                    }

                    height: boundingBox.height
                    width: boundingBox.width

                    text: "Change..."
                    activeFocusOnPress: true

                    style: I2SvgButtonStyle {
                        fileCache: IngeScapeTheme.svgFileIngeScape

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
                        if (rootPopup.licensesController)
                        {
                            var directoryPath = rootPopup.licensesController.selectLicensesDirectory();
                            if (directoryPath) {
                                txtLicensesPath.text = directoryPath;

                                rootPopup.licensesController.updateLicensesPath(directoryPath);
                            }
                        }
                    }
                }
            }

            // Vertical space
            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                height: 10
            }
        }

        Text {
            id: errorMessage

            anchors {
                top: directoryPathItem.bottom
                topMargin: 25
                left: parent.left
                leftMargin: 25
                right: parent.right
                rightMargin: 25
            }
            wrapMode: Text.WordWrap

            height: (text === "") ? 0 : 30

            Connections {
                target: rootPopup.licensesController

                onLicenseLimitationReached: {
                    errorMessage.visible = true
                }
            }

            text: rootPopup.licensesController ? rootPopup.licensesController.errorMessageWhenLicenseFailed : ""

            color: IngeScapeTheme.orangeColor
            font {
                family: IngeScapeTheme.textFontFamily
                weight : Font.Medium
                pixelSize : 16
            }
        }

        Item {
            id: summary

            anchors {
                top: errorMessage.bottom
                topMargin: (errorMessage.height > 0) ? 25 : 0
                left: parent.left
                right: parent.right
            }

            height: childrenRect.height

            Text {
                id : summaryTitle

                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: 10
                }

                text: rootPopup.licensesController.licenseDetailsList.count > 1 ? qsTr("Summary") : qsTr("Your license")

                color: IngeScapeTheme.whiteColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    capitalization: Font.AllUppercase
                    weight : Font.Medium
                    pixelSize : 20
                }
            }

            Rectangle {
                id: summarySeparator

                anchors {
                    left: parent.left
                    right: parent.right
                    top: summaryTitle.bottom
                    topMargin : 10
                }

                height: 1

                color: IngeScapeTheme.whiteColor
            }

            LicenseInformationView {
                id: summaryView

                anchors {
                   top: summarySeparator.bottom
                   topMargin: 10
                   right: parent.right
                   left: parent.left
                }

                licenseInformation: rootPopup.licensesController.mergedLicense
            }
        }

        Item {
            id: licensesDetails

            anchors {
                top: summary.bottom
                topMargin : 22
                left: parent.left
                right: parent.right
            }

            height: childrenRect.height

            Item {
                id: onlyOneLicenseView
                anchors {
                    top: parent.top
                    right: parent.right
                    left: parent.left
                }

                visible: rootPopup.licensesController.licenseDetailsList.count === 1

                height : visible ? buttonDelete.height : 0

                property LicenseInformationM model : rootPopup.licensesController.licenseDetailsList.get(0)

                LabellessSvgButton {
                    id: buttonDelete

                    anchors {
                        top: parent.top
                        right: parent.right
                    }

                    enabled: true

                    onEnabledChanged : {
                        console.log("hey ");
                    }

                    releasedID: "delete-license"
                    pressedID: "delete-license-pressed"
                    disabledID: "delete-license-pressed"

                    onClicked: {
                        // Disable root popup while other popup isn't closed
                        rootPopup.enabled = false;

                        // Open delete confirmation popup
                        deleteConfirmationPopup.license = onlyOneLicenseView.model
                        deleteConfirmationPopup.open();
                    }
                }

                SvgImage {
                    id: licenseFilePicto

                    anchors {
                        verticalCenter : buttonDelete.verticalCenter
                        left: parent.left
                    }

                    visible: licenseFileText.text !== ""

                    svgElementId : onlyOneLicenseView.model && onlyOneLicenseView.model.ingescapeLicenseValidity  &&  onlyOneLicenseView.model.editorLicenseValidity ? "license-ok" : "license-fail"
                }

                Text {
                    id : licenseFileText

                    anchors {
                        verticalCenter: licenseFilePicto.verticalCenter
                        left: licenseFilePicto.right
                        leftMargin: 8
                    }

                    text: onlyOneLicenseView.model.fileName

                    color: IngeScapeTheme.whiteColor
                    font {
                        family: IngeScapeTheme.textFontFamily
                        weight : Font.Medium
                        pixelSize : 14
                    }
                }
            }

            Rectangle {
                id: detailsSeparator

                anchors {
                    left: parent.left
                    right: parent.right
                    top: onlyOneLicenseView.bottom
                    topMargin : 10
                }

                height: 1

                color: IngeScapeTheme.whiteColor
            }
        }

        Item {
            id: footer

            anchors {
                top: licensesDetails.bottom
                right : parent.right
                left: parent.left
            }

            height: 70

            Button {
                id: okButton

                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }

                property var boundingBox: IngeScapeTheme.svgFileIngeScape.boundsOnElement("button");

                height: boundingBox.height
                width: boundingBox.width

                activeFocusOnPress: true
                text: rootPopup.allowsOnlyQuit ? qsTr("Quit") : qsTr("OK")

                anchors {
                    verticalCenter: parent.verticalCenter
                }

                style: I2SvgButtonStyle {
                    fileCache: IngeScapeTheme.svgFileIngeScape

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
                    if (rootPopup.allowsOnlyQuit)
                    {
                        console.info("QML: QUIT on License Popup")

                        // Quit our application (close the main window)
                        Qt.quit();
                    }
                    else
                    {
                        console.log("QML: OK on License Popup")
                        rootPopup.validate();
                    }
                }
            }
        }

    }

    //    ScrollView {
    //        id: detailsScrollView

    //        anchors {
    //            top: summary.bottom
    //            left: parent.left
    //            leftMargin: 25
    //            right: parent.right
    //            rightMargin: 25
    //            bottom: buttonRow.top
    //            bottomMargin: 25
    //        }

    //        visible: licenseDetailsRepeater.model && licenseDetailsRepeater.model.count > 0

    //        contentItem: Column {
    //            id: detailsColumn
    //            anchors {
    //                left: parent.left
    //                right: parent.right
    //            }

    //            spacing: 26

    ////            LicenseInformationView {
    ////                licenseInformation: rootPopup.licensesController.mergedLicense
    ////            }

    //            Text {
    //                text: rootPopup.licensesController ? qsTr("License details:") : ""

    //                color: IngeScapeTheme.whiteColor
    //                font {
    //                    family: IngeScapeTheme.textFontFamily
    //                    weight: Font.Medium
    //                    pixelSize: 16
    //                    bold: true
    //                }
    //            }

    //            Repeater {
    //                id: licenseDetailsRepeater
    //                model: rootPopup.licensesController ? rootPopup.licensesController.licenseDetailsList : 0

    //                delegate: Column {
    //                    spacing: 26

    //                    Item {

    //                        anchors {
    //                            left: parent.left
    //                            right: parent.right
    //                        }

    //                        height: childrenRect.height

    //                        Text {
    //                            id: licenseFileName
    //                            text: qsTr("###\n### License file name: %1\n###").arg(model ? model.fileName : "")

    //                            anchors {
    //                                left: parent.left
    //                                right: parent.right
    //                            }

    //                            color: IngeScapeTheme.whiteColor
    //                            elide: Text.ElideRight
    //                            font {
    //                                family: IngeScapeTheme.textFontFamily
    //                                weight: Font.Medium
    //                                pixelSize: 16
    //                                italic: true
    //                            }
    //                        }

    //                        Button {
    //                            text: "DEL"
    //                            anchors {
    //                                right: parent.right
    //                                verticalCenter: licenseFileName.verticalCenter
    //                            }

    //                            onClicked: {
    //                                // Disable root popup while other popup isn't closed
    //                                rootPopup.enabled = false;

    //                                // Open delete confirmation popup
    //                                deleteConfirmationPopup.license = model
    //                                deleteConfirmationPopup.open();
    //                            }

    //                        }
    //                    }

    //                    LicenseInformationView {
    //                        licenseInformation: model ? model.QtObject : null
    //                    }
    //                }
    //            }
    //        }
    //    }

    //    Item {
    //        id: emptyLicenseFeedback
    //        anchors.fill: detailsScrollView

    //        visible: !detailsScrollView.visible

    //        Text {
    //            id: emptyLicenseFeedbackText

    //            anchors {
    //                left: parent.left
    //                right: parent.right
    //                verticalCenter: parent.verticalCenter
    //            }

    //            text: "No license file found.\nThe editor is running in demo mode with limitations.\n\n\n\nTo change this, please set the license directory above,\ndrop a license file here \nor use the \"Import...\" button below."
    //            verticalAlignment: Text.AlignVCenter
    //            horizontalAlignment: Text.AlignHCenter

    //            color: IngeScapeTheme.whiteColor
    //            wrapMode: Text.WordWrap
    //            font {
    //                family: IngeScapeTheme.textFontFamily
    //                pixelSize : 18
    //                italic: true
    //            }
    //        }

    //        Button {
    //            property var boundingBox: IngeScapeTheme.svgFileIngeScape.boundsOnElement("button");

    //            height: boundingBox.height
    //            width: boundingBox.width

    //            activeFocusOnPress: true
    //            text: "Import..."

    //            anchors {
    //                top: emptyLicenseFeedbackText.bottom
    //                topMargin: 25
    //                horizontalCenter: parent.horizontalCenter
    //            }

    //            style: I2SvgButtonStyle {
    //                fileCache: IngeScapeTheme.svgFileIngeScape

    //                pressedID: releasedID + "-pressed"
    //                releasedID: "button"
    //                disabledID: releasedID + "-disabled"

    //                font {
    //                    family: IngeScapeTheme.textFontFamily
    //                    weight : Font.Medium
    //                    pixelSize : 16
    //                }
    //                labelColorPressed: IngeScapeTheme.blackColor
    //                labelColorReleased: IngeScapeTheme.whiteColor
    //                labelColorDisabled: IngeScapeTheme.greyColor

    //            }

    //            onClicked: {
    //                if (rootPopup.licensesController) {
    //                    rootPopup.licensesController.importLicense();
    //                }
    //            }
    //        }
    //    }

    //    DropArea {
    //        id: dropZone
    //        anchors.fill: detailsScrollView

    //        property bool dragHovering: false

    //        onEntered: {
    //            dragHovering = true
    //        }

    //        onExited: {
    //            dragHovering = false
    //        }

    //        onDropped: {
    //            dragHovering = false
    //            if (drop.hasUrls && rootPopup.licensesController)
    //            {
    //                rootPopup.licensesController.addLicenses(drop.urls)
    //            }
    //        }

    //        // Overlay appearing when the user drags something over the drop zone
    //        Rectangle {
    //            id: dropZoneOverlay
    //            anchors.fill: parent

    //            color: IngeScapeTheme.veryLightGreyColor
    //            opacity: dropZone.dragHovering ? 0.65 : 0

    //            Behavior on opacity {
    //                NumberAnimation {}
    //            }
    //        }
    //    }

    // Feedback layer on root popup when it is disabled
    Rectangle {
        anchors.fill: parent

        visible : !rootPopup.enabled

        color: '#99000000';
    }

    //
    // Delete Confirmation
    //
    ConfirmationPopup {
        id: deleteConfirmationPopup

        property var license: null

        confirmationText: "You will definitely lose all the rights this license grants you.\nDo you want to delete it completely?"

        onConfirmed: {
            rootPopup.enabled = true;
            if (license && rootPopup.licensesController) {
                rootPopup.licensesController.deleteLicense(license.QtObject);
            }
        }

        onCancelled: {
            rootPopup.enabled = true;
        }
    }
}



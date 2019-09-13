/*
 *	IngeScape Editor
 *
 *  Copyright © 2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *      Justine Limoges <limoges@ingenuity.io>
 */

import QtQuick 2.8
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.3

import I2Quick 1.0

import INGESCAPE 1.0

//import "../theme" as Theme;


Window {
    id: rootItem

    title: model.QtObject ? model.QtObject.name
                          : qsTr("Definition")

    height: minimumHeight
    width: minimumWidth

    minimumWidth: 666
    minimumHeight: 420

    flags: Qt.Dialog

//    automaticallyOpenWhenCompleted: true
//    isModal: false
//    dismissOnOutsideTap : false;
//    keepRelativePositionToInitialParent : false;


    //--------------------------------
    //
    // Properties
    //
    //--------------------------------

    // our model is a definition
    property var definition: model.QtObject;



    // Columns with a fixed size
    // - Type
    property int widthColumnType: 135
    // - Mute
    property int widthColumnMute: 40


    // Minimum widht of resizable columns
    // - Name
    property int minWidthColumnName: 143
    // - Definition Value
    property int minWidthColumnInitialValue: 150
    // - Mapping Value
    property int minWidthColumnCurrentValue: 150

    // Maximum widht of resizable columns
    // - Name
    property int maxWidthColumnName: 230


    // Resizable columns
    // - Name
    // 8 <=> scrollbar width
    // - Available extra width that can be splitted between our 3 resizable columns
    property int splittableAvailableWidth: (tabs.width - 8 - widthColumnType - minWidthColumnInitialValue - minWidthColumnCurrentValue - minWidthColumnName - widthColumnMute)
    property int extraWidthForResizableColumns: Math.max(0, (splittableAvailableWidth/3))
    property int extraWidthForValuesOnly: if (widthColumnName === maxWidthColumnName) {
                                              Math.max(0, ((splittableAvailableWidth - (maxWidthColumnName - minWidthColumnName))/2));
                                          }
                                          else {
                                              0;
                                          }

    // - Width Resizable Columns
    property int widthColumnName: ((minWidthColumnName + extraWidthForResizableColumns) > maxWidthColumnName) ? maxWidthColumnName : (minWidthColumnName + extraWidthForResizableColumns);
    property int widthColumnInitialValue: if (widthColumnName === maxWidthColumnName) {
                                              minWidthColumnInitialValue + extraWidthForValuesOnly;
                                          }
                                          else {
                                              minWidthColumnInitialValue + extraWidthForResizableColumns;
                                          }
    property int widthColumnCurrentValue: if (widthColumnName === maxWidthColumnName) {
                                              minWidthColumnCurrentValue + extraWidthForValuesOnly;
                                          }
                                          else {
                                              minWidthColumnCurrentValue + extraWidthForResizableColumns;
                                          }

    // List of widths
    property var widthsOfColumns: [
        widthColumnName,
        widthColumnType,
        widthColumnInitialValue,
        widthColumnCurrentValue,
        widthColumnMute
    ]


    //--------------------------------
    //
    // Signals
    //
    //--------------------------------

    // Emitted when user clicks on "history" button
    signal openHistory();


    //--------------------------------
    //
    // Behavior
    //
    //--------------------------------

    Connections {
        target: definition

        //ignoreUnknownSignals: true

        onBringToFront: {
            //console.log("QML of Agent Definition Editor: onBringToFront");

            // Raises the window in the windowing system.
            rootItem.raise();
        }
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


        LabellessSvgButton {
            id: btnCloseEditor

            anchors {
                verticalCenter: definitionNameItem.verticalCenter
                right : parent.right
                rightMargin: 20
            }

            pressedID: releasedID + "-pressed"
            releasedID: "closeEditor"
            disabledID : releasedID


            onClicked: {
                // Close our popup
                rootItem.close();
            }
        }


        // Definition name and version
        Item {
            id : definitionNameItem

            anchors {
                top : parent.top
                topMargin: 15
                left : parent.left
                leftMargin: 18
                right : parent.right
                rightMargin: 18
            }

            height : definitionNameTxt.height

            TextMetrics {
                id : definitionName

                elideWidth: (definitionNameItem.width - versionName.width)
                elide: Text.ElideRight

                text: definition ? definition.name : ""
            }

            Text {
                id: definitionNameTxt

                anchors {
                    left : parent.left
                }

                text: definitionName.elidedText

                color: IngeScapeEditorTheme.definitionEditorsLabelColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    pixelSize: 23
                    weight: Font.Medium
                }

            }

            Text {
                id : versionName
                anchors {
                    bottom: definitionNameTxt.bottom
                    bottomMargin : 2
                    left : definitionNameTxt.right
                    leftMargin: 5
                }

                text: definition ? "(v" + definition.version + ")" : ""
                color: definitionNameTxt.color

                font {
                    family: IngeScapeTheme.textFontFamily
                    pixelSize : 16
                    italic : true
                }
            }
        }


        Text {
            id : descriptionTxt
            anchors {
                top : definitionNameItem.bottom
                topMargin: 28
                left : definitionNameItem.left
                right : parent.right
                rightMargin: 22
            }

            text: definition ? definition.description : ""

            width: parent.width
            wrapMode: Text.Wrap
            elide : Text.ElideRight
            maximumLineCount : 3

            color: IngeScapeEditorTheme.definitionEditorsAgentDescriptionColor
            font {
                family: IngeScapeTheme.textFontFamily
                pixelSize : 16
            }
        }



        I2TabView {
            id : tabs

            anchors {
                top: parent.top
                topMargin: 130
                left: parent.left
                leftMargin: 20
                right: parent.right
                rightMargin: 20
                bottom: parent.bottom
                bottomMargin: 50
            }
            clip : true

            style: I2TabViewStyle {
                frameOverlap: 1
                tabsAlignment : Qt.AlignHCenter
                tab: Item {
                    implicitHeight: 30
                    implicitWidth: 104

                    I2SvgItem {
                        id : svgMenu
                        anchors.centerIn: parent

                        svgFileCache: IngeScapeEditorTheme.svgFileIngeScapeEditor
                        svgElementId: if (index === 0) {
                                          "tab-0" + (styleData.selected ? "-Selected" : "")
                                      } else if (0 < index && index < tabRepeater.count - 1) {
                                          "tab-1" + (styleData.selected ? "-Selected" : "")
                                      } else if (index === tabRepeater.count - 1) {
                                          "tab-2" + (styleData.selected ? "-Selected" : "")
                                      }

                        Text {
                            anchors.centerIn: parent

                            text : styleData.title

                            color : styleData.selected ? IngeScapeTheme.veryDarkGreyColor : IngeScapeTheme.whiteColor
                            font {
                                family: IngeScapeTheme.textFontFamily
                                weight : Font.Medium
                                pixelSize : 16
                            }
                        }
                    }
                }

                frame: Rectangle {
                    color : "transparent"
                }
            }

            onCurrentIndexChanged: {
            }


            //---------------------------------------
            //
            // Content of our tabs
            //
            //---------------------------------------
            Repeater {
                id: tabRepeater
                model : {
                    var model = [ qsTr("Inputs"), qsTr("Outputs"), qsTr("Parameters") ]
                    if (definition && definition.callsList && definition.callsList.count > 0) {
                        model.push(qsTr("Calls"))
                    }
                    return model;
                }

                Tab {
                    id : tab
                    title: modelData;
                    active : true

                    property int modelIndex: index

                    Item {
                        anchors.fill: parent

                        /// ****** Headers of columns ***** ////
                        Row {
                            id: tableauHeaderRow

                            anchors {
                                left: parent.left
                                right: parent.right
                                top: parent.top
                                topMargin: 15
                            }
                            height : 33

                            Repeater {
                                model: if (tab.modelIndex === 1) { // "Output"
                                           [
                                           qsTr("Name"),
                                           qsTr("Type"),
                                           qsTr("Initial value"),
                                           qsTr("Current value"),
                                           qsTr("Mute")
                                           ]
                                       } else if (tab.modelIndex === 3) { // "Calls"
                                           [
                                           qsTr("Prototype"),
                                           "",
                                           "",
                                           "",
                                           ""
                                           ]
                                       } else {
                                           [
                                           qsTr("Name"),
                                           qsTr("Type"),
                                           qsTr("Initial value"),
                                           "",
                                           ""
                                           ]
                                       }

                                Item {
                                    height : 33
                                    width : rootItem.widthsOfColumns[index]

                                    Text {
                                        anchors {
                                            left: parent.left
                                            leftMargin: 2
                                            verticalCenter: parent.verticalCenter
                                        }

                                        text: modelData

                                        color: IngeScapeEditorTheme.definitionEditorsAgentDescriptionColor
                                        font {
                                            family: IngeScapeTheme.textFontFamily
                                            pixelSize : 16
                                        }
                                    }
                                }
                            }
                        }


                        // separator
                        Rectangle {
                            anchors {
                                left : parent.left
                                right : parent.right
                                top : tableauHeaderRow.bottom
                            }
                            height : 1

                            color : IngeScapeTheme.blackColor
                        }

                        /// ****** List ***** ////
                        ScrollView {
                            id : scrollView

                            anchors {
                                top: tableauHeaderRow.bottom
                                left : parent.left
                                right : parent.right
                                bottom : parent.bottom
                            }

                            // Prevent drag overshoot on Windows
                            flickableItem.boundsBehavior: Flickable.OvershootBounds

                            style: IngeScapeScrollViewStyle {
                            }

                            // Content of our scrollview
                            ListView {
                                id : listView

                                width : scrollView.width

                                model: if (definition) {
                                           switch (tabs.currentIndex)
                                           {
                                           case 0:
                                               definition.inputsList
                                               break;
                                           case 1:
                                               definition.outputsList
                                               break;
                                           case 2:
                                               definition.parametersList
                                               break;
                                           }
                                       }
                                       else {
                                           0
                                       }

                                delegate: Item {
                                    anchors {
                                        left : parent.left
                                        right : parent.right
                                    }
                                    height : 30

                                    Row {
                                        id: listLine

                                        anchors {
                                            fill : parent
                                        }

                                        // Name
                                        Text {
                                            text: model.name

                                            anchors {
                                                verticalCenter: parent.verticalCenter
                                            }
                                            verticalAlignment: Text.AlignVCenter
                                            width : rootItem.widthsOfColumns[0]
                                            elide: Text.ElideRight
                                            height: parent.height
                                            color: IngeScapeTheme.whiteColor
                                            font {
                                                family: IngeScapeTheme.textFontFamily
                                                pixelSize : 16
                                            }
                                        }


                                        // Type
                                        Item {
                                            anchors {
                                                verticalCenter: parent.verticalCenter
                                            }
                                            height: parent.height
                                            width : rootItem.widthsOfColumns[1]

                                            Rectangle {
                                                id : circle
                                                anchors {
                                                    left : parent.left
                                                    verticalCenter: parent.verticalCenter
                                                }

                                                width : 16
                                                height : width
                                                radius : width/2

                                                color: IngeScapeEditorTheme.colorOfIOPTypeWithConditions(model.agentIOPValueTypeGroup, true);
                                            }

                                            Text {
                                                text: AgentIOPValueTypes.enumToString(model.agentIOPValueType)

                                                anchors {
                                                    verticalCenter: circle.verticalCenter
                                                    verticalCenterOffset: 1
                                                    left : circle.right
                                                    leftMargin: 5
                                                    right : parent.right
                                                }
                                                verticalAlignment: Text.AlignVCenter
                                                elide: Text.ElideRight
                                                height: parent.height
                                                color: IngeScapeTheme.whiteColor
                                                font {
                                                    family: IngeScapeTheme.textFontFamily
                                                    pixelSize : 16
                                                }
                                            }
                                        }


                                        // Initial Value
                                        Text {
                                            text: model.displayableDefaultValue

                                            anchors {
                                                verticalCenter: parent.verticalCenter
                                            }
                                            verticalAlignment: Text.AlignVCenter
                                            width : rootItem.widthsOfColumns[2]
                                            height: parent.height
                                            elide: Text.ElideRight
                                            color: IngeScapeTheme.whiteColor
                                            font {
                                                family: IngeScapeTheme.textFontFamily
                                                pixelSize : 16
                                            }
                                        }


                                        // Current Value
                                        Text {
                                            text: model.displayableCurrentValue

                                            visible: (model.agentIOPType === AgentIOPTypes.OUTPUT)

                                            anchors {
                                                verticalCenter: parent.verticalCenter
                                            }
                                            verticalAlignment: Text.AlignVCenter
                                            width : rootItem.widthsOfColumns[3]
                                            height: parent.height
                                            color: IngeScapeTheme.whiteColor
                                            elide: Text.ElideRight

                                            font {
                                                family: IngeScapeTheme.textFontFamily
                                                pixelSize : 16
                                            }
                                        }

                                        // Mute
                                        Item {
                                            anchors {
                                                verticalCenter: parent.verticalCenter
                                            }
                                            width: rootItem.widthsOfColumns[4]
                                            height: parent.height

                                            LabellessSvgButton {
                                                id: btnMuteOutput

                                                anchors {
                                                    verticalCenter: parent.verticalCenter
                                                    right : parent.right
                                                }

                                                visible: (model.agentIOPType === AgentIOPTypes.OUTPUT)
                                                enabled: visible


                                                pressedID: releasedID + "-pressed"
                                                //releasedID: model.isMutedOutput ? "active-mute" : "inactive-mute"
                                                releasedID: model.isMuted ? "active-mute" : "inactive-mute"

                                                disabledID : releasedID

                                                onClicked: {
                                                    model.QtObject.changeMuteOutput();
                                                }
                                            }
                                        }
                                    }
                                }


                                //separator
                                Rectangle {
                                    anchors {
                                        left : parent.left
                                        right : parent.right
                                        bottom : parent.bottom
                                    }
                                    height : 1

                                    color : IngeScapeTheme.blackColor
                                }

                            }
                        }
                    }
                }

            }

        }


        //
        // History button
        //
        MouseArea {
            id : historyBtn
            enabled: visible
            anchors {
                left : tabs.left
                top : tabs.bottom
                topMargin: 16
            }

            height : history.height
            width : history.width

            hoverEnabled: true

            onClicked: {
                if (definition) {
                    definition.openValuesHistoryOfAgent();
                }
                rootItem.openHistory();
            }

            Text {
                id: history

                anchors {
                    left : parent.left
                }
                text : "> Outputs history"
                color: historyBtn.pressed ? IngeScapeTheme.lightGreyColor : IngeScapeTheme.whiteColor
                elide: Text.ElideRight

                font {
                    family: IngeScapeTheme.textFontFamily
                    pixelSize: 16
                }
            }

            // Underline
            Rectangle {
                visible: historyBtn.containsMouse

                anchors {
                    left : historyBtn.left
                    right : history.right
                    bottom : parent.bottom
                }

                height : 1

                color : history.color
            }
        }

    }


    I2Layer {
        id: overlayLayerComboBox
        objectName: "overlayLayerComboBox"

        anchors.fill: parent
    }
}

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
 *      Bruno Lemenicier  <lemenicier@ingenuity.io>
 *
 */

import QtQuick 2.8
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import I2Quick 1.0

import INGESCAPE 1.0


// theme sub-directory
import "../theme" as Theme;

// parent-directory
import ".." as Editor;


Item {
    id: rootItem

    anchors.fill: parent

    //--------------------------------
    //
    // Properties
    //
    //--------------------------------

    // Controller associated to our view
    property var controller : null;




    //--------------------------------
    //
    // Content
    //
    //--------------------------------

    // allowing to deselect selected record
    MouseArea {
        anchors.fill: parent
        onClicked:  {
            if(controller.selectedRecord)
            {
                controller.selectedRecord = null;
            }
        }
    }

    //
    // List of records
    //
    ScrollView {
        id : recordsListScrollView

        anchors {
            top: parent.top
            topMargin: 78
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        // Prevent drag overshoot on Windows
        flickableItem.boundsBehavior: Flickable.OvershootBounds

        style: IngeScapeScrollViewStyle {
        }

        // Content of our scrollview
        ListView {
            id: recordsList

            model: controller.recordsList

            delegate: componentRecordListItem

            height: contentHeight

            anchors
            {
                left:parent.left
                right:parent.right
            }


            //
            // Transition animations
            //
            add: Transition {
                NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
                NumberAnimation { property: "scale"; from: 0.0; to: 1.0 }
            }

            displaced: Transition {
                NumberAnimation { properties: "x,y"; easing.type: Easing.OutBounce }

                // ensure opacity and scale values return to 1.0
                NumberAnimation { property: "opacity"; to: 1.0 }
                NumberAnimation { property: "scale"; to: 1.0 }
            }

            move: Transition {
                NumberAnimation { properties: "x,y"; easing.type: Easing.OutBounce }

                // ensure opacity and scale values return to 1.0
                NumberAnimation { property: "opacity"; to: 1.0 }
                NumberAnimation { property: "scale"; to: 1.0 }
            }

            remove: Transition {
                // ensure opacity and scale values return to 0.0
                NumberAnimation { property: "opacity"; to: 0.0 }
                NumberAnimation { property: "scale"; to: 0.0 }
            }

        }
    }



    //
    // Header
    //
    Rectangle {
        id: header

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom : recordsListScrollView.top
        }

        color : IngeScapeTheme.selectedTabsBackgroundColor
    }


    //
    // Separator
    //
    Rectangle {
        anchors {
            bottom: recordsListScrollView.top
            left: parent.left
            right: parent.right
        }

        color: IngeScapeTheme.leftPanelBackgroundColor

        height: 1
    }



    //
    // Visual representation of an record in our list
    //
    Component {
        id: componentRecordListItem



            Item {
                id : recordItem

                property var agentmodel: model.listOfAgents

                property int margin: 5

                height: recordInfos.height + margin*2

                anchors {
                    left : parent.left
                    right : parent.right
                }

                // separator
                Rectangle {
                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                    }

                    color: IngeScapeTheme.leftPanelBackgroundColor

                    height: 1
                }

                Column
                {
                    id:recordInfos
                    y: margin

                    anchors {
                        left : parent.left
                        right : parent.right
                        leftMargin: 28
                        rightMargin: 12
                    }

                    spacing : 4

                    // Name
                    Text {
                        id: recordName

                        anchors {
                            left : parent.left
                            right : parent.right
                        }
                        elide: Text.ElideRight

                        text: model.recordModel.name
                        color: IngeScapeTheme.agentsListLabelColor
                        font: IngeScapeTheme.headingFont
                    }

                    // IP address
                    Text {
                        id: recordIP

                        anchors {
                            left : parent.left
                            right : parent.right
                        }
                        elide: Text.ElideRight

                        text: qsTr("%1 %2").arg(Qt.formatDate(model.recordModel.beginDateTime, "dd/MM/yyyy")).arg(Qt.formatTime(model.recordModel.beginDateTime, "HH:mm"))
                        color: IngeScapeTheme.agentsListTextColor
                        font: IngeScapeTheme.heading2Font
                    }
                }


                // Record can be clicked
                MouseArea
                {
                    anchors.fill: parent

                    onPressed: {
                        if (controller) {
                            if(controller.selectedRecord === model.QtObject)
                            {
                                controller.selectedRecord = null;
                            } else {
                                controller.selectedRecord = model.QtObject;
                            }
                        }
                    }
                }

                // Selected Record feedback
                Item {
                    anchors.fill: parent
                    visible : controller && (controller.selectedRecord === model.QtObject);

                    Rectangle {
                        anchors {
                            left : parent.left
                            top : parent.top
                            bottom: parent.bottom
                        }

                        width : 6
                        color : IngeScapeTheme.selectedAgentColor
                    }

                    Button {
                        id: removeButton

                        activeFocusOnPress: true

                        anchors {
                            top: parent.top
                            topMargin: 10
                            right : parent.right
                            rightMargin: 12
                        }

                        style: Theme.LabellessSvgButtonStyle {
                            fileCache: IngeScapeTheme.svgFileINGESCAPE

                            pressedID: releasedID + "-pressed"
                            releasedID: "supprimer"
                            disabledID : releasedID
                        }

                        onClicked: {

                            if (controller)
                            {
                                // Delete selected record
                                controller.deleteSelectedRecord();
                            }
                        }
                    }
                }



            }




    }
}





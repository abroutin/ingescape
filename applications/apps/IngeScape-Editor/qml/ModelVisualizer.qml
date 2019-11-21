/*
 *	IngeScape Editor
 *
 *  Copyright © 2017-2018 Ingenuity i/o. All rights reserved.
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
import QtQuick.Window 2.3

import INGESCAPE 1.0


// agent sub-directory
import "agent" as Agent


Rectangle {
    id: rootRectangle

    anchors.fill: parent

    color: IngeScapeTheme.editorsBackgroundColor
    border {
        width: 2
        color: IngeScapeTheme.editorsBackgroundBorderColor
    }

    clip: true

    visible: IngeScapeEditorC.isVisibleModelVisualizer


    Rectangle {
        id: rctGlobalNumbers

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            //margins: 5
        }
        width: 140

        color: "transparent"
        /*border {
            width: 1
            color: IngeScapeTheme.editorsBackgroundBorderColor
        }*/

        Column {
            spacing: 5

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: 5
            }

            Text {
                text: "Platform:"

                color: "gray"
                font {
                    pointSize: 16
                    weight: Font.Bold
                }
            }

            Text {
                text: IngeScapeEditorC.currentPlatformName

                color: "white"
                font {
                    pointSize: 16
                    weight: Font.Bold
                }
            }

            Item {
                id: space
                width: 10
                height: 10
            }

            Text {
                text: "IGS app ON:"

                color: "gray"
                font {
                    pointSize: 16
                    weight: Font.Bold
                }
            }

            Text {
                text: "- " + IgsNetworkController.numberOfAgents + " agents"

                color: "white"
                font {
                    pointSize: 16
                    weight: Font.Medium
                }
            }

            Text {
                text: "- " + IgsNetworkController.numberOfLaunchers + " launchers"

                color: "white"
                font {
                    pointSize: 16
                    weight: Font.Medium
                }
            }

            Text {
                text: "- " + IgsNetworkController.numberOfRecorders + " recorders"

                color: "white"
                font {
                    pointSize: 16
                    weight: Font.Medium
                }
            }

            Text {
                text: "- " + IgsNetworkController.numberOfEditors + " editors"

                color: "white"
                font {
                    pointSize: 16
                    weight: Font.Medium
                }
            }

            Text {
                text: "- " + IgsNetworkController.numberOfAssessments + " assessments"

                color: "white"
                font {
                    pointSize: 16
                    weight: Font.Medium
                }
            }

            Text {
                text: "- " + IgsNetworkController.numberOfExpes + " expes"

                color: "white"
                font {
                    pointSize: 16
                    weight: Font.Medium
                }
            }
        }
    }

    //
    // List of all "Agents Grouped by Name"
    //
    ListView {
        id: list

        anchors {
            left: rctGlobalNumbers.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            margins: 5
        }
        //height: contentHeight

        model: IngeScapeEditorC.modelManager.allAgentsGroupsByName

        delegate: Rectangle {

            property AgentsGroupedByNameVM groupByNameVM: model.QtObject

            anchors {
                left: parent.left
            }
            width: childrenRect.width + 5
            //height: columnListOfGroupsByDefinition.height + 5
            height: columnListOfGroupsByDefinition.height + columnMappingElements.height + 5

            color: groupByNameVM && groupByNameVM.isON ? "#22CCCC" : "#227777"
            border {
                width: 1
                color: IngeScapeTheme.editorsBackgroundBorderColor
            }

            Row {
                id: rowGroupByName
                spacing: 15

                Text {
                    text: groupByNameVM ? groupByNameVM.name : ""
                    width: 150
                    color: groupByNameVM && groupByNameVM.isON ? "white" : "#888888"
                    font {
                        pointSize: 14
                        weight: Font.Bold
                    }
                }

                Text {
                    text: groupByNameVM ? groupByNameVM.models.count + " M"
                                        : "0 M"
                    width: 25
                    color: groupByNameVM && groupByNameVM.isON ? "white" : "#888888"
                    font.pointSize: 14
                }

                Text {
                    text: groupByNameVM ? groupByNameVM.numberOfAgentsON + " ON"
                                        : "0 ON"
                    width: 35
                    color: "green"
                    font {
                        pointSize: 14
                        weight: Font.Bold
                    }
                }

                Text {
                    text: groupByNameVM ? groupByNameVM.numberOfAgentsOFF + " OFF"
                                        : "0 OFF"
                    width: 35
                    color: "red"
                    font {
                        pointSize: 14
                        weight: Font.Bold
                    }
                }

                Column {
                    id: columnListOfGroupsByDefinition

                    height: childrenRect.height + 5

                    Repeater {
                        model: groupByNameVM ? groupByNameVM.allAgentsGroupsByDefinition : null

                        Rectangle {

                            property AgentsGroupedByDefinitionVM groupByDefinitionVM: model.QtObject

                            width: childrenRect.width + 5
                            height: childrenRect.height + 5

                            color: groupByDefinitionVM.isON ? "#22CC22" : "#227722"
                            border {
                                width: 1
                                color: IngeScapeTheme.editorsBackgroundBorderColor
                            }

                            Row {
                                spacing: 15

                                Text {
                                    text: groupByDefinitionVM.definition ? groupByDefinitionVM.definition.name : "Def is NULL"
                                    color: groupByDefinitionVM.isON ? "white" : "#888888"
                                    width: 150
                                    elide: Text.ElideMiddle
                                    font.pointSize: 14
                                }

                                Text {
                                    text: groupByDefinitionVM.models.count + " M"
                                    color: groupByDefinitionVM.isON ? "white" : "#888888"
                                    width: 25
                                    font.pointSize: 14
                                }


                                Column {
                                    id: columnAgents

                                    height: childrenRect.height + 5

                                    Repeater {
                                        model: groupByDefinitionVM.models

                                        Rectangle {

                                            property AgentM agentM: model.QtObject

                                            width: childrenRect.width
                                            height: 35

                                            color: agentM.isON ? "#2222CC" : "#222277"
                                            border {
                                                width: 1
                                                color: IngeScapeTheme.editorsBackgroundBorderColor
                                            }

                                            Row {
                                                spacing: 15

                                                anchors {
                                                    left: parent.left
                                                    top: parent.top
                                                    margins: 2
                                                }

                                                Text {
                                                    text: agentM.hostname
                                                    width: 150
                                                    color: agentM.isON ? "white" : "#888888"
                                                    font {
                                                        pointSize: 14
                                                        weight: Font.Bold
                                                    }
                                                }

                                                Text {
                                                    text: agentM.peerId
                                                    color: agentM.isON ? "white" : "#888888"
                                                }
                                            }

                                            Text {
                                                anchors {
                                                    left: parent.left
                                                    bottom: parent.bottom
                                                    margins: 2
                                                }

                                                text: agentM.commandLine
                                                color: agentM.isON ? "white" : "#888888"
                                                width: 690
                                                elide: Text.ElideLeft
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Column {
                id: columnMappingElements

                anchors {
                    top: rowGroupByName.bottom
                }
                height: childrenRect.height

                Repeater {
                    model: groupByNameVM ? groupByNameVM.allMappingElements : null

                    Text {
                        property MappingElementVM mappingElement: model.QtObject

                        text: mappingElement ? qsTr("%1 (%2)").arg(mappingElement.name).arg(mappingElement.models.count)
                                             : ""
                        color: mappingElement && mappingElement.hasCorrespondingLink ? "white" : "#888888"
                        font.pointSize: 13
                    }
                }
            }
        }
    }




}

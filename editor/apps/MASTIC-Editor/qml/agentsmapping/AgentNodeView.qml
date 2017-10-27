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
 *      Alexandre Lemort   <lemort@ingenuity.io>
 *
 */

import QtQuick 2.8
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import I2Quick 1.0

import MASTIC 1.0


Item {
    id: rootItem


    //--------------------------------
    //
    // Properties
    //
    //--------------------------------
    // Controller associated to our view
    property var controller : null;

    // Model associated to our QML item
    property var agentMappingVM: null
    property var agentName: agentMappingVM ? agentMappingVM.agentName : ""

    property bool isReduced : agentMappingVM && agentMappingVM.isReduced

    width : 228
    height : (rootItem.agentMappingVM && !rootItem.isReduced)?
                 (54 + 20*Math.max(rootItem.agentMappingVM.inputsList.count , rootItem.agentMappingVM.outputsList.count))
               : 42


    // Init position of our agent
    x: (agentMappingVM && agentMappingVM.position) ? agentMappingVM.position.x : 0
    y: (agentMappingVM && agentMappingVM.position) ? agentMappingVM.position.y : 0


    clip : true

    //
    // Bindings to save the position of our agent
    //
    Binding {
        target: rootItem.agentMappingVM
        property: "position"
        value: Qt.point(rootItem.x,rootItem.y)
    }


    Behavior on height {
        NumberAnimation {
            onStopped: {
            }
        }
    }

    //--------------------------------
    //
    // Signals
    //
    //--------------------------------




    //--------------------------------
    //
    // Functions
    //
    //--------------------------------



    //--------------------------------
    //
    // Content
    //
    //--------------------------------

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        drag.target: parent

        onPressed: {
            if (controller && agentMappingVM) {
                controller.selectedAgent = agentMappingVM
            }

            parent.z = rootItem.parent.maxZ++;
        }

        onPositionChanged: {
            //     console.log("agentMapping position " + model.position.x + "  " + model.position.y)
        }

        onDoubleClicked: {
            if (agentMappingVM) {
                agentMappingVM.isReduced = !agentMappingVM.isReduced;
            }
        }
    }

    Rectangle {
        anchors {
            fill: parent
            leftMargin: 10
            rightMargin: 12
            topMargin: 1
            bottomMargin: 1
        }

        color : mouseArea.pressed?
                    MasticTheme.darkGreyColor2
                  : (rootItem.agentMappingVM && rootItem.agentMappingVM.isON)? MasticTheme.darkGreyColor : MasticTheme.veryDarkGreyColor
        radius : 6

        Rectangle {
            anchors.fill: parent
            anchors.margins: -1

            visible : (controller && rootItem.agentMappingVM && (controller.selectedAgent === rootItem.agentMappingVM))
            color : "transparent"
            radius : 6

            border {
                width : 2
                color : MasticTheme.selectedAgentColor
            }

            Button {
                id: removeButton

                property var boundingBox: MasticTheme.svgFileMASTIC.boundsOnElement("supprimer");
                height : boundingBox.height
                width :  boundingBox.width

                visible : (rootItem.agentMappingVM && !rootItem.agentMappingVM.isON)

                anchors {
                    top: parent.top
                    topMargin: 10
                    right : parent.right
                    rightMargin: 10
                }

                style: I2SvgButtonStyle {
                    fileCache: MasticTheme.svgFileMASTIC

                    pressedID: releasedID + "-pressed"
                    releasedID: "supprimer"
                    disabledID : releasedID
                }

                onClicked: {
                    //                if (controller)
                    //                {
                    //                    // Delete our agent
                    //                    controller.deleteAgent(model.QtObject);
                    //                }
                }
            }

        }

        // Agent Name
        Text {
            id : agentName
            anchors {
                left : parent.left
                leftMargin: 20
                right : parent.right
                rightMargin: 20
                verticalCenter: parent.top
                verticalCenterOffset: 20
            }


            elide: Text.ElideRight
            text : rootItem.agentName

            color : (rootItem.agentMappingVM && rootItem.agentMappingVM.isON)? MasticTheme.agentsONNameMappingColor : MasticTheme.agentsOFFNameMappingColor
            font: MasticTheme.headingFont
        }



        // warnings
        Rectangle {
            height : 18
            width : height
            radius : height/2

            //visible: (model && model.models) ? (model.models.count > 1) : false

            anchors {
                verticalCenter: agentName.verticalCenter
                verticalCenterOffset: 1
                right : parent.left
                rightMargin: 41
            }

            color : MasticTheme.redColor

            Text {
                anchors.centerIn : parent
                text: "1" //(model && model.models) ? model.models.count : ""

                color : MasticTheme.whiteColor
                font {
                    family: MasticTheme.labelFontFamilyBlack
                    bold : true
                    pixelSize : 14
                }
            }
        }


        //Separator
        Rectangle {
            id : separator
            anchors {
                left : parent.left
                leftMargin: 18
                right : parent.right
                rightMargin: 18
                verticalCenter : parent.top
                verticalCenterOffset: 40
            }

            height : 2
            color : agentName.color

            visible : !rootItem.isReduced
        }



        //
        //
        // Global Points
        //
        Rectangle {
            id : inputGlobalPoint

            anchors {
                horizontalCenter : parent.left
                verticalCenter: parent.verticalCenter
            }

            height : 13
            width : height
            radius : height/2

            color : if (agentMappingVM) {
                        switch (agentMappingVM.reducedMapValueTypeInInput)
                        {
                        case AgentIOPValueTypes.INTEGER:
                            agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                            break;
                        case AgentIOPValueTypes.DOUBLE:
                            agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                            break;
                        case AgentIOPValueTypes.STRING:
                            agentMappingVM.isON? MasticTheme.redColor2 : MasticTheme.darkRedColor2
                            break;
                        case AgentIOPValueTypes.BOOL:
                            agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                            break;
                        case AgentIOPValueTypes.IMPULSION:
                            agentMappingVM.isON? MasticTheme.purpleColor : MasticTheme.darkPurpleColor
                            break;
                        case AgentIOPValueTypes.DATA:
                            agentMappingVM.isON? MasticTheme.greenColor : MasticTheme.darkGreenColor
                            break;
                        case AgentIOPValueTypes.MIXED:
                            agentMappingVM.isON? MasticTheme.whiteColor : MasticTheme.greyColor4
                            break;
                        case AgentIOPValueTypes.UNKNOWN:
                            "#000000"
                            break;
                        default:
                            MasticTheme.whiteColor;
                            break;
                        }
                    } else {
                        MasticTheme.whiteColor
                    }

            visible : rootItem.isReduced && rootItem.agentMappingVM && rootItem.agentMappingVM.inputsList.count > 0
        }

        Rectangle {
            id : outputGlobalPoint

            anchors {
                horizontalCenter : parent.right
                verticalCenter: parent.verticalCenter
            }

            height : 13
            width : height
            radius : height/2

            color : if (agentMappingVM) {
                        switch (agentMappingVM.reducedMapValueTypeInOutput)
                        {
                        case AgentIOPValueTypes.INTEGER:
                            agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                            break;
                        case AgentIOPValueTypes.DOUBLE:
                            agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                            break;
                        case AgentIOPValueTypes.STRING:
                            agentMappingVM.isON? MasticTheme.redColor2 : MasticTheme.darkRedColor2
                            break;
                        case AgentIOPValueTypes.BOOL:
                            agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                            break;
                        case AgentIOPValueTypes.IMPULSION:
                            agentMappingVM.isON? MasticTheme.purpleColor : MasticTheme.darkPurpleColor
                            break;
                        case AgentIOPValueTypes.DATA:
                            agentMappingVM.isON? MasticTheme.greenColor : MasticTheme.darkGreenColor
                            break;
                        case AgentIOPValueTypes.MIXED:
                            agentMappingVM.isON? MasticTheme.whiteColor : MasticTheme.greyColor4
                            break;
                        case AgentIOPValueTypes.UNKNOWN:
                            "#000000"
                            break;
                        default:
                            MasticTheme.whiteColor;
                            break;
                        }
                    } else {
                        MasticTheme.whiteColor
                    }

            visible : rootItem.isReduced && rootItem.agentMappingVM && rootItem.agentMappingVM.outputsList.count > 0
        }


        //
        //
        // Inlets / Input slots
        //
        Column {
            id: columnInputSlots

            anchors {
                left: parent.left
                right : parent.right

                top: separator.bottom
                topMargin: 5
                bottom: parent.bottom
            }

            visible : !rootItem.isReduced

            Repeater {
                // List of intput slots VM
                model: rootItem.agentMappingVM ? rootItem.agentMappingVM.inputsList : 0

                delegate: Item {
                    id: inputSlotItem

                    property var myModel: model.QtObject

                    height : 20
                    anchors {
                        left : parent.left
                        right : parent.right
                    }

                    Text {
                        id : agentInput
                        anchors {
                            left : parent.left
                            leftMargin: 20
                            right : parent.horizontalCenter
                            rightMargin: 5
                            verticalCenter: parent.verticalCenter
                        }
                        elide: Text.ElideRight
                        text : myModel.modelM ? myModel.modelM.name : ""

                        color : (rootItem.agentMappingVM && rootItem.agentMappingVM.isON)? MasticTheme.agentsONInputsOutputsMappingColor : MasticTheme.agentsOFFInputsOutputsMappingColor
                        font: MasticTheme.heading2Font
                    }

                    Rectangle {
                        id : draggablePointFROM

                        height : linkPoint.height
                        width : height
                        radius : height/2

                        property bool dragActive : mouseAreaPointFROM.drag.active;

                        Drag.active: mouseAreaPointFROM.drag.active;
                        Drag.hotSpot.x: width/2
                        Drag.hotSpot.y: height/2

                        border {
                            width : 1
                            color : draggablePointFROM.dragActive? linkPoint.color : "transparent"
                        }


                        color : draggablePointFROM.dragActive? MasticTheme.agentsMappingBackgroundColor : "transparent"
                        parent : draggablePointFROM.dragActive? rootItem.parent  : linkPoint

                        MouseArea {
                            id: mouseAreaPointFROM
                            anchors.fill: parent
                            drag.target: parent

                            hoverEnabled: true

                            drag.smoothed: false
                            cursorShape: (draggablePointFROM.dragActive)? Qt.ClosedHandCursor : Qt.PointingHandCursor //Qt.OpenHandCursor

                            onPressed: {
                                draggablePointFROM.z = rootItem.parent.maxZ++;
                                linkDraggablePoint.z = rootItem.parent.maxZ++;
                            }

                            onReleased: {
                                // replace the draggablePointTO
                                draggablePointFROM.x = 0
                                draggablePointFROM.y = 0
                            }
                        }

                        Link {
                            id : linkDraggablePoint
                            parent : rootItem.parent
                            visible: draggablePointFROM.dragActive
                            secondPoint: Qt.point(myModel.position.x + draggablePointFROM.width/2 , myModel.position.y)
                            firstPoint: Qt.point(draggablePointFROM.x + draggablePointFROM.width, draggablePointFROM.y + draggablePointFROM.height/2)

                            defaultColor:linkPoint.color
                        }
                    }


                    Rectangle {
                        id : linkPoint

                        anchors {
                            horizontalCenter: parent.left
                            verticalCenter: parent.verticalCenter
                        }

                        height : 13
                        width : height
                        radius : height/2

                        border {
                            width : 0
                            color : MasticTheme.lightGreyColor // "#DADADA"
                        }

                        color : if (agentMappingVM && myModel && myModel.modelM) {

                                    switch (myModel.modelM.agentIOPValueType)
                                    {
                                    case AgentIOPValueTypes.INTEGER:
                                        agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                                        break;
                                    case AgentIOPValueTypes.DOUBLE:
                                        agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                                        break;
                                    case AgentIOPValueTypes.STRING:
                                        agentMappingVM.isON? MasticTheme.redColor2 : MasticTheme.darkRedColor2
                                        break;
                                    case AgentIOPValueTypes.BOOL:
                                        agentMappingVM.isON? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                                        break;
                                    case AgentIOPValueTypes.IMPULSION:
                                        agentMappingVM.isON? MasticTheme.purpleColor : MasticTheme.darkPurpleColor
                                        break;
                                    case AgentIOPValueTypes.DATA:
                                        agentMappingVM.isON? MasticTheme.greenColor : MasticTheme.darkGreenColor
                                        break;
                                    case AgentIOPValueTypes.MIXED:
                                        agentMappingVM.isON? MasticTheme.whiteColor : MasticTheme.greyColor4
                                        break;
                                    case AgentIOPValueTypes.UNKNOWN:
                                        "#000000"
                                        break;
                                    default:
                                        "#000000"
                                        break;
                                    }

                                } else {
                                    MasticTheme.whiteColor
                                }
                    }


                    DropArea {
                        id: inputDropArea

                        anchors.fill: linkPoint

                        onEntered: {
                            console.log("drag enter ")

                            if (drag.source !== null)
                            {
                                var dragItem = drag.source;

                                console.log("source "+dragItem)
                                if (typeof dragItem.dragActive !== 'undefined')
                                {
                                    dragItem.color = dragItem.border.color;
                                    linkPoint.border.width = 2
                                }
                                else
                                {
                                    console.log("no dragActive "+dragItem.agent)
                                }
                            }
                            else
                            {
                                console.log("no source "+ drag.source)
                            }
                        }


                        onPositionChanged: {
                        }


                        onExited: {
                            var dragItem = drag.source;
                            if (typeof dragItem.dragActive !== 'undefined')
                            {
                                dragItem.color = "transparent";
                                linkPoint.border.width = 0
                            }
                        }

                    }



                    //
                    // Bindings to save the anchor point of our input slot
                    // i.e. the point used to draw a link
                    //
                    Binding {
                        target: myModel

                        property: "position"

                        // the position inside the agent is not the same if the agent is reduced or not
                        value:  (rootItem.agentMappingVM && !rootItem.agentMappingVM.isReduced) ?
                                    (Qt.point(rootItem.x + columnInputSlots.x + inputSlotItem.x + linkPoint.x + linkPoint.width/2,
                                              rootItem.y + columnInputSlots.y + inputSlotItem.y + linkPoint.y + linkPoint.height/2))
                                  : (Qt.point(rootItem.x + inputGlobalPoint.x + inputGlobalPoint.width/2,
                                              rootItem.y + inputGlobalPoint.y + inputGlobalPoint.height/2));
                    }

                }
            }
        }



        //
        //
        // Outlets / Output slots
        //
        Column {
            id: columnOutputSlots

            anchors {
                left: parent.left
                right : parent.right
                top: separator.bottom
                topMargin: 5
                bottom: parent.bottom
            }

            visible : !rootItem.isReduced

            Repeater {
                // List of output slots VM
                model: (rootItem.agentMappingVM)? rootItem.agentMappingVM.outputsList : 0

                delegate: Item {
                    id: outputSlotItem

                    property var myModel: model.QtObject

                    height : 20
                    anchors {
                        left : parent.left
                        right : parent.right
                    }

                    Text {
                        id : agentOutput
                        anchors {
                            left : parent.horizontalCenter
                            leftMargin: 5
                            right : parent.right
                            rightMargin: 20
                            verticalCenter: parent.verticalCenter
                        }

                        horizontalAlignment : Text.AlignRight

                        elide: Text.ElideRight
                        text : myModel.modelM ? myModel.modelM.name : ""

                        color : (rootItem.agentMappingVM && rootItem.agentMappingVM.isON)? MasticTheme.agentsONInputsOutputsMappingColor : MasticTheme.agentsOFFInputsOutputsMappingColor
                        font: MasticTheme.heading2Font
                    }



                    Rectangle {
                        id : draggablePointTO

                        height : linkPointOut.height
                        width : height
                        radius : height/2

                        border {
                            width : 1
                            color : draggablePointTO.dragActive? linkPointOut.color : "transparent"
                        }

                        property bool dragActive : mouseAreaPointTO.drag.active;

                        Drag.active: draggablePointTO.dragActive;
                        Drag.hotSpot.x: 0
                        Drag.hotSpot.y: 0

                        color : draggablePointTO.dragActive? MasticTheme.agentsMappingBackgroundColor : "transparent"
                        parent : draggablePointTO.dragActive? rootItem.parent  : linkPointOut

                        MouseArea {
                            id: mouseAreaPointTO
                            anchors.fill: parent
                            drag.target: parent

                            hoverEnabled: true

                            drag.smoothed: false
                            cursorShape: (draggablePointTO.dragActive)? Qt.ClosedHandCursor : Qt.PointingHandCursor //Qt.OpenHandCursor

                            onPressed: {
                                draggablePointTO.z = rootItem.parent.maxZ++;
                                linkDraggablePointTO.z = rootItem.parent.maxZ++;
                            }

                            onReleased: {
                                // replace the draggablePointTO
                                draggablePointTO.x = 0
                                draggablePointTO.y = 0
                            }
                        }



                        Link {
                            id : linkDraggablePointTO
                            parent : rootItem.parent
                            visible: draggablePointTO.dragActive
                            firstPoint: Qt.point(myModel.position.x + draggablePointTO.width/2 , myModel.position.y)
                            secondPoint: Qt.point(draggablePointTO.x,draggablePointTO.y + draggablePointTO.height/2)

                            defaultColor:linkPointOut.color
                        }
                    }



                    Rectangle {
                        id : linkPointOut

                        anchors {
                            horizontalCenter: parent.right
                            verticalCenter: parent.verticalCenter
                        }

                        height : 13
                        width : height
                        radius : height/2

                        color : if (agentMappingVM && myModel && myModel.modelM) {

                                    switch (myModel.modelM.agentIOPValueType)
                                    {
                                    case AgentIOPValueTypes.INTEGER:
                                        (agentMappingVM.isON && !myModel.modelM.isMuted) ? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                                        break;
                                    case AgentIOPValueTypes.DOUBLE:
                                        (agentMappingVM.isON && !myModel.modelM.isMuted)? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                                        break;
                                    case AgentIOPValueTypes.STRING:
                                        (agentMappingVM.isON && !myModel.modelM.isMuted)? MasticTheme.redColor2 : MasticTheme.darkRedColor2
                                        break;
                                    case AgentIOPValueTypes.BOOL:
                                        (agentMappingVM.isON && !myModel.modelM.isMuted)? MasticTheme.orangeColor2 : MasticTheme.darkOrangeColor2
                                        break;
                                    case AgentIOPValueTypes.IMPULSION:
                                        (agentMappingVM.isON && !myModel.modelM.isMuted)? MasticTheme.purpleColor : MasticTheme.darkPurpleColor
                                        break;
                                    case AgentIOPValueTypes.DATA:
                                        (agentMappingVM.isON && !myModel.modelM.isMuted)? MasticTheme.greenColor : MasticTheme.darkGreenColor
                                        break;
                                    case AgentIOPValueTypes.MIXED:
                                        (agentMappingVM.isON && !myModel.modelM.isMuted)? MasticTheme.whiteColor : MasticTheme.greyColor4
                                        break;
                                    case AgentIOPValueTypes.UNKNOWN:
                                        "#000000"
                                        break;
                                    default:
                                        "#000000"
                                        break;
                                    }

                                } else {
                                    MasticTheme.whiteColor
                                }



                        I2SvgItem {
                            anchors.centerIn: parent
                            svgFileCache: MasticTheme.svgFileMASTIC
                            svgElementId: "outputIsMuted"

                            visible : myModel.modelM && myModel.modelM.isMuted
                        }
                    }


                    //
                    // Bindings to save the anchor point of our input slot
                    // i.e. the point used to draw a link
                    //
                    Binding {
                        target: myModel

                        property: "position"

                        // the position inside the agent is not the same if the agent is reduced or not
                        value: (rootItem.agentMappingVM && !rootItem.agentMappingVM.isReduced) ?
                                   (Qt.point(rootItem.x + columnOutputSlots.x + outputSlotItem.x + linkPointOut.x + linkPointOut.width/2,
                                             rootItem.y + columnOutputSlots.y + outputSlotItem.y + linkPointOut.y + linkPointOut.height/2))
                                 : (Qt.point(rootItem.x + outputGlobalPoint.x + outputGlobalPoint.width/2,
                                             rootItem.y + outputGlobalPoint.y + outputGlobalPoint.height/2));
                    }
                }
            }
        }

    }



}

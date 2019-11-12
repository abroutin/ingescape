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
import "../theme" as Theme;
import ".." as Editor;

import QtQuick.Controls 2.0 as Controls2


WindowBlockTouches {
    id: rootItem

    title: actionM ? actionM.name : "Action"

    width: 700
    height: minimumHeight

    minimumWidth: 475
    minimumHeight: IngeScapeEditorTheme.appMinHeight

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

    // action model
    property var actionM: panelController ? panelController.editedAction : null;

    // action view model
    property var actionVM: panelController ? panelController.editedViewModel : null;

    property var allAgentsGroupsByName: IngeScapeEditorC.modelManager ? IngeScapeEditorC.modelManager.allAgentsGroupsByName : null;

    // our scenario controller
    property var controller: null;

    // our panel controller
    property var panelController: null;

    property var heightStartTime: (startTimeItem.visible ? (startTimeItem.height + startTimeItem.anchors.topMargin) : 0)


    //--------------------------------
    //
    // Signals
    //
    //--------------------------------



    //--------------------------------------------------------
    //
    //
    // Behaviors
    //
    //
    //--------------------------------------------------------

    Connections {
        target: panelController

        //ignoreUnknownSignals: true

        onBringToFront: {
            //console.log("QML of Action Editor: onBringToFront");

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
            width: 1
            color: IngeScapeTheme.editorsBackgroundBorderColor
        }
        color: IngeScapeTheme.editorsBackgroundColor


        Item {
            id: mainItem

            anchors {
                fill : parent
                margins : 20
            }

            focus:true

            Text {
                id: txtTitle

                anchors {
                    left: parent.left
                    top: parent.top
                    right: parent.right
                }

                text: actionVM ? qsTr("Action in timeline") : qsTr("Action")

                elide: Text.ElideRight
                color: IngeScapeTheme.whiteColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    pixelSize: 23
                    weight: Font.Medium
                }
            }

            // Only for DEBUG: display action UID
            Text {
                id: txtUID

                anchors {
                    right: btnCloseEditor.left
                    rightMargin: 20
                    top: parent.top
                }

                visible: IngeScapeEditorC.isAvailableModelVisualizer

                text: rootItem.actionM ? "(uid=" + rootItem.actionM.uid + ")" : ""

                color: IngeScapeTheme.whiteColor
                font {
                    family: IngeScapeTheme.textFontFamily
                    pixelSize: 16
                }
            }

            LabellessSvgButton {
                id: btnCloseEditor

                anchors {
                    top: parent.top
                    right: parent.right
                }

                pressedID: releasedID + "-pressed"
                releasedID: "closeEditor"
                disabledID : releasedID


                onClicked: {
                    // Close our popup
                    rootItem.close();
                }
            }

            //
            // Name
            //
            Item {
                id : nameItem
                anchors {
                    left : parent.left
                    right : parent.right
                    top: txtTitle.bottom
                    topMargin : 25
                }

                height: textFieldName.height

                Connections {
                    target: rootItem

                    Component.onCompleted: {
                        // Set focus on action's name if new action is edited
                        if (rootItem.active && panelController && panelController.originalAction === null) {
                            textFieldName.forceActiveFocus();
                        }
                    }
                }

                TextField {
                    id: textFieldName

                    anchors {
                        left : parent.left
                        verticalCenter : parent.verticalCenter
                    }

                    height: 25
                    width: 435

                    verticalAlignment: TextInput.AlignVCenter
                    text: actionM ? actionM.name : ""

                    style: I2TextFieldStyle {
                        backgroundColor: IngeScapeTheme.darkBlueGreyColor
                        borderColor: IngeScapeTheme.whiteColor;
                        borderErrorColor: IngeScapeTheme.redColor
                        radiusTextBox: 1
                        borderWidth: 0;
                        borderWidthActive: 1
                        textIdleColor: IngeScapeTheme.whiteColor;
                        textDisabledColor: IngeScapeTheme.darkGreyColor;

                        padding.left: 3
                        padding.right: 3

                        font {
                            pixelSize:15
                            family: IngeScapeTheme.textFontFamily
                        }

                    }

                    onTextChanged: {
                        if (activeFocus &&  actionM ) {
                            actionM.name = text;
                        }
                    }

                    onActiveFocusChanged: {
                        if (!activeFocus) {
                            // Move cursor to our first character when we lose focus
                            // (to always display the beginning or our text instead of
                            // an arbitrary part if our text is too long)
                            cursorPosition = 0;
                        }
                        else {
                            textFieldName.selectAll();
                        }
                    }

                    Binding {
                        target: textFieldName
                        property: "text"
                        value: (actionM ? actionM.name : "")
                    }
                }
            }


            //
            // Start Time
            //
            Item {
                id : startTimeItem

                anchors {
                    left : parent.left
                    right : parent.right
                    top : nameItem.bottom
                    topMargin : 15
                }
                height : textFieldStartTime.height

                visible : actionVM !== null

                Text {
                    id : textstartTime
                    anchors {
                        left : parent.left
                        verticalCenter : parent.verticalCenter
                    }

                    text : "Start Time:"

                    color: IngeScapeTheme.lightGreyColor
                    font {
                        family: IngeScapeTheme.textFontFamily
                        pixelSize: 16
                    }
                }


                TextField {
                    id: textFieldStartTime

                    anchors {
                        left : textstartTime.right
                        leftMargin: 10
                        verticalCenter : parent.verticalCenter
                    }

                    height: 25
                    width: 105
                    verticalAlignment: TextInput.AlignVCenter
                    text: actionVM ? actionVM.startTimeString : "00:00:00.000"
                    inputMask: "00:00:00.000"
                    inputMethodHints: Qt.ImhTime
                    validator: RegExpValidator { regExp: /^(?:(?:([01]?\d|2[0-3]):)?([0-5]?\d):)?([0-5]?\d).([0-9\s][0-9\s][0-9\s])$/ }

                    style: I2TextFieldStyle {
                        backgroundColor: IngeScapeTheme.darkBlueGreyColor
                        borderColor: IngeScapeTheme.whiteColor;
                        borderErrorColor: IngeScapeTheme.redColor
                        radiusTextBox: 1
                        borderWidth: 0;
                        borderWidthActive: 1
                        textIdleColor: IngeScapeTheme.whiteColor;
                        textDisabledColor: IngeScapeTheme.darkGreyColor;

                        padding.left: 3
                        padding.right: 3

                        font {
                            pixelSize:15
                            family: IngeScapeTheme.textFontFamily
                        }

                    }

                    onActiveFocusChanged: {
                        if (!activeFocus) {
                            // Move cursor to our first character when we lose focus
                            // (to always display the beginning or our text instead of
                            // an arbitrary part if our text is too long)
                            cursorPosition = 0;
                        }
                        else {
                            textFieldStartTime.selectAll();
                        }
                    }


                    onTextChanged: {
                        if (activeFocus && actionVM) {
                            actionVM.startTimeString = text;
                        }
                    }

                    Binding {
                        target: textFieldStartTime
                        property: "text"
                        value: (actionVM ? actionVM.startTimeString : "")
                    }
                }

            }


            //
            // Effects
            //
            Item {
                id : effectsListItem
                anchors {
                    left : parent.left
                    right : parent.right
                    top : (startTimeItem.visible) ? startTimeItem.bottom : nameItem.bottom
                    topMargin: 15
                }
                height: Math.min(titleEffects.height + 6 + scrollView.anchors.topMargin + effectsList.contentHeight + scrollView.anchors.bottomMargin + addEffects.height,
                                 mainItem.height - (txtTitle.height + nameItem.height + nameItem.anchors.topMargin + heightStartTime + effectsListItem.anchors.topMargin + conditionsItem.height + conditionsItem.anchors.topMargin + advancedModesItem.height + advancedModesItem.anchors.topMargin + 10 + okButton.height))

                Behavior on height {
                    NumberAnimation {}
                }

//                Rectangle {
//                    color: "red"
//                    anchors.fill: parent
//                }

                // Title
                Text {
                    id : titleEffects
                    anchors {
                        left : parent.left
                        right : parent.right
                    }

                    text : "Effects"

                    color: IngeScapeTheme.whiteColor
                    font {
                        family: IngeScapeTheme.textFontFamily
                        pixelSize: 19
                    }
                }

                // separator
                Rectangle {
                    id : separatorEffect
                    anchors {
                        left : parent.left
                        right : parent.right
                        top : titleEffects.bottom
                        topMargin: 5
                    }
                    height : 1
                    color : IngeScapeTheme.whiteColor
                }

                // Effects List
                ScrollView {
                    id : scrollView

                    anchors {
                        top : separatorEffect.bottom
                        topMargin: 6
                        right : parent.right
                        left : parent.left
                        bottom : addEffects.top
                        bottomMargin: 6
                    }

                    style: IngeScapeScrollViewStyle {
                    }

                    // Prevent drag overshoot on Windows
                    flickableItem.boundsBehavior: Flickable.OvershootBounds

                    // Effects List
                    ListView {
                        id: effectsList

                        spacing: 6

                        model: actionM ? actionM.effectsList : 0

                        delegate: componentEffectsListItem
                    }
                }

                // Add effect button
                LabellessSvgButton {
                    id: addEffects

                    anchors {
                        left: parent.left
                        bottom: parent.bottom
                    }

                    pressedID: releasedID + "-pressed"
                    releasedID: "createButton"
                    disabledID : releasedID


                    onClicked: {
                        if (panelController)
                        {
                            panelController.createNewEffect();
                        }
                    }
                }

                // Component effect list item
                Component {
                    id: componentEffectsListItem

                    // Effects items are draggable to reorganize list order
                    Editor.DraggableListItem {
                        id: effectListItem

                        keysDragNDrop: ["OrganizeEffectsList"]

                        // Reorganize actions list model when an item is moved
                        onMoveItemRequested: {
                            actionM.effectsList.move(from, to, 1);
                        }

                        property var myEffect: model.QtObject
//                        property var myEffectIopIsNotImpulsion: myEffect && (myEffect.effectType === ActionEffectTypes.VALUE) && ( myEffect.modelM && myEffect.modelM.agentIOP && myEffect.modelM.agentIOP.firstModel && (myEffect.modelM.agentIOP.firstModel.agentIOPValueType !== AgentIOPValueTypes.IMPULSION) )
//                        property var myEffectIopIsBool: myEffect && (myEffect.effectType === ActionEffectTypes.VALUE) && ( myEffect.modelM && myEffect.modelM.agentIOP && myEffect.modelM.agentIOP.firstModel && (myEffect.modelM.agentIOP.firstModel.agentIOPValueType === AgentIOPValueTypes.BOOL) )

                        Rectangle {
                            height: (myEffect && (myEffect.effectType === ActionEffectTypes.MAPPING)) ? 90 : 62
                            width: scrollView.width - 9 // scrollbar size

                            color : IngeScapeTheme.editorsBackgroundColor
                            radius: 5
                            border {
                                width : 1
                                color : IngeScapeTheme.blackColor
                            }

                            // Effect Type
                            Row {
                                id : rowEffectsTypes

                                anchors {
                                    right : parent.right
                                    left : parent.left
                                    leftMargin: 10
                                    top : parent.top
                                    topMargin: 6
                                }
                                height : 14
                                spacing : 15

                                ExclusiveGroup {
                                    id : effectTypesExclusifGroup
                                }

                                Repeater {
                                    model : controller ? controller.effectsTypesList : 0

                                    CheckBox {
                                        id : effectTypeCB
                                        anchors {
                                            verticalCenter: parent.verticalCenter;
                                        }

                                        checked: myEffect && (myEffect.effectType === model.value)
                                        exclusiveGroup: effectTypesExclusifGroup
                                        activeFocusOnPress: true;

                                        style: CheckBoxStyle {
                                            label: Text {
                                                anchors {
                                                    verticalCenter: parent.verticalCenter
                                                    verticalCenterOffset: 2
                                                }
                                                color: control.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor

                                                text: model.name
                                                elide: Text.ElideRight

                                                font {
                                                    family: IngeScapeTheme.textFontFamily
                                                    pixelSize: 15
                                                }
                                            }

                                            indicator: Rectangle {
                                                implicitWidth: 14
                                                implicitHeight: 14
                                                radius: height / 2
                                                border.width: 0
                                                color: IngeScapeTheme.darkBlueGreyColor

                                                Rectangle {
                                                    anchors.centerIn: parent
                                                    visible : control.checked
                                                    width: 8
                                                    height: 8
                                                    radius: height / 2

                                                    border.width: 0
                                                    color: IngeScapeTheme.whiteColor
                                                }
                                            }

                                        }

                                        onCheckedChanged: {
                                            if (myEffect && checked) {
                                                myEffect.effectType = model.value
                                            }
                                        }

                                        Binding {
                                            target: effectTypeCB
                                            property: "checked"
                                            value: (myEffect && (myEffect.effectType === model.value))
                                        }
                                    }
                                }
                            }


                            //
                            // Effect Details for Agent and Value
                            //
                            Item {
                                anchors {
                                    right: parent.right
                                    rightMargin: 10
                                    left: rowEffectsTypes.left
                                    bottom: parent.bottom
                                    bottomMargin: 6
                                }
                                height: 25

                                visible: (myEffect && myEffect.effectType !== ActionEffectTypes.MAPPING)

                                // Agent
                                IngeScapeComboboxItemModel {
                                    id: agentEffectCombo

                                    anchors {
                                        left: parent.left
                                        verticalCenter : parent.verticalCenter
                                    }
                                    height: parent.height
                                    width: 148

                                    model: rootItem.allAgentsGroupsByName

                                    Binding {
                                        target: agentEffectCombo
                                        property: "selectedIndex"
                                        value: (myEffect && myEffect.modelM && rootItem.allAgentsGroupsByName) ? rootItem.allAgentsGroupsByName.indexOf(myEffect.modelM.agent)
                                                                                                                : -1
                                    }

                                    onSelectedItemChanged: {
                                        if ((agentEffectCombo.selectedIndex >= 0) && myEffect && myEffect.modelM)
                                        {
                                            myEffect.modelM.agent = agentEffectCombo.selectedItem;

                                            // Register agent selected to choose it for the next effect or condition
                                            // Do it only here, because it is updated everytime agent model of effect change
                                            if (rootItem.panelController) {
                                                rootItem.panelController.lastAgentSelected = agentEffectCombo.selectedItem;
                                            }
                                        }
                                    }
                                }

                                // Agent's Inputs/Outputs/Parameters
                                IngeScapeComboboxItemModelAgentsIOP {
                                    id: iopEffectsCombo

                                    anchors {
                                        left: agentEffectCombo.right
                                        leftMargin: 6
                                        verticalCenter : parent.verticalCenter
                                    }

                                    height: 25
                                    width: 148

                                    visible: myEffect && (myEffect.effectType === ActionEffectTypes.VALUE)
                                    enabled: visible

                                    inputsNumber: (myEffect && myEffect.modelM && (myEffect.effectType === ActionEffectTypes.VALUE)) ? myEffect.modelM.inputsNumber : 0
                                    outputsNumber: (myEffect && myEffect.modelM && (myEffect.effectType === ActionEffectTypes.VALUE)) ? myEffect.modelM.outputsNumber : 0
                                    parametersNumber: (myEffect && myEffect.modelM && (myEffect.effectType === ActionEffectTypes.VALUE)) ? myEffect.modelM.parametersNumber : 0

                                    model: (myEffect && myEffect.modelM) ? myEffect.modelM.iopMergedList : 0

                                    Binding {
                                        target: iopEffectsCombo
                                        property: "selectedIndex"
                                        value: (myEffect && myEffect.modelM && myEffect.modelM.agentIOP) ? myEffect.modelM.iopMergedList.indexOf(myEffect.modelM.agentIOP)
                                                                                                         : -1
                                    }

                                    onSelectedItemChanged: {
                                        if ((iopEffectsCombo.selectedIndex >= 0) && myEffect && myEffect.modelM)
                                        {
                                            myEffect.modelM.agentIOP = iopEffectsCombo.selectedItem;

                                            // Register agent IOP selected to choose it for the next effect or condition
                                            if (rootItem.panelController) {
                                                panelController.lastIOPSelected = iopEffectsCombo.selectedItem;
                                            }

                                            // Revalidate text field and combo entry regarding which one is visible and the type of the selected IOP.
                                            userInputItem.revalidateInput()
                                        }
                                    }
                                }

                                // Target Value
                                InputIOPValueField {
                                    id: userInputItem

                                    anchors {
                                        left: iopEffectsCombo.right
                                        leftMargin: 6

                                        right: (btnWarningActionEditor.visible ? btnWarningActionEditor.left : parent.right)
                                        rightMargin: (btnWarningActionEditor.visible ? 6 : 0)

                                        verticalCenter: parent.verticalCenter
                                    }

                                    iopVM: (effectListItem.myEffect && effectListItem.myEffect.modelM) ? effectListItem.myEffect.modelM.agentIOP : undefined
                                    forceHide: !effectListItem.myEffect || (effectListItem.myEffect.effectType !== ActionEffectTypes.VALUE)

                                    function getModelValue() {
                                        if (effectListItem.myEffect && effectListItem.myEffect.modelM)
                                        {
                                            return effectListItem.myEffect.modelM.value
                                        }
                                        else
                                        {
                                            return ""
                                        }

                                    }

                                    function setModelValue(value) {
                                        if (effectListItem.myEffect && effectListItem.myEffect.modelM)
                                        {
                                            effectListItem.myEffect.modelM.value = value
                                        }
                                    }
                                }

                                LabellessSvgButton {
                                    id: btnWarningActionEditor

                                    anchors {
                                        right: parent.right
                                        rightMargin: 0
                                        verticalCenter: parent.verticalCenter
                                    }

                                    visible: myEffect && (myEffect.effectType === ActionEffectTypes.VALUE) && myEffect.modelM && myEffect.modelM.agentIOP && myEffect.modelM.agentIOP.firstModel && (myEffect.modelM.agentIOP.firstModel.agentIOPType !== AgentIOPTypes.OUTPUT)


                                    checkable: true

                                    pressedID: releasedID + "-pressed"
                                    releasedID: "warningActionEditor"
                                    disabledID : releasedID


                                    onClicked: {
                                        infosBulle.open();
                                    }



                                    I2PopupBase {
                                        id: infosBulle

                                        height : backgroundPopup.height
                                        width : backgroundPopup.width
                                        keepRelativePositionToInitialParent: true
                                        layerColor : 'transparent'

                                        onOpened: {
                                            infosBulle.visible = true
                                            infosBulle.x = infosBulle.x - infosBulle.width + 25
                                            infosBulle.y = infosBulle.y - infosBulle.height - 3
                                        }

                                        onClosed: {
                                            // the pop up moves otherwise before being not visible
                                            infosBulle.visible = false
                                        }

                                        I2SvgItem {
                                            id : backgroundPopup

                                            svgFileCache: IngeScapeTheme.svgFileIngeScape
                                            svgElementId: "tooltip"
                                        }

                                        Text {
                                            id : textInfos
                                            anchors {
                                                fill : parent
                                                margins: 6
                                            }

                                            color : IngeScapeTheme.veryDarkGreyColor
                                            wrapMode: Text.Wrap
                                            text : qsTr("Writing to inputs or parameters cannot be reverted.")
                                            font {
                                                family : IngeScapeTheme.textFontFamily
                                                pixelSize: 13
                                            }
                                        }
                                    }

                                }

                                // Effect Type (on Agent)
                                IngeScapeComboboxItemModel {
                                    id: effectTypeCombo

                                    anchors {
                                        left: agentEffectCombo.right
                                        leftMargin: 6
                                        verticalCenter : parent.verticalCenter
                                    }
                                    height: 25
                                    width: 98

                                    visible: (myEffect && (myEffect.effectType === ActionEffectTypes.AGENT))
                                    enabled: visible

                                    model: (controller ? controller.agentEffectValuesList : 0)

                                    Binding {
                                        target: effectTypeCombo
                                        property: "selectedIndex"
                                        value: (myEffect && myEffect.modelM && controller) ? controller.agentEffectValuesList.indexOfEnumValue(myEffect.modelM.agentEffectValue)
                                                                                           : -1
                                    }

                                    onSelectedItemChanged: {
                                        if ((effectTypeCombo.selectedIndex >= 0) && myEffect && myEffect.modelM)
                                        {
                                            myEffect.modelM.agentEffectValue = effectTypeCombo.selectedItem.value;
                                        }
                                    }
                                }
                            }

                            //
                            // Effect Details for Mapping
                            //
                            Item {
                                anchors {
                                    right : parent.right
                                    rightMargin: 10
                                    left : rowEffectsTypes.left
                                    bottom : parent.bottom
                                    bottomMargin: 6
                                }
                                visible: (myEffect && myEffect.effectType === ActionEffectTypes.MAPPING)

                                // Output Agent
                                IngeScapeComboboxItemModel {
                                    id: comboEffectOnMapping_OutputAgent

                                    anchors {
                                        left : parent.left
                                        bottom : comboEffectOnMapping_Output.top
                                        bottomMargin: 6
                                    }

                                    height : 25
                                    width : 148

                                    model: rootItem.allAgentsGroupsByName

                                    enabled: (rootItem.allAgentsGroupsByName && (rootItem.allAgentsGroupsByName.count > 0))
                                    placeholderText: (rootItem.allAgentsGroupsByName && (rootItem.allAgentsGroupsByName.count === 0) ? "- No Item -"
                                                                                                                                       : "- Select an item -")

                                    Binding {
                                        target: comboEffectOnMapping_OutputAgent
                                        property: "selectedIndex"
                                        value: (myEffect && myEffect.modelM && rootItem.allAgentsGroupsByName) ? rootItem.allAgentsGroupsByName.indexOf(myEffect.modelM.outputAgent)
                                                                                                               : -1
                                    }

                                    onSelectedItemChanged: {
                                        if ((comboEffectOnMapping_OutputAgent.selectedIndex >= 0) && myEffect && myEffect.modelM)
                                        {
                                            myEffect.modelM.outputAgent = comboEffectOnMapping_OutputAgent.selectedItem;
                                        }
                                    }
                                }

                                // Outputs (of output agent)
                                IngeScapeComboboxItemModelAgentsIOP {
                                    id: comboEffectOnMapping_Output

                                    anchors {
                                        left : parent.left
                                        bottom : parent.bottom
                                    }
                                    height : 25
                                    width : 148

                                    model : (myEffect && myEffect.modelM) ? myEffect.modelM.outputsList : 0
                                    enabled: visible

                                    Binding {
                                        target: comboEffectOnMapping_Output
                                        property: "selectedIndex"
                                        value: (myEffect && myEffect.modelM && myEffect.modelM.outputsList) ? myEffect.modelM.outputsList.indexOf(myEffect.modelM.output)
                                                                             : -1
                                    }

                                    onSelectedItemChanged: {
                                        if ((comboEffectOnMapping_Output.selectedIndex >= 0) && myEffect && myEffect.modelM)
                                        {
                                            myEffect.modelM.output = comboEffectOnMapping_Output.selectedItem;
                                        }
                                    }
                                }

                                // ON/OFF slider
                                Item {
                                    id: disableMappingItem

                                    anchors {
                                        left : comboEffectOnMapping_OutputAgent.right
                                        right : comboEffectOnMapping_InputAgent.left
                                        top : comboEffectOnMapping_OutputAgent.top
                                        bottom : parent.bottom
                                    }
                                    clip : true

                                    Rectangle {
                                        anchors {
                                            verticalCenter: parent.verticalCenter
                                            right : rectRight.right
                                            left : rectLeft.left
                                        }
                                        color : IngeScapeTheme.blackColor
                                        height : 1
                                    }

                                    Rectangle {
                                        id : rectLeft
                                        anchors {
                                            horizontalCenter: parent.left
                                            top : parent.top
                                            bottom : parent.bottom
                                            topMargin: comboEffectOnMapping_OutputAgent.height/2
                                            bottomMargin: comboEffectOnMapping_OutputAgent.height/2
                                        }
                                        width : 12
                                        color : IngeScapeTheme.veryDarkGreyColor
                                        border {
                                            width: 1
                                            color : IngeScapeTheme.blackColor
                                        }
                                    }

                                    Rectangle {
                                        id : rectRight
                                        anchors {
                                            horizontalCenter: parent.right
                                            top : parent.top
                                            bottom : parent.bottom
                                            topMargin: comboEffectOnMapping_OutputAgent.height/2
                                            bottomMargin: comboEffectOnMapping_OutputAgent.height/2
                                        }
                                        width : 12
                                        color : IngeScapeTheme.veryDarkGreyColor
                                        border {
                                            width: 1
                                            color : IngeScapeTheme.blackColor
                                        }
                                    }

                                    Button {
                                        id : enabledbutton
                                        anchors.centerIn: parent

                                        style: I2SvgToggleButtonStyle {
                                            fileCache: IngeScapeTheme.svgFileIngeScape

                                            toggleCheckedReleasedID: "enabledToggle-checked";
                                            toggleCheckedPressedID: "enabledToggle-checked-pressed";
                                            toggleUncheckedReleasedID: "enabledToggle";
                                            toggleUncheckedPressedID: "enabledToggle-pressed";

                                            // No disabled states
                                            toggleCheckedDisabledID: ""
                                            toggleUncheckedDisabledID: ""

                                            labelMargin: 0;
                                        }

                                        onCheckedChanged: {
                                            if (myEffect && myEffect.modelM)
                                            {
                                                if (checked) {
                                                    myEffect.modelM.mappingEffectValue = MappingEffectValues.MAPPED;
                                                }
                                                else {
                                                    myEffect.modelM.mappingEffectValue = MappingEffectValues.UNMAPPED;
                                                }
                                            }
                                        }

                                        Binding {
                                            target: enabledbutton
                                            property: "checked"
                                            value: (myEffect && myEffect.modelM && (myEffect.modelM.mappingEffectValue === MappingEffectValues.MAPPED)) ? true
                                                                                                                                                        : false
                                        }
                                    }


                                    Text {
                                        anchors {
                                            horizontalCenter: enabledbutton.horizontalCenter
                                            top: enabledbutton.bottom
                                            topMargin: 3
                                        }

                                        text: (myEffect && myEffect.modelM && (myEffect.modelM.mappingEffectValue === MappingEffectValues.MAPPED)) ? "Mapped"
                                                                                                                                                    : "Not mapped"
                                        color: IngeScapeTheme.whiteColor
                                        font {
                                            family: IngeScapeTheme.textFontFamily
                                            pixelSize: 12
                                        }
                                    }
                                }

                                // Input Agent
                                IngeScapeComboboxItemModel {
                                    id: comboEffectOnMapping_InputAgent

                                    anchors {
                                        right : parent.right
                                        bottom : comboEffectOnMapping_Input.top
                                        bottomMargin: 6
                                    }

                                    height : 25
                                    width : 148

                                    model : rootItem.allAgentsGroupsByName

                                    enabled: (rootItem.allAgentsGroupsByName && (rootItem.allAgentsGroupsByName.count > 0))
                                    placeholderText: (rootItem.allAgentsGroupsByName && (rootItem.allAgentsGroupsByName.count === 0) ? "- No Item -"
                                                                                                                                       : "- Select an item -")

                                    Binding {
                                        target: comboEffectOnMapping_InputAgent
                                        property: "selectedIndex"
                                        value: (myEffect && myEffect.modelM && rootItem.allAgentsGroupsByName) ? rootItem.allAgentsGroupsByName.indexOf(myEffect.modelM.agent)
                                                                                                                : -1
                                    }

                                    onSelectedItemChanged: {
                                        if ((comboEffectOnMapping_InputAgent.selectedIndex >= 0) && myEffect && myEffect.modelM)
                                        {
                                            myEffect.modelM.agent = comboEffectOnMapping_InputAgent.selectedItem;
                                        }
                                    }
                                }

                                // Inputs (of input agent)
                                IngeScapeComboboxItemModelAgentsIOP {
                                    id: comboEffectOnMapping_Input

                                    anchors {
                                        right : parent.right
                                        bottom : parent.bottom
                                    }
                                    height : 25
                                    width : 148

                                    model : (myEffect && myEffect.modelM) ? myEffect.modelM.inputsList : 0
                                    enabled: visible

                                    Binding {
                                        target: comboEffectOnMapping_Input
                                        property: "selectedIndex"
                                        value: (myEffect && myEffect.modelM && myEffect.modelM.inputsList) ? myEffect.modelM.inputsList.indexOf(myEffect.modelM.input)
                                                                                                           : -1
                                    }

                                    onSelectedItemChanged: {
                                        if ((comboEffectOnMapping_Input.selectedIndex >= 0) && myEffect && myEffect.modelM)
                                        {
                                            myEffect.modelM.input = comboEffectOnMapping_Input.selectedItem;
                                        }
                                    }
                                }
                            }

                            // Delete Effect
                            LabellessSvgButton {
                                id: btnDeleteEffect

                                height : 10
                                width : 10
                                anchors {
                                    top: parent.top
                                    right : parent.right
                                    margins: 5
                                }

                                pressedID: releasedID + "-pressed"
                                releasedID: "closeEditor"
                                disabledID : releasedID


                                onClicked: {
                                    if (panelController && myEffect)
                                    {
                                        panelController.removeEffect(myEffect);
                                    }
                                }
                            }
                        }
                    }
                }
            }


            //
            // Conditions
            //
            Item {
                id : conditionsItem
                anchors {
                    left : parent.left
                    right : parent.right
                    top : effectsListItem.bottom
                    topMargin: 15
                }
                height: isOpened ? Math.min(230, titleConditionsMouseArea.height + 1 + validityDuration.height + validityDuration.anchors.topMargin + scrollViewConditions.contentItem.height
                                            + scrollViewConditions.anchors.topMargin + scrollViewConditions.anchors.bottomMargin + addCondition.height)
                                 : titleConditionsMouseArea.height + 1
                clip: true

                Behavior on anchors.top {
                    NumberAnimation {}
                }
                Behavior on height {
                    NumberAnimation {}
                }

                property bool isOpened : false

                Connections {
                    target: rootItem

                    Component.onCompleted: {
                        // make the conditions list visible if there are conditions
                        if (rootItem.active && actionM && (actionM.conditionsList.count > 0)) {
                            conditionsItem.isOpened = true;
                        }
                    }
                }

//                Rectangle {
//                    color: "red"
//                    anchors.fill: parent
//                }

                // Title
                MouseArea {
                    id : titleConditionsMouseArea
                    anchors {
                        left : parent.left
                        right : parent.right
                    }
                    height : 29

                    onClicked: {
                        conditionsItem.isOpened = !conditionsItem.isOpened;
                    }

                    I2SvgItem {
                        id : arrow
                        anchors {
                            left: parent.left
                            verticalCenter: titleCdt.verticalCenter
                        }

                        svgFileCache: IngeScapeTheme.svgFileIngeScape
                        svgElementId: "arrowWhite"

                        rotation: conditionsItem.isOpened ? 0 : 270
                    }

                    Text {
                        id : titleCdt
                        anchors {
                            left : arrow.right
                            leftMargin: 6
                            right : parent.right
                            verticalCenter: parent.verticalCenter
                        }

                        text: qsTr("Conditions for this action")

                        color: titleConditionsMouseArea.containsPress ? IngeScapeTheme.lightGreyColor : IngeScapeTheme.whiteColor
                        font {
                            family: IngeScapeTheme.textFontFamily
                            pixelSize: 19
                        }
                    }

                }

                // separator
                Rectangle {
                    id : separatorCdt
                    anchors {
                        left : parent.left
                        right : parent.right
                        top : titleConditionsMouseArea.bottom
                    }
                    height : 1
                    color : IngeScapeTheme.whiteColor
                }

                /// Validity duration
                Item {
                    id : validityDuration
                    anchors {
                        left : parent.left
                        right : parent.right
                        top : separatorCdt.bottom
                        topMargin: 8
                    }

                    height: validityDurationCombo.height

                    visible : conditionsItem.isOpened
                    enabled : visible

                    Text {
                        id : textValidity
                        anchors {
                            left : parent.left
                            verticalCenter : parent.verticalCenter
                        }

                        text : "Conditions shall be checked"

                        color: IngeScapeTheme.whiteColor
                        font {
                            family: IngeScapeTheme.textFontFamily
                            pixelSize: 16
                        }
                    }

                    Row {
                        anchors {
                            verticalCenter : parent.verticalCenter
                            left : textValidity.right
                            leftMargin: 10
                        }
                        spacing : 6

                        IngeScapeComboboxItemModel {
                            id: validityDurationCombo

                            anchors.verticalCenter : parent.verticalCenter
                            height: 25
                            width: 115

                            model: controller ? controller.validationDurationsTypesList : 0

                            Binding {
                                target: validityDurationCombo
                                property: "selectedIndex"
                                value: (actionM && controller) ? controller.validationDurationsTypesList.indexOfEnumValue(actionM.validityDurationType)
                                                               : -1
                            }

                            onSelectedItemChanged: {
                                if (validityDurationCombo.selectedIndex >= 0 && actionM)
                                {
                                    actionM.validityDurationType = validityDurationCombo.selectedItem.value;
                                }
                            }
                        }

                        TextField {
                            id: textFieldValidity

                            anchors {
                                verticalCenter : parent.verticalCenter
                            }

                            visible: actionM && (actionM.validityDurationType === ValidationDurationTypes.CUSTOM)
                            enabled: visible
                            height: 25
                            width: 57
                            horizontalAlignment: TextInput.AlignLeft
                            verticalAlignment: TextInput.AlignVCenter

                            text : actionM ? actionM.validityDurationString : "0.0"
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            validator: RegExpValidator { regExp: /(\d{1,5})([.]\d{3})?$/ }

                            style: I2TextFieldStyle {
                                backgroundColor: IngeScapeTheme.darkBlueGreyColor
                                borderColor: IngeScapeTheme.whiteColor;
                                borderErrorColor: IngeScapeTheme.redColor
                                radiusTextBox: 1
                                borderWidth: 0;
                                borderWidthActive: 1
                                textIdleColor: IngeScapeTheme.whiteColor;
                                textDisabledColor: IngeScapeTheme.darkGreyColor;

                                padding.left: 3
                                padding.right: 3

                                font {
                                    pixelSize:15
                                    family: IngeScapeTheme.textFontFamily
                                }

                            }

                            onActiveFocusChanged: {
                                if (!activeFocus) {
                                    // Move cursor to our first character when we lose focus
                                    // (to always display the beginning or our text instead of
                                    // an arbitrary part if our text is too long)
                                    cursorPosition = 0;
                                }
                                else {
                                    textFieldValidity.selectAll();
                                }
                            }

                            onTextChanged: {
                                if (activeFocus && actionM) {
                                    actionM.validityDurationString = text;
                                }
                            }

                            Binding {
                                target: textFieldValidity
                                property: "text"
                                value: (actionM ? actionM.validityDurationString : "")
                            }
                        }

                        Text {
                            anchors.verticalCenter : parent.verticalCenter

                            text: "seconds"
                            visible: textFieldValidity.visible

                            color: IngeScapeTheme.whiteColor
                            font {
                                family: IngeScapeTheme.textFontFamily
                                pixelSize: 15
                            }
                        }

                    }
                }

                //
                // Conditions List
                //
                ScrollView {
                    id : scrollViewConditions

                    anchors {
                        top: validityDuration.bottom
                        topMargin: 8
                        right: parent.right
                        left: parent.left
                        bottom: addCondition.top
                        bottomMargin: 6
                    }

                    enabled: conditionsItem.isOpened

                    style: IngeScapeScrollViewStyle {
                    }

                    // Prevent drag overshoot on Windows
                    flickableItem.boundsBehavior: Flickable.OvershootBounds


                    //
                    // Conditions List
                    //
                    contentItem: Column {
                        id: conditionsListColumn

                        spacing: 6
                        height: childrenRect.height
                        width: scrollViewConditions.width - 9 // scrollbar size + 1

                        Repeater {
                            model: (actionM && conditionsItem.isOpened) ? actionM.conditionsList : 0

                            Rectangle {
                                id: rectToName

                                // my condition
                                property var myCondition: model.QtObject
                                property bool myConditionTypeIsValue: myCondition && (myCondition.conditionType === ActionConditionTypes.VALUE)
                                property bool myConditionIopIsNotImpulsion: myConditionTypeIsValue && myCondition.modelM && myCondition.modelM.agentIOP && myCondition.modelM.agentIOP.firstModel && (myCondition.modelM.agentIOP.firstModel.agentIOPValueType !== AgentIOPValueTypes.IMPULSION)
                                property bool myConditionIopIsBool: myConditionTypeIsValue && myCondition.modelM && myCondition.modelM.agentIOP && myCondition.modelM.agentIOP.firstModel && (myCondition.modelM.agentIOP.firstModel.agentIOPValueType === AgentIOPValueTypes.BOOL)
                                anchors {
                                    right : parent.right
                                    left : parent.left
                                }
                                height: 62

                                color : "transparent"
                                radius: 5
                                border {
                                    width : 1
                                    color : IngeScapeTheme.blackColor
                                }

                                // Condition Type
                                Row {
                                    id : rowConditionsTypes
                                    anchors {
                                        right : parent.right
                                        left : parent.left
                                        leftMargin: 10
                                        top : parent.top
                                        topMargin: 6
                                    }
                                    height : 14
                                    spacing : 15

                                    ExclusiveGroup {
                                        id : cdtTypesExclusifGroup
                                    }

                                    Repeater {
                                        model: controller ? controller.conditionsTypesList : 0

                                        CheckBox {
                                            id: conditionsTypeCB

                                            anchors.verticalCenter: parent.verticalCenter

                                            checked: myCondition && (myCondition.conditionType === model.value)

                                            exclusiveGroup: cdtTypesExclusifGroup
                                            activeFocusOnPress: true;

                                            style: CheckBoxStyle {
                                                label: Text {
                                                    anchors {
                                                        verticalCenter: parent.verticalCenter
                                                        verticalCenterOffset: 2
                                                    }
                                                    color: control.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor

                                                    text: model.name
                                                    elide: Text.ElideRight

                                                    font {
                                                        family: IngeScapeTheme.textFontFamily
                                                        pixelSize: 15
                                                    }
                                                }

                                                indicator: Rectangle {
                                                    implicitWidth: 14
                                                    implicitHeight: 14
                                                    radius: height / 2
                                                    border.width: 0
                                                    color: IngeScapeTheme.darkBlueGreyColor

                                                    Rectangle {
                                                        anchors.centerIn: parent
                                                        visible : control.checked
                                                        width: 8
                                                        height: 8
                                                        radius: height / 2
                                                        border.width: 0
                                                        color: IngeScapeTheme.whiteColor
                                                    }
                                                }

                                            }

                                            onCheckedChanged: {
                                                if (myCondition && checked) {
                                                    myCondition.conditionType = model.value
                                                }
                                            }


                                            Binding {
                                                target: conditionsTypeCB
                                                property: "checked"
                                                value: (myCondition && (myCondition.conditionType === model.value))
                                            }
                                        }
                                    }
                                }


                                //
                                // Conditions Details
                                //
                                Item {
                                    id: conditionRowItem

                                    anchors {
                                        left: rowConditionsTypes.left
                                        right: parent.right
                                        rightMargin: 10
                                        bottom: parent.bottom
                                        bottomMargin: 6
                                    }

                                    Row {
                                        id: conditionRowFixeSize

                                        anchors {
                                            left: parent.left
                                            bottom: parent.bottom
                                        }
                                        height: agentCombo.height
                                        spacing: 6

                                        // Agent
                                        IngeScapeComboboxItemModel {
                                            id: agentCombo

                                            anchors.verticalCenter : parent.verticalCenter
                                            height : 25
                                            width : 148

                                            model: rootItem.allAgentsGroupsByName

                                            enabled: (rootItem.allAgentsGroupsByName && (rootItem.allAgentsGroupsByName.count > 0))
                                            placeholderText: (rootItem.allAgentsGroupsByName && (rootItem.allAgentsGroupsByName.count === 0) ? "- No Item -"
                                                                                                                                               : "- Select an item -")

                                            Binding {
                                                target: agentCombo
                                                property: "selectedIndex"
                                                value: (myCondition && myCondition.modelM && rootItem.allAgentsGroupsByName) ? rootItem.allAgentsGroupsByName.indexOf(myCondition.modelM.agent)
                                                                                                                              : -1
                                            }

                                            onSelectedItemChanged: {
                                                if (agentCombo.selectedIndex >= 0 && actionM)
                                                {
                                                    myCondition.modelM.agent = agentCombo.selectedItem;

                                                    // Register agent selected to choose it for the next effect or condition
                                                    // Do it only here, because it is updated everytime agent model of condition change
                                                    if (rootItem.panelController) {
                                                        rootItem.panelController.lastAgentSelected = agentCombo.selectedItem;
                                                    }
                                                }
                                            }
                                        }

                                        // Agent Inputs/Outputs
                                        IngeScapeComboboxItemModelAgentsIOP {
                                            id: ioCombo

                                            anchors.verticalCenter: parent.verticalCenter
                                            height : 25
                                            width : 148

                                            visible: myConditionTypeIsValue
                                            enabled: visible

                                            model: (myCondition && myCondition.modelM) ? myCondition.modelM.iopMergedList : 0

                                            Binding {
                                                target: ioCombo
                                                property: "selectedIndex"
                                                value: (myCondition && myCondition.modelM && myCondition.modelM.iopMergedList) ? myCondition.modelM.iopMergedList.indexOf(myCondition.modelM.agentIOP)
                                                                                                                               : -1
                                            }

                                            onSelectedItemChanged: {
                                                if ((ioCombo.selectedIndex >= 0) && myCondition && myCondition.modelM)
                                                {
                                                    myCondition.modelM.agentIOP = ioCombo.selectedItem;

                                                    // Register agent IOP selected to choose it for the next effect or condition
                                                    if (rootItem.panelController) {
                                                        panelController.lastIOPSelected = ioCombo.selectedItem;
                                                    }

                                                    inputComparisonItem.revalidateInput();
                                                }
                                            }
                                        }

                                        // Combo to select the value of the agent condition
                                        IngeScapeComboboxItemModel {
                                            id: comboAgentConditionValues

                                            anchors.verticalCenter: parent.verticalCenter
                                            height: 25
                                            width: 78

                                            visible: (myCondition && (myCondition.conditionType === ActionConditionTypes.AGENT))

                                            model: (controller ? controller.allAgentConditionValues : 0)

                                            Binding {
                                                target: comboAgentConditionValues
                                                property: "selectedIndex"
                                                value: (myCondition && myCondition.modelM && controller) ? controller.allAgentConditionValues.indexOfEnumValue(myCondition.modelM.agentConditionValue)
                                                                                           : -1
                                            }

                                            onSelectedItemChanged: {
                                                if (comboAgentConditionValues.selectedIndex >= 0 && myCondition && myCondition.modelM)
                                                {
                                                    myCondition.modelM.agentConditionValue = comboAgentConditionValues.selectedItem.value;
                                                }
                                            }
                                        }

                                        // Combo to select the type of the value comparison
                                        IngeScapeComboboxItemModel {
                                            id: comboValueComparisonTypes

                                            anchors {
                                                verticalCenter : parent.verticalCenter
                                            }
                                            height: 25
                                            width: 44

                                            visible: myConditionIopIsNotImpulsion

                                            model: (controller ? controller.allValueComparisonTypes : 0)

                                            Binding {
                                                target: comboValueComparisonTypes
                                                property: "selectedIndex"
                                                value: (myCondition && myCondition.modelM && controller) ? controller.allValueComparisonTypes.indexOfEnumValue(myCondition.modelM.valueComparisonType)
                                                                                           : -1
                                            }

                                            onSelectedItemChanged: {
                                                if (comboValueComparisonTypes.selectedIndex >= 0 && myCondition && myCondition.modelM)
                                                {
                                                    myCondition.modelM.valueComparisonType = comboValueComparisonTypes.selectedItem.value;
                                                }
                                            }
                                        }
                                    }

                                    // Comparison Value
                                    InputIOPValueField {
                                        id: inputComparisonItem

                                        anchors {
                                            right: parent.right
                                            leftMargin: 6
                                            left: conditionRowFixeSize.right
                                            bottom: parent.bottom
                                        }

                                        iopVM: rectToName.myCondition.modelM.agentIOP
                                        forceHide: !rectToName.myCondition || (rectToName.myCondition.conditionType !== ActionEffectTypes.VALUE)

                                        function getModelValue() {
                                            return rectToName.myCondition.modelM.comparisonValue
                                        }

                                        function setModelValue(value) {
                                            rectToName.myCondition.modelM.comparisonValue = value
                                        }
                                    }
                                }


                                // Delete Condition
                                LabellessSvgButton {
                                    id: btnDeleteCondition

                                    height : 10
                                    width : 10
                                    anchors {
                                        top: parent.top
                                        right : parent.right
                                        margins: 5
                                    }

                                    pressedID: releasedID + "-pressed"
                                    releasedID: "closeEditor"
                                    disabledID : releasedID


                                    onClicked: {
                                        if (panelController && myCondition)
                                        {
                                            panelController.removeCondition(myCondition);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // add conditions
                LabellessSvgButton {
                    id: addCondition


                    anchors {
                        left: parent.left
                        bottom: parent.bottom
                    }

                    enabled : conditionsItem.isOpened
                    visible: enabled

                    pressedID: releasedID + "-pressed"
                    releasedID: "createButton"
                    disabledID : releasedID


                    onClicked: {
                        if (panelController)
                        {
                            panelController.createNewCondition();
                        }
                    }
                }

            }


            //
            // Advanced modes
            //
            Item {
                id : advancedModesItem
                anchors {
                    left : parent.left
                    right : parent.right
                    top : conditionsItem.bottom
                    topMargin: 15
                }
                clip : true
                height: isOpened ? (titleadvModeMouseArea.height + 1 + revertActionitem.height + revertActionitem.anchors.topMargin + rearmActionitem.height + rearmActionitem.anchors.topMargin + 2)
                                 : titleadvModeMouseArea.height + 1

//                Rectangle {
//                    color: "red"
//                    anchors.fill: parent
//                }

                Behavior on anchors.top {
                    NumberAnimation {}
                }
                Behavior on height {
                    NumberAnimation {}
                }

                property bool isOpened: false

                Connections {
                    target : rootItem
                    Component.onCompleted : {
                        // make the advanced modes visible if there are some modes checked
                        if (actionM && (actionM.shallRevert || actionM.shallRearm)) {
                            advancedModesItem.isOpened = true;
                        }
                    }
                }

                //Title
                MouseArea {
                    id : titleadvModeMouseArea
                    anchors {
                        left : parent.left
                        right : parent.right
                    }
                    height : 29

                    onClicked: {
                        advancedModesItem.isOpened = !advancedModesItem.isOpened;
                    }

                    I2SvgItem {
                        id : arrowadvModes
                        anchors {
                            left : parent.left
                            verticalCenter: titleAdvMode.verticalCenter
                        }

                        svgFileCache : IngeScapeTheme.svgFileIngeScape
                        svgElementId: "arrowWhite"

                        rotation: advancedModesItem.isOpened? 0 : 270
                    }

                    Text {
                        id : titleAdvMode
                        anchors {
                            left : arrowadvModes.right
                            leftMargin: 6
                            right : parent.right
                            verticalCenter: parent.verticalCenter
                        }

                        text : "Advanced options"

                        color: titleadvModeMouseArea.containsPress ? IngeScapeTheme.lightGreyColor : IngeScapeTheme.whiteColor
                        font {
                            family: IngeScapeTheme.textFontFamily
                            pixelSize: 19
                        }
                    }
                }

                // separator
                Rectangle {
                    id : separatorAdvMode
                    anchors {
                        left : parent.left
                        right : parent.right
                        top : titleadvModeMouseArea.bottom
                    }
                    height : 1
                    color : IngeScapeTheme.whiteColor
                }

                // Revert Action
                Item {
                    id : revertActionitem
                    anchors {
                        left: parent.left;
                        right: parent.right;
                        top : separatorAdvMode.bottom
                        topMargin: 8
                    }
                    height : revertActionCB.checked ? revertActionTime.height : revertActionCB.height
                    visible : advancedModesItem.isOpened
                    enabled : visible

                    Behavior on height {
                        NumberAnimation {}
                    }

                    CheckBox {
                        id : revertActionCB
                        anchors {
                            left: parent.left;
                            top : parent.top
                        }

                        checked : actionM && actionM.shallRevert;
                        activeFocusOnPress: true;

                        style: CheckBoxStyle {
                            label: Text {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    verticalCenterOffset: 2
                                }

                                color: control.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor

                                text: "Revert action"
                                elide: Text.ElideRight

                                font {
                                    family: IngeScapeTheme.textFontFamily
                                    pixelSize: 16
                                }

                            }

                            indicator: Rectangle {
                                implicitWidth: 14
                                implicitHeight: 14
                                border.width: 0
                                color: IngeScapeTheme.darkBlueGreyColor

                                I2SvgItem {
                                    visible: control.checked
                                    anchors.centerIn: parent

                                    svgFileCache: IngeScapeTheme.svgFileIngeScape
                                    svgElementId: "check";

                                }
                            }

                        }

                        onCheckedChanged: {
                            if (actionM) {
                                actionM.shallRevert = checked
                            }
                        }


                        Binding {
                            target: revertActionCB
                            property: "checked"
                            value: (actionM && actionM.shallRevert)
                        }
                    }

                    Column {
                        id : revertActionTime

                        anchors {
                            left : revertActionCB.right
                            leftMargin: 14
                            right : parent.right
                            top: revertActionCB.top
                        }

                        height : childrenRect.height
                        spacing: 6

                        enabled: revertActionCB.checked
                        visible : enabled

                        ExclusiveGroup {
                            id : revertActionOpt
                        }

                        CheckBox {
                            id : revertActionTimeCB
                            anchors {
                                left: parent.left;
                            }
                            checked : actionM && actionM.shallRevertWhenValidityIsOver;
                            exclusiveGroup: revertActionOpt
                            activeFocusOnPress: true;

                            style: CheckBoxStyle {
                                label: Text {
                                    anchors {
                                        verticalCenter: parent.verticalCenter
                                        verticalCenterOffset: 2
                                    }
                                    color: control.enabled ? (control.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor)
                                                           : IngeScapeTheme.disabledTextColor

                                    text: "when conditions check terminates"
                                    elide: Text.ElideRight

                                    font {
                                        family: IngeScapeTheme.textFontFamily
                                        pixelSize: 16
                                    }
                                }

                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius : height / 2
                                    border.width: 0
                                    color: control.enabled ? IngeScapeTheme.darkBlueGreyColor : IngeScapeTheme.disabledTextColor

                                    Rectangle {
                                        anchors.centerIn: parent
                                        visible : control.checked
                                        width: 8
                                        height: 8
                                        radius: height / 2
                                        border.width: 0
                                        color: IngeScapeTheme.whiteColor
                                    }
                                }

                            }

                            onCheckedChanged: {
                                if (actionM) {
                                    actionM.shallRevertWhenValidityIsOver = checked
                                }
                            }


                            Binding {
                                target : revertActionTimeCB
                                property : "checked"
                                value : (actionM && actionM.shallRevertWhenValidityIsOver)
                            }
                        }

                        Row {
                            anchors {
                                left: parent.left;
                                right: parent.right
                            }
                            height: 25

                            CheckBox {
                                id : revertActionAfterCB
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                }

                                checked: actionM && actionM.shallRevertAfterTime
                                exclusiveGroup: revertActionOpt
                                activeFocusOnPress: true

                                style: CheckBoxStyle {
                                    label: Text {
                                        anchors {
                                            verticalCenter: parent.verticalCenter
                                            verticalCenterOffset: 2
                                        }
                                        color: control.enabled ? (control.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor)
                                                               : IngeScapeTheme.disabledTextColor

                                        text: "after " // space allowing to keep selection possible for the whole row

                                        elide: Text.ElideRight

                                        font {
                                            family: IngeScapeTheme.textFontFamily
                                            pixelSize: 16
                                        }
                                    }

                                    indicator: Rectangle {
                                        implicitWidth: 14
                                        implicitHeight: 14
                                        radius : height / 2
                                        border.width: 0
                                        color: control.enabled ? IngeScapeTheme.darkBlueGreyColor : IngeScapeTheme.disabledTextColor

                                        Rectangle {
                                            anchors.centerIn: parent
                                            visible : control.checked
                                            width: 8
                                            height: 8
                                            radius: height / 2
                                            border.width: 0
                                            color: IngeScapeTheme.whiteColor
                                        }
                                    }

                                }

                                onCheckedChanged: {
                                    if (actionM) {
                                        actionM.shallRevertAfterTime = checked
                                    }
                                }

                                Binding {
                                    target: revertActionAfterCB
                                    property: "checked"
                                    value: (actionM && actionM.shallRevertAfterTime)
                                }
                            }


                            TextField {
                                id: textFieldDuration

                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    verticalCenterOffset: 2
                                }

                                height: 25
                                width: 57
                                enabled: revertActionAfterCB.enabled
                                horizontalAlignment: TextInput.AlignLeft
                                verticalAlignment: TextInput.AlignVCenter

                                text : actionM ? actionM.revertAfterTimeString : "0.0"
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                validator: RegExpValidator { regExp: /(\d{1,5})([.]\d{3})?$/ }

                                style: I2TextFieldStyle {
                                    backgroundColor: IngeScapeTheme.darkBlueGreyColor
                                    backgroundDisabledColor: IngeScapeTheme.disabledTextColor
                                    borderColor: IngeScapeTheme.whiteColor;
                                    borderErrorColor: IngeScapeTheme.redColor
                                    radiusTextBox: 1
                                    borderWidth: 0;
                                    borderWidthActive: 1
                                    textIdleColor: IngeScapeTheme.whiteColor;
                                    textDisabledColor: IngeScapeTheme.darkGreyColor;

                                    padding.left: 3
                                    padding.right: 3

                                    font {
                                        pixelSize:14
                                        family: IngeScapeTheme.textFontFamily
                                    }

                                }

                                onActiveFocusChanged: {
                                    if (!activeFocus) {
                                        // Move cursor to our first character when we lose focus
                                        // (to always display the beginning or our text instead of
                                        // an arbitrary part if our text is too long)
                                        cursorPosition = 0;
                                    }
                                    else {
                                        textFieldDuration.selectAll();
                                    }
                                }

                                onTextChanged: {
                                    if (activeFocus &&  actionM ) {
                                        actionM.revertAfterTimeString = text;
                                    }
                                }

                                Binding {
                                    target: textFieldDuration
                                    property: "text"
                                    value: (actionM ? actionM.revertAfterTimeString : "")
                                }

                                onFocusChanged: {
                                    if (focus) {
                                        revertActionAfterCB.checked = true;
                                    }
                                }
                            }


                            Text {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    verticalCenterOffset: 2
                                }
                                color: revertActionAfterCB.enabled ? (revertActionAfterCB.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor)
                                                                   : IngeScapeTheme.disabledTextColor

                                text: " seconds"
                                elide: Text.ElideRight

                                font {
                                    family: IngeScapeTheme.textFontFamily
                                    pixelSize: 16
                                }

                                MouseArea {
                                    anchors.fill: parent

                                    onPressed: {
                                        revertActionAfterCB.checked = true;
                                    }
                                }
                            }

                        }

                    }
                }

                // Rearm Action
                Item {
                    id : rearmActionitem
                    anchors {
                        left: parent.left;
                        right: parent.right;
                        top : revertActionitem.bottom
                        topMargin: 17
                    }
                    height : 50
                    visible : advancedModesItem.isOpened
                    enabled : visible

                    Column {
                        anchors {
                            left: parent.left;
                            right: parent.right;
                            top : parent.top
                        }


                        CheckBox {
                            id : rearmActionCB
                            anchors {
                                left: parent.left;
                            }

                            checked : actionM && actionM.shallRearm;
                            activeFocusOnPress: true;
                            height: 25

                            style: CheckBoxStyle {
                                label: Text {
                                    anchors {
                                        verticalCenter: parent.verticalCenter
                                        verticalCenterOffset: 2
                                    }
                                    color: control.checked? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor

                                    text: "Allow multiple triggers as long as conditions are verified"
                                    elide: Text.ElideRight

                                    font {
                                        family: IngeScapeTheme.textFontFamily
                                        pixelSize: 16
                                    }
                                }

                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    border.width: 0
                                    color: IngeScapeTheme.darkBlueGreyColor

                                    I2SvgItem {
                                        visible: control.checked
                                        anchors.centerIn: parent

                                        svgFileCache: IngeScapeTheme.svgFileIngeScape
                                        svgElementId: "check";

                                    }
                                }

                            }

                            onCheckedChanged: {
                                if (actionM) {
                                    actionM.shallRearm = checked
                                }
                            }


                            Binding {
                                target: rearmActionCB
                                property: "checked"
                                value: (actionM && actionM.shallRearm)
                            }
                        }

                        Row {

                            anchors {
                                left: parent.left;
                                leftMargin: 18
                            }

                            height: 25

                            Text {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    verticalCenterOffset: 2

                                }

                                color: rearmActionCB.enabled ? (rearmActionCB.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor)
                                                             : IngeScapeTheme.disabledTextColor

                                text: "with trigger every "
                                elide: Text.ElideRight

                                font {
                                    family: IngeScapeTheme.textFontFamily
                                    pixelSize: 16
                                }

                                MouseArea {
                                    anchors.fill: parent

                                    onPressed: {
                                        rearmActionCB.checked = true;
                                    }
                                }
                            }

                            TextField {
                                id: textFieldTimeBeforeRearm

                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    verticalCenterOffset: 2
                                }

                                height: 25
                                width: 57
                                enabled: rearmActionCB.enabled
                                horizontalAlignment: TextInput.AlignLeft
                                verticalAlignment: TextInput.AlignVCenter

                                text: actionM ? actionM.rearmAfterTimeString : "0.0"
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                                validator: RegExpValidator { regExp: /(\d{1,5})([.]\d{3})?$/ }

                                style: I2TextFieldStyle {
                                    backgroundColor: IngeScapeTheme.darkBlueGreyColor
                                    backgroundDisabledColor: IngeScapeTheme.disabledTextColor
                                    borderColor: IngeScapeTheme.whiteColor;
                                    borderErrorColor: IngeScapeTheme.redColor
                                    radiusTextBox: 1
                                    borderWidth: 0;
                                    borderWidthActive: 1
                                    textIdleColor: IngeScapeTheme.whiteColor;
                                    textDisabledColor: IngeScapeTheme.darkGreyColor;

                                    padding.left: 3
                                    padding.right: 3

                                    font {
                                        pixelSize:14
                                        family: IngeScapeTheme.textFontFamily
                                    }

                                }

                                onActiveFocusChanged: {
                                    if (!activeFocus) {
                                        // Move cursor to our first character when we lose focus
                                        // (to always display the beginning or our text instead of
                                        // an arbitrary part if our text is too long)
                                        cursorPosition = 0;
                                    }
                                    else {
                                        textFieldTimeBeforeRearm.selectAll();
                                    }
                                }

                                onTextChanged: {
                                    if (activeFocus &&  actionM ) {
                                        actionM.rearmAfterTimeString = text;
                                    }
                                }

                                Binding {
                                    target: textFieldTimeBeforeRearm
                                    property: "text"
                                    value: (actionM ? actionM.rearmAfterTimeString : "")
                                }

                                onFocusChanged: {
                                    if (focus) {
                                        rearmActionCB.checked = true;
                                    }
                                }
                            }


                            Text {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    verticalCenterOffset: 2
                                }

                                color: rearmActionCB.enabled ? (rearmActionCB.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor)
                                                             : IngeScapeTheme.disabledTextColor

                                text: " seconds"
                                elide: Text.ElideRight

                                font {
                                    family: IngeScapeTheme.textFontFamily
                                    pixelSize: 16
                                }

                                MouseArea {
                                    anchors.fill: parent

                                    onPressed: {
                                        rearmActionCB.checked = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }


            // Delete Button
            MouseArea {
                id: actionDeleteBtn

                anchors {
                    left : parent.left
                    leftMargin: 15
                    verticalCenter: cancelButton.verticalCenter
                    verticalCenterOffset: 2
                }
                height : actionDelete.height
                width : actionDelete.width

                enabled: visible
                visible: (panelController && panelController.originalAction !== null)
                hoverEnabled: true

                onClicked: {
                    if (controller && panelController && panelController.originalAction)
                    {
                        if (controller.isActionInsertedInTimeLine(panelController.originalAction)
                                || IngeScapeEditorC.agentsMappingC.isActionInsertedInMapping(panelController.originalAction))
                        {
                            deleteConfirmationPopup.open();
                        }
                        else {
                            // Delete our action
                            controller.deleteAction(panelController.originalAction);
                        }
                    }
                }

                Text {
                    id: actionDelete

                    anchors {
                        left : parent.left
                    }
                    text : "Delete Action"
                    color: actionDeleteBtn.pressed ? IngeScapeTheme.greyColor : IngeScapeTheme.lightGreyColor
                    elide: Text.ElideRight

                    font {
                        family: IngeScapeTheme.textFontFamily
                        pixelSize: 16
                    }
                }

                // underline
                Rectangle {
                    visible: actionDeleteBtn.containsMouse

                    anchors {
                        left : actionDeleteBtn.left
                        right : actionDelete.right
                        bottom : parent.bottom
                    }

                    height : 1

                    color : actionDelete.color
                }
            }


            // Button "Cancel"
            Button {
                id: cancelButton
                activeFocusOnPress: true

                property var boundingBox: IngeScapeTheme.svgFileIngeScape.boundsOnElement("button");

                height: boundingBox.height
                width: boundingBox.width

                enabled : visible
                text: "Cancel"

                anchors {
                    verticalCenter: okButton.verticalCenter
                    right: okButton.left
                    rightMargin: 20
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
                    labelColorDisabled: IngeScapeTheme.whiteColor

                }

                onClicked: {
                    // Close our popup
                    rootItem.close();
                }
            }

            // Button "OK"
            Button {
                id: okButton

                property var boundingBox: IngeScapeTheme.svgFileIngeScape.boundsOnElement("button");

                height: boundingBox.height
                width: boundingBox.width

                enabled: visible && textFieldName.text !== ""
                activeFocusOnPress: true
                text: "OK"

                anchors {
                    bottom: parent.bottom
                    right: parent.right
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
                    labelColorDisabled: IngeScapeTheme.whiteColor

                }

                onClicked: {
                    if (controller) {
                        controller.validateActionEditor(model.QtObject);
                    }

                    // Close our popup
                    rootItem.close();
                }
            }



            //
            // Keys handling
            //
            Keys.onReturnPressed : {
                if (okButton.enabled) {
                    okButton.clicked();
                }
            }

            Keys.onEscapePressed : {
                if (cancelButton.enabled) {
                    cancelButton.clicked();
                }
            }
        }
    }


    //
    // Delete Confirmation
    //
    ConfirmationPopup {
        id: deleteConfirmationPopup

        confirmationText: "This action is used in the platform.\nDo you want to completely delete it?"

        onConfirmed: {
            if (panelController.originalAction) {
                // Delete our action
                controller.deleteAction(panelController.originalAction);
            }
        }
    }


    I2Layer {
        id: overlayLayerComboBox
        objectName: "overlayLayerComboBox"

        anchors.fill: parent
    }

    I2Layer {
        id: overlayLayer
        objectName: "overlayLayer"

        anchors.fill: parent
    }
}

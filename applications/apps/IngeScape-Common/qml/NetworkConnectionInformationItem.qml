import QtQuick 2.8
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 2.0 as Controls2

import INGESCAPE 1.0

import I2Quick 1.0


I2CustomRectangle {
    id: root

    //--------------------------------------------------------
    //
    // Properties
    //
    //--------------------------------------------------------

    // Flag indicating if our edition mode is opened
    readonly property alias isEditionModeOpened: rootPrivate.isEditionModeOpened

    // Flag indicating if we are online
    property bool isOnline: false

    // Current network device
    property string currentNetworkDevice: ""

    // Current port
    property int currentPort: 31520

    // Duration of animations in milliseconds (250 ms => default duration of QML animations)
    property int animationDuration: 250

    // List of network devices
    property alias listOfNetworkDevices: selectNetworkDeviceCombobox.model

    // Selected index of our list of network devices
    property alias listOfNetworkDevicesSelectedIndex: selectNetworkDeviceCombobox.selectedIndex;

    // Error message
    property alias errorMessage: textErrorMessage.text

    // Auto-close timeout in milliseconds
    property alias autoCloseTimeoutInMilliseconds: autoCloseTimer.interval


    //
    // Configure our item
    //
    width: content.x * 2 + content.width
    height: content.y * 2 + content.height

    bottomLeftRadius: 5

    color: (root.isOnline) ? IngeScapeTheme.veryDarkGreyColor : "#8C1B1F"

    borderColor: (root.isOnline) ? IngeScapeTheme.editorsBackgroundBorderColor : "#FF1D25"
    borderWidth: 1

    fuzzyRadius: 8



    //
    // Private properties
    //
    QtObject {
        id: rootPrivate

        // Flag indicating if we can perform animations
        property bool canPerformAnimations: false

        // Flag indicating if our edition mode is opened
        property bool isEditionModeOpened: false

        // Flag indicating if we can auto-close our edition mode
        property bool canAutoCloseEditionMode: false
    }



    //--------------------------------------------------------
    //
    // Signals
    //
    //--------------------------------------------------------

    // Triggered when our edition mode will be opened
    signal willOpenEditionMode();

    // Triggered when our edition mode will be closed
    signal willCloseEditionMode();


    // Triggered when we press the "OK" button
    signal changeNetworkSettings(string networkDevice, int port, bool clearPlatform);




    //--------------------------------------------------------
    //
    // Methods
    //
    //--------------------------------------------------------

    // Open our widget
    function open()
    {
        if (!rootPrivate.isEditionModeOpened)
        {
            // Notify change
            root.willOpenEditionMode();

            // Update internal states
            rootPrivate.isEditionModeOpened = true;
            rootPrivate.canAutoCloseEditionMode = true;

            // Get focus
            root.forceActiveFocus();

            // Reset UI
            // - network device
            resetComboboxSelectNetworkDevice();
            // - port
            selectPortTextfield.text = Qt.binding(function() {
               return root.currentPort;
            });
            // - clearPlatform flag
            clearPlatform.checked = true;
        }
    }


    // Close our widget
    function close()
    {
        if (rootPrivate.isEditionModeOpened)
        {
            // Stop our timer
            autoCloseTimer.stop();

            // Notify change
            root.willCloseEditionMode();

            // Update internal states
            rootPrivate.isEditionModeOpened = false;
            rootPrivate.canAutoCloseEditionMode = false;
        }
    }



    // Reset our combobox used to select a network device
    function resetComboboxSelectNetworkDevice()
    {
        // Close our combobox
        selectNetworkDeviceCombobox.close();

        // Update selected index
        selectNetworkDeviceCombobox.selectedIndex = Qt.binding(function() {
            var index = (
                         (root.listOfNetworkDevices)
                         ? root.listOfNetworkDevices.indexOf(root.currentNetworkDevice)
                         : -1
                        );

            // When our current network device is offline, we try to help our end-users
            // by selecting a network device when we can i.e. when there is a single network device available
            return (
                    (index >= 0)
                    ? index
                    : ((root.listOfNetworkDevices.length === 1) ? 0 : -1)
                   );
        });
    }


    // Check if values are valid or not
    function checkValues(index, networkDevice, port, forceQmlUpdate)
    {
        return (
                // Coherent values
                (index >= 0)
                && (networkDevice !== "")
                && (port !== "")
                // Values are still available
                && listOfNetworkDevices
                && (listOfNetworkDevices.indexOf(networkDevice) >= 0)
                );
    }


    // Reset auto-close timer
    function resetAutoCloseTimer()
    {
        rootPrivate.canAutoCloseEditionMode = false;
        rootPrivate.canAutoCloseEditionMode = true;
    }


    //--------------------------------------------------------
    //
    // Behavior
    //
    //--------------------------------------------------------

    Behavior on color {
        enabled: rootPrivate.canPerformAnimations

        ColorAnimation {
            duration: root.animationDuration
        }
    }


    Behavior on borderColor {
        enabled: rootPrivate.canPerformAnimations

        ColorAnimation {
            duration: root.animationDuration
        }
    }


    Component.onCompleted: {
        rootPrivate.canPerformAnimations = true;
    }


    onListOfNetworkDevicesChanged: {
        resetComboboxSelectNetworkDevice();
    }



    // Timer used to auto-close our edition mode
    Timer {
        id: autoCloseTimer

        running: root.isOnline && rootPrivate.canAutoCloseEditionMode
                 && !(selectNetworkDeviceCombobox.comboList.visible || selectPortTextfield.activeFocus)

        repeat: false
        triggeredOnStart: false

        interval: 20000

        onTriggered: {
            root.close();
        }
    }



    //--------------------------------------------------------
    //
    // Content
    //
    //--------------------------------------------------------


    Column {
        id: content

        anchors {
            top: parent.top
            topMargin: 15
            left: parent.left
            leftMargin: 15
        }

        width: (rootPrivate.isEditionModeOpened)
               ? 300
               : Math.min(300, Math.max(textInfoPart1.implicitWidth, textInfoPart2.implicitWidth) + columnInfo.anchors.rightMargin + buttons.width)


        Behavior on width {
            enabled: rootPrivate.canPerformAnimations

            NumberAnimation {
                duration: root.animationDuration
            }
        }


        // Header
        Item {
            id: header

            anchors {
                left: parent.left
                right: parent.right
            }

            height: childrenRect.y + childrenRect.height


            // Buttons
            Item {
                id: buttons

                anchors {
                    right: parent.right
                }

                width: childrenRect.width
                height: childrenRect.height


                // Button - Edit network settings
                LabellessSvgButton {
                    id: buttonOpenSettings

                    releasedID: "settings"
                    pressedID: "settings-pressed"
                    disabledID: "settings-disabled"

                    visible: !rootPrivate.isEditionModeOpened
                    enabled: visible

                    onClicked: {
                        root.open();
                    }

                    Controls2.ToolTip {
                        delay: Qt.styleHints.mousePressAndHoldInterval
                        visible: buttonOpenSettings.enabled && buttonOpenSettings.__behavior.containsMouse
                        text: qsTr("Edit network settings")
                    }
                }


                // Button - Quit edit network settings
                LabellessSvgButton {
                    id: buttonCloseSettings

                    releasedID: "closeEditor"
                    pressedID: "closeEditor-pressed"

                    visible: rootPrivate.isEditionModeOpened
                    enabled: visible

                    onClicked: {
                        root.close();
                    }

                    Controls2.ToolTip {
                        delay: Qt.styleHints.mousePressAndHoldInterval
                        visible: buttonCloseSettings.enabled && buttonCloseSettings.__behavior.containsMouse
                        text: qsTr("Close network settings")
                    }
                }
            }


            // Infos
            Column {
                id: columnInfo

                anchors {
                    left: parent.left
                    right: buttons.left
                    rightMargin: 20
                }

                spacing: 0

                Text {
                    id: textInfoPart1

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    color: IngeScapeTheme.whiteColor

                    font {
                        family: IngeScapeTheme.textFontFamily
                        weight: Font.Medium
                        pixelSize: 18
                    }

                    elide: Text.ElideRight

                    text: (root.isOnline) ? qsTr("Connected on %1").arg(root.currentNetworkDevice)
                                          : qsTr("Offline")
                }

                Item {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    height: (root.isOnline) ? textInfoPart2.height : 0

                    visible: (height !== 0)

                    clip: (height !== textInfoPart2.height)

                    opacity: (root.isOnline) ? 1 : 0

                    Behavior on height {
                        enabled: rootPrivate.canPerformAnimations

                        NumberAnimation {
                            duration: root.animationDuration
                        }
                    }

                    Behavior on opacity {
                        enabled: rootPrivate.canPerformAnimations

                        NumberAnimation {
                            duration: root.animationDuration
                        }
                    }

                    Text {
                        id: textInfoPart2

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        color: IngeScapeTheme.whiteColor

                        font {
                            family: IngeScapeTheme.textFontFamily
                            weight: Font.Medium
                            pixelSize: 18
                        }

                        elide: Text.ElideRight

                        text: qsTr("with port %1").arg(root.currentPort)
                    }
                }
            }
        }


        // Edit network settings
        Item {
            id: editNetworkSettings

            anchors {
                left: parent.left
                right: parent.right
            }

            height: (rootPrivate.isEditionModeOpened) ? editNetworkSettingsColumn.height : 0

            opacity: (rootPrivate.isEditionModeOpened) ? 1 : 0

            visible: (height != 0)
            clip: (height !== editNetworkSettingsColumn.height)

            Behavior on height {
                enabled: rootPrivate.canPerformAnimations

                NumberAnimation {
                    duration: root.animationDuration
                }
            }

            Behavior on opacity {
                enabled: rootPrivate.canPerformAnimations

                NumberAnimation {
                    duration: root.animationDuration
                }
            }

            Column {
                id: editNetworkSettingsColumn

                anchors {
                    left: parent.left
                    right: parent.right
                }

                spacing: 15

                // Title
                Column {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    spacing: 10

                    Item {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        height: 10

                        Rectangle {
                            anchors {
                                bottom: parent.bottom
                                left: parent.left
                                right: parent.right
                            }

                            height: 1

                            color: root.borderColor
                        }
                    }

                    Text {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        text: qsTr("Edit network settings")

                        color: IngeScapeTheme.whiteColor

                        font {
                            family: IngeScapeTheme.textFontFamily
                            weight: Font.Medium
                            pixelSize: 18
                        }
                    }
                }


                // Select network device
                Column {
                    id: selectNetworkDevice

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    spacing: 6

                    Text {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        text: qsTr("Network device")

                        color: IngeScapeTheme.whiteColor

                        font {
                            family: IngeScapeTheme.textFontFamily
                            weight: Font.Medium
                            pixelSize: 16
                        }
                    }


                    I2ComboboxStringList {
                        id: selectNetworkDeviceCombobox

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        height : 25

                        style: IngeScapeComboboxStyle {}
                        scrollViewStyle: IngeScapeScrollViewStyle {}

                        _mouseArea.hoverEnabled: true

                        Controls2.ToolTip {
                            delay: Qt.styleHints.mousePressAndHoldInterval
                            visible: selectNetworkDeviceCombobox._mouseArea.containsMouse
                            text: selectNetworkDeviceCombobox.text
                        }

                        placeholderText: qsTr("Select a network device...")

                        model: null
                        selectedIndex: -1

                        delegate: customDelegate.component

                        IngeScapeToolTipComboboxDelegate {
                            id: customDelegate

                            comboboxStyle: selectNetworkDeviceCombobox.style
                            selection: selectNetworkDeviceCombobox.selectedIndex

                            height: selectNetworkDeviceCombobox.comboButton.height
                            width:  selectNetworkDeviceCombobox.comboButton.width

                            // Called from the component's MouseArea
                            // 'index' is the index of the clicked component inside the model.
                            function onDelegateClicked(index) {
                                selectNetworkDeviceCombobox.onDelegateClicked(index)
                            }

                            // Called from the component to get the text of the current item to display
                            // 'index' is the index of the component to be displayed inside the model.
                            function getItemText(index) {
                                return selectNetworkDeviceCombobox.modelToString(selectNetworkDeviceCombobox.model[index]);
                            }
                        }


                        onSelectedIndexChanged: {
                            root.resetAutoCloseTimer();
                        }
                    }
                }


                // Select port
                Column {
                    id: selectPort

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    spacing: 6

                    Text {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        text: qsTr("Port")

                        color: IngeScapeTheme.whiteColor

                        font {
                            family: IngeScapeTheme.textFontFamily
                            weight: Font.Medium
                            pixelSize: 16
                        }
                    }


                    TextField {
                        id: selectPortTextfield

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        height: 25

                        verticalAlignment: TextInput.AlignVCenter

                        text: root.currentPort

                        validator: IntValidator {
                            bottom: 1
                            top: 65535
                        }

                        style: I2TextFieldStyle {
                            backgroundColor: IngeScapeTheme.darkBlueGreyColor
                            borderColor: IngeScapeTheme.whiteColor
                            borderErrorColor: IngeScapeTheme.redColor
                            radiusTextBox: 1
                            borderWidth: 0
                            borderWidthActive: 1
                            textIdleColor: IngeScapeTheme.whiteColor
                            textDisabledColor: IngeScapeTheme.darkGreyColor

                            padding {
                                left: 3
                                right: 3
                            }

                            font {
                                pixelSize:15
                                family: IngeScapeTheme.textFontFamily
                            }

                        }

                        Binding {
                            target: selectPortTextfield
                            property: "text"
                            value: root.currentPort
                        }


                        onTextChanged: {
                            root.resetAutoCloseTimer();
                        }
                    }

                }


                // Clear platform ?
                CheckBox {
                    id: clearPlatform

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    checked: true

                    activeFocusOnPress: true

                    style: CheckBoxStyle {
                        label: Text {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                verticalCenterOffset: 2
                            }

                            text: qsTr("Clear platform when modified")

                            font {
                                family: IngeScapeTheme.textFontFamily
                                pixelSize: 16
                            }

                            color: control.checked ? IngeScapeTheme.whiteColor : IngeScapeTheme.lightGreyColor
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
                                svgElementId: "check"
                            }
                        }
                    }


                    onCheckedChanged: {
                        root.resetAutoCloseTimer();
                    }
                }


                // Error message
                Text {
                    id: textErrorMessage

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    wrapMode: Text.WordWrap

                    color: (root.isOnline) ? IngeScapeTheme.orangeColor : IngeScapeTheme.whiteColor

                    font {
                        family: IngeScapeTheme.textFontFamily
                        bold: true
                        pixelSize: 16
                    }

                    text: ""

                    visible: (text.length !== 0)
                }


                // Actions
                Item {
                    id: actions

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    height: 40

                    property var buttonBoundingBox: IngeScapeTheme.svgFileIngeScape.boundsOnElement("button");


                    Row {
                        anchors {
                            right: parent.right
                            bottom: parent.bottom
                        }

                        height: actions.buttonBoundingBox.height

                        spacing: 15

                        Button {
                            id: buttonCancel

                            anchors {
                                top: parent.top
                                bottom: parent.bottom
                            }

                            width: actions.buttonBoundingBox.width

                            activeFocusOnPress: true

                            text: qsTr("Cancel")

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
                                root.close();
                            }
                        }

                        Button {
                            id: buttonOk

                            anchors {
                                top: parent.top
                                bottom: parent.bottom
                            }

                            width: actions.buttonBoundingBox.width

                            enabled: root.checkValues(selectNetworkDeviceCombobox.selectedIndex,
                                                      selectNetworkDeviceCombobox.selectedItem,
                                                      selectPortTextfield.text,
                                                      (root.listOfNetworkDevices ? root.listOfNetworkDevices.length : 0)
                                                      )

                            activeFocusOnPress: true

                            text: qsTr("OK")

                            style: I2SvgButtonStyle {
                                fileCache: IngeScapeTheme.svgFileIngeScape

                                pressedID: releasedID + "-pressed"
                                releasedID: "button"
                                disabledID: releasedID + "-disabled"

                                font {
                                    family: IngeScapeTheme.textFontFamily
                                    weight: Font.Medium
                                    pixelSize: 16
                                }
                                labelColorPressed: IngeScapeTheme.blackColor
                                labelColorReleased: IngeScapeTheme.whiteColor
                                labelColorDisabled: IngeScapeTheme.greyColor
                            }

                            onClicked: {
                                root.changeNetworkSettings(selectNetworkDeviceCombobox.selectedItem, selectPortTextfield.text, clearPlatform.checked);
                            }
                        }
                    }
                }
            }
        }
    }




    //--------------------------------------------------------
    //
    // Layers
    //
    //--------------------------------------------------------

    // layer used to display comboboxes
    I2Layer {
        id: overlayLayerComboBox

        objectName: "overlayLayerComboBox"

        anchors.fill: parent
    }
}
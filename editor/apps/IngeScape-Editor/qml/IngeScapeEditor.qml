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
 *      Alexandre Lemort   <lemort@ingenuity.io>
 *      Justine Limoges    <limoges@ingenuity.io>
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

// host sub-directory
import "host" as Host

// record sub-directory
import "record" as Record

// scenario sub-directory
import "scenario" as Scenario


Item {
    id: rootItem

    anchors.fill: parent


    //--------------------------------------------------------
    //
    //
    // Properties
    //
    //
    //--------------------------------------------------------



    //--------------------------------------------------------
    //
    //
    // Functions
    //
    //
    //--------------------------------------------------------

    //
    // function allowing to open the history panel
    //
    function openHistory() {
        //console.log("QML: function openHistory()");

        historyPanel.show();

        // Raises the window in the windowing system
        historyPanel.raise();
    }


    //
    // Function allowing to open the network configuration popup
    //
    function openConfiguration() {
        if (IngeScapeEditorC.networkC) {
            IngeScapeEditorC.networkC.updateAvailableNetworkDevices();
        }

        networkConfigurationPopup.open();
    }


    //
    // Function allowing to open the license (configuration) popup
    //
    function openLicensePopup() {
        licensePopup.open();
    }


    // Function allowing to open the Getting Started popup
    function openGettingStarted() {
        gettingStartedWindow.resetInternetUrl();
        gettingStartedWindow.visible = true;
    }


    //--------------------------------------------------------
    //
    //
    // Behaviors
    //
    //
    //--------------------------------------------------------

    // When the QML is loaded...
    Component.onCompleted: {
        // FIXME Several popup may appear at startup depending on the current platform configuration. Need to prioritize them and maybe show them sequentialy, not on top of each other.

        // ...we check the value of the error message when a connection attempt fails
        if (IngeScapeEditorC.errorMessageWhenConnectionFailed !== "")
        {
            //console.error("On Completed: Error Message = " + IngeScapeEditorC.errorMessageWhenConnectionFailed);
            networkConfigurationPopup.open();
        }

        // ...we check the value of the flag "is Valid License"
        if (IngeScapeEditorC.licensesC && IngeScapeEditorC.licensesC.mergedLicense && !IngeScapeEditorC.licensesC.mergedLicense.editorLicenseValidity)
        {
            openLicensePopup();
        }

        // ...we check if we must open the getting started window
        if (IngeScapeEditorC.gettingStartedShowAtStartup)
        {
            openGettingStarted();
        }
    }


    Connections {
        target: IngeScapeEditorC.licensesC.mergedLicense

        //ignoreUnknownSignals: true

        onIngescapeLicenseValidityChanged: {
            console.log("QML (IngeScape Editor): on is License Valid Changed");
            if (IngeScapeEditorC.licensesC && IngeScapeEditorC.licensesC.mergedLicense && !IngeScapeEditorC.licensesC.mergedLicense.ingescapeLicenseValidity) {
                openLicensePopup();
            }
        }

        onEditorLicenseValidityChanged: {
            console.log("QML (IngeScape Editor): on is Editor License Valid Changed");
            if (IngeScapeEditorC.licensesC && IngeScapeEditorC.licensesC.mergedLicense && !IngeScapeEditorC.licensesC.mergedLicense.editorLicenseValidity) {
                openLicensePopup();
            }
        }
    }


    //--------------------------------------------------------
    //
    //
    // Content
    //
    //
    //--------------------------------------------------------


    //
    // Center panel
    //
    Item {
        id: centerPanel

        anchors {
            left: leftPanel.right
            right: parent.right
            top: parent.top
            bottom: bottomPanel.top
        }


        AgentsMappingView {
            id: agentsMappingView

            anchors.fill: parent

            controller: IngeScapeEditorC.agentsMappingC
        }


        NetworkConnectionInformationItem {
            id: networkConfigurationInfo

            anchors {
                top: parent.top
                topMargin: -1
                right: parent.right
                rightMargin: -1
            }

            isOnline: IngeScapeEditorC.networkC && IngeScapeEditorC.networkC.isOnline

            currentNetworkDevice: IngeScapeEditorC.networkDevice
            currentPort: IngeScapeEditorC.port

            listOfNetworkDevices: IngeScapeEditorC.networkC ? IngeScapeEditorC.networkC.availableNetworkDevices : null


            onWillOpenEditionMode: {
                // Update our list of available network devices
                if (IngeScapeEditorC.networkC)
                {
                    IngeScapeEditorC.networkC.updateAvailableNetworkDevices();
                }
            }

            onChangeNetworkSettings: {
                if (IngeScapeEditorC.networkC && IngeScapeEditorC.networkC.isAvailableNetworkDevice(networkDevice))
                {
                    // Re-Start the Network
                    var success = IngeScapeEditorC.restartNetwork(port, networkDevice, clearPlatform);
                    if (success)
                    {
                        close();
                    }
                    else
                    {
                        console.error("Network cannot be (re)started on device " + networkDevice + " and port " + port);
                    }
                }
            }
        }
    }


    //
    // Scenario TimeLine
    //
    ScenarioTimeLine {
        id: bottomPanel

        anchors {
            left: leftPanel.right
            right: parent.right
            bottom: parent.bottom
        }
        height: 0

        scenarioController: IngeScapeEditorC.scenarioC
        timeLineController: IngeScapeEditorC.timeLineC

        onUnlicensedAction: {
            licensePopup.open();
        }
    }


    //
    // Left panel
    //
    Rectangle {
        id: leftPanel

        width: IngeScapeEditorTheme.leftPanelWidth

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }

        color: IngeScapeEditorTheme.agentsMappingBackgroundColor

        I2CustomRectangle {
            anchors {
                fill: parent
                topMargin: 9
            }
            color: IngeScapeTheme.blackColor

            fuzzyRadius: 8
            topRightRadius : 5

            borderWidth: 1
            borderColor: IngeScapeEditorTheme.selectedTabsBackgroundColor

            // tabs of left panel
            I2TabView {
                id : leftPanelTabs

                anchors.fill: parent

                style: I2TabViewStyle {
                    frameOverlap: 1

                    tab: I2CustomRectangle {
                        color: styleData.selected ? IngeScapeEditorTheme.selectedTabsBackgroundColor : "transparent"

                        implicitWidth: (IngeScapeEditorC.recordsSupervisionC && (IngeScapeEditorC.recordsSupervisionC.isRecorderON === true)) ? leftPanelTabs.width / 4
                                                                                                                                              : leftPanelTabs.width / 3
                        implicitHeight: 26
                        topRightRadius : 5

                        visible: (styleData.index === 3) ? (IngeScapeEditorC.recordsSupervisionC && (IngeScapeEditorC.recordsSupervisionC.isRecorderON === true))
                                                         : true

                        Text {
                            id: text

                            anchors.fill: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter;

                            text: styleData.title

                            color: styleData.selected ? IngeScapeTheme.whiteColor : IngeScapeTheme.greyColor
                            wrapMode: Text.Wrap;

                            font {
                                family: IngeScapeTheme.labelFontFamily;
                                weight: Font.ExtraBold;
                                pixelSize:18;
                                capitalization: Font.AllUppercase;
                            }
                        }
                    }

                    frame: Rectangle {
                        color: IngeScapeEditorTheme.selectedTabsBackgroundColor
                    }
                }

                currentIndex: 0

                /*onCurrentIndexChanged: {
                    console.log("on Current Index changed: " + currentIndex);
                }*/

                Connections {
                    target: IngeScapeEditorC.recordsSupervisionC

                    onIsRecorderONChanged: {
                        //console.log("on Is Recorder ON changed: " + IngeScapeEditorC.recordsSupervisionC.isRecorderON);

                        if (IngeScapeEditorC.recordsSupervisionC && (IngeScapeEditorC.recordsSupervisionC.isRecorderON === false)) {
                            leftPanelTabs.currentIndex = 0;
                        }
                    }
                }


                Tab {
                    id: tabAgents

                    title: qsTr("Agents");
                    active : true

                    Agent.AgentsList {
                        id: agentsList

                        anchors.fill: parent

                        agentsSupervisionController: IngeScapeEditorC.agentsSupervisionC
                        licensesController: IngeScapeEditorC.licensesC

                        onUnlicensedAction: {
                            licensePopup.open();
                        }
                    }
                }

                Tab {
                    id: tabHosts

                    title: qsTr("Hosts");

                    Host.HostsList {
                        id: hostsList

                        anchors.fill: parent

                        controller: IngeScapeEditorC.hostsSupervisionC
                    }
                }

                Tab {
                    id: idActions

                    title: qsTr("ACTIONS");
                    active : false

                    Item {
                        anchors.fill: parent

                        Scenario.ActionsList {
                            id: actionsList

                            anchors {
                                fill : parent
                                bottomMargin: actionsPanel.height
                            }

                            scenarioController: IngeScapeEditorC.scenarioC

                            onUnlicensedAction: {
                                licensePopup.open();
                            }
                        }

                        Scenario.ActionsPanel {
                            id : actionsPanel
                            anchors {
                                left : parent.left
                                right: parent.right
                                bottom : parent.bottom
                            }
                            height : 320
                            controller: IngeScapeEditorC.scenarioC
                        }
                    }
                }

                Tab {
                    id: tabRecords

                    title: qsTr("Records");

                    Record.RecordsList {
                        controller: IngeScapeEditorC.recordsSupervisionC
                        id: recordsList
                        anchors.fill: parent
                    }
                }
            }
        }
    }


    //
    // Model Visualizer
    //
    Loader {
        id: loaderOfModelVisualizer

        anchors {
            top: parent.top
            left: leftPanel.right
            right: parent.right
        }
        height: 250

        source: IngeScapeEditorC.isAvailableModelVisualizer ? "ModelVisualizer.qml" : ""
    }


    // List of "Agent Definition Editor(s)"
    Repeater {
        model: (IngeScapeEditorC.modelManager ? IngeScapeEditorC.modelManager.openedDefinitions : 0)

        delegate: Item {
            Agent.AgentDefinitionEditor {
                id: agentDefinitionEditor

                visible: true

                Component.onCompleted: {
                    x = rootItem.Window.window.x + rootItem.Window.width/2 - agentDefinitionEditor.width/2 + (index * 40);
                    y = rootItem.Window.window.y + rootItem.Window.height/2  - agentDefinitionEditor.height / 2.0 + (index * 40);

                    // Add this window to the list of opened windows
                    IngeScapeEditorC.addOpenedWindow(agentDefinitionEditor);
                }

                onClosing: {
                    IngeScapeEditorC.closeDefinition(model.QtObject);
                }

                onOpenHistory : {
                    //console.log("QML: slot onOpenHistory");

                    historyPanel.show();

                    // Raises the window in the windowing system
                    historyPanel.raise();
                }
            }
        }
    }


    // List of "Actions Editor(s)"
    Repeater {
        model: IngeScapeEditorC.scenarioC ? IngeScapeEditorC.scenarioC.openedActionsEditorsControllers : 0;

        delegate: Item {
            Scenario.ActionEditor {
                id: actionEditor

                controller : IngeScapeEditorC.scenarioC
                panelController: model.QtObject

                visible: true

                Component.onCompleted: {
                    x = rootItem.Window.window.x + rootItem.Window.width/2 - actionEditor.width/2  + (index * 40);
                    y = rootItem.Window.window.y + rootItem.Window.height/2 - actionEditor.height/2 + (index * 40);

                    // Add this window to the list of opened windows
                    IngeScapeEditorC.addOpenedWindow(actionEditor);
                }

                onClosingVersion: {
                    IngeScapeEditorC.closeActionEditor(model.QtObject);
                }
            }
        }
    }


    //
    // List of "Log Stream Viewer"
    //
    Repeater {
        model : IngeScapeEditorC.openedLogStreamControllers

        delegate: Item {
            Agent.LogStreamPanel {
                id: logStreamPanel

                controller: model.QtObject

                visible: true

                Component.onCompleted: {
                    x = rootItem.Window.window.x + rootItem.Window.width/2 - logStreamPanel.width/2  + (index * 40);
                    y = rootItem.Window.window.y + rootItem.Window.height/2 - logStreamPanel.height/2 + (index * 40);

                    // Add this window to the list of opened windows
                    IngeScapeEditorC.addOpenedWindow(logStreamPanel);
                }

                onClosingVersion: {
                    IngeScapeEditorC.closeLogStreamController(model.QtObject);
                }
            }
        }
    }


    //
    // History Panel
    //
    Agent.HistoryPanel {
        id: historyPanel

        Component.onCompleted: {
            // Center window
            x = rootItem.Window.window.x + rootItem.Window.width/2 - historyPanel.width/2;
            y = rootItem.Window.window.y + rootItem.Window.height/2 - historyPanel.height/2;
        }

        onVisibleChanged: {
            //console.log("onVisibleChanged of historyPanel: visible = " + historyPanel.visible);

            if (visible) {
                // Add this window to the list of opened windows
                IngeScapeEditorC.addOpenedWindow(historyPanel);
            }
            else {
                // Remove this window from the list of opened windows
                IngeScapeEditorC.removeOpenedWindow(historyPanel);
            }
        }
    }


    //
    // Network Configuration (Popup)
    //
    NetworkConfiguration {
        id: networkConfigurationPopup

        anchors.centerIn: parent
    }


    //
    // License (Configuration) Popup
    //
    LicensePopup {
        id: licensePopup

        anchors.centerIn: parent

        licenseController: IngeScapeEditorC.licensesC
    }

    GettingStartedWindow {
        id: gettingStartedWindow
    }


    // Overlay layer used to display streaming
    I2Layer {
        id: streamingLayer

        objectName: "streamingLayer"

        anchors {
            left: leftPanel.right
            top: parent.top
            bottom: bottomPanel.top
            right: parent.right
            fill: null
        }
    }
}

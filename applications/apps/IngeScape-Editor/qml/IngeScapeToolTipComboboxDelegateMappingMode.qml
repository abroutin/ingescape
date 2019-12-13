import QtQuick 2.0

// Dependency for Tooltip
import QtQuick.Controls 1.4
import QtQuick.Controls.Private 1.0

import I2Quick 1.0
import INGESCAPE 1.0
import "qrc:/qml/styles/combobox/" as I2StyleComboboxPath


/**
  * This component is the based on the implementation of I2ComboboxBaseDelegate.
  *
  * It adds tooltips to the delegate component.
  * Tooltip mechanics are "old school" (pre QtQuick Controls 2.0) because
  * of a bug with trackpads, at least under Mac OSX.
  */
Item {
    id: comboboxDelegate

    property Item comboboxStyle: I2StyleComboboxPath.I2ComboboxStyle{}
    property Component component: component
    property int selection: -1

    // Called from the component's MouseArea
    // 'index' is the index of the clicked component inside the model.
    // MUST be redefined when using I2ComboboxBaseDelegate
    function onDelegateClicked(index) {}

    // Called from the component to get the text of the current item to display
    // 'index' is the index of the component to be displayed inside the model.
    // MUST be redefined when using I2ComboboxBaseDelegate
    function getItemText(index) {}

    Component {
        id: component

        Rectangle {
            id: rect
            color: (mouseArea.containsPress ? comboboxStyle.listBackgroundColorPressed
                                            : (selection === index ? comboboxStyle.listBackgroundColorSelected
                                                                   : comboboxStyle.listBackgroundColorIdle));

            height: comboboxDelegate.height
            width: comboboxDelegate.width

            border.width: comboboxStyle.listCellBorderWidth
            border.color: comboboxStyle.listCellBorderColor
            radius: 0;

            Item {
                id: containerPicto

                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }

                width : 25
                height: childrenRect.height

                SvgImage {
                    id: picto

                    anchors.horizontalCenter: parent.horizontalCenter

                    property string svgId : (index === 0) ? "mapping-mode-control" : "mapping-mode-observe"

                    svgElementId : (index === selection) ? svgId + "-selected" : svgId
                }
            }

            Text {
                id: text

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: containerPicto.right
                    right: parent.right
                }

                elide : Text.ElideRight
                font: comboboxStyle.itemsFont;
                color: (mouseArea.containsPress ? comboboxStyle.listItemTextColorPressed
                                                : (selection === index ? comboboxStyle.listItemTextColorSelected
                                                                       : comboboxStyle.listItemTextColorIdle));
                text: index >= 0 ? getItemText(index) : "";
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    onDelegateClicked(index);
                }

//                // Tooltip is handled without QtQuick.Controls 2.0
//                // Because of an issues with the trackpad (Window is no more interactive)
//                onExited: {
//                    Tooltip.hideText();
//                }
//                onCanceled: {
//                    Tooltip.hideText();
//                }
            }
        }
    }
}
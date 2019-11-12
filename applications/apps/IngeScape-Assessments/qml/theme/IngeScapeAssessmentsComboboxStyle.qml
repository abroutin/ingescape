/*
 *	IngeScape Assessments
 *
 *  Copyright © 2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Mathieu Soum       <soum@ingenuity.io>
 *
 */

import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import I2Quick 1.0

import INGESCAPE 1.0

import "qrc:/variables/fontawesome.js" as FontAwesome

I2ComboboxStyle {
    borderColorIdle: IngeScapeAssessmentsTheme.regularDarkBlueHeader
    borderWidthIdle:0
    currentTextColorIdle: IngeScapeAssessmentsTheme.regularDarkBlueHeader
    layerObjectName: "overlayLayerComboBox"
    font {
        family: IngeScapeTheme.textFontFamily
        weight: Font.Medium
        pixelSize: 16
    }
    dropdownIcon: FontAwesome.Icon.ChevronDown
}
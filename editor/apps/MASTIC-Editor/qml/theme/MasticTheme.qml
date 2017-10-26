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
 *      Justine Limoges    <limoges@ingenuity.io>
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

pragma Singleton
import QtQuick 2.5
import I2Quick 1.0


/**
  * Singleton that provides access to standard colors, fonts, etc.
  */
Item {
    id: theme


    //-----------------------------------------
    //
    // Sizes of UI elements
    //
    //-----------------------------------------

    // Minimum size of our application (Window)
    readonly property int appMinWidth: 1024
    readonly property int appMinHeight: 768

    // Default size of our application (Window)
    readonly property int appDefaultWidth: 1920
    readonly property int appDefaultHeight: 1080

    // Width of our left panel
    readonly property int leftPanelWidth: 320

    // Height of our bottom panel
    readonly property int bottomPanelHeight: 200


    //-----------------------------------------
    //
    // SVG files
    //
    //-----------------------------------------

    // - Main SVG file
    property var svgFileMASTIC : svgFileCacheMASTIC

    I2SvgFileCache {
        id: svgFileCacheMASTIC
        svgFile: "qrc:/resources/SVG/mastic-pictos.svg"
    }


    //-----------------------------------------
    //
    // Colors
    //
    //-----------------------------------------


    readonly property color whiteColor: "#FFFFFF"
    readonly property color blackColor: "#17191E"

    readonly property color redColor: "#E30513"
    readonly property color darkRedColor:"#7A110E"

    readonly property color veryDarkGreyColor: "#282D34"
    readonly property color darkGreyColor2: "#2B3137"
    readonly property color darkGreyColor: "#3C424F"
    readonly property color greyColor : "#525A66"
    readonly property color greyColor2 : "#3B424F"
    readonly property color blueGreyColor: "#8896AA"

    readonly property color greyColor4 : "#575756"
    readonly property color greyColor3 : "#706f6f"
    readonly property color lightGreyColor : "#9D9C9C"


    readonly property color darkBlueGreyColor: "#525A66"


    readonly property color orangeColor: "#F39200"


    //-----------------------------------------
    //
    // Fonts
    //
    //-----------------------------------------

    // Default font family
    readonly property string defaultFontFamily: "Arial"


    property alias labelFontFamilyExtraBold : fontAkrobatExtraBold.name
    property alias labelFontFamilyBlack : fontAkrobatBlack.name

    property alias labelFontFamily : fontAkrobat.name
    property alias textFontFamily : fontBloggerSans.name


    FontLoader {
        id: fontAkrobat
        source: "qrc:/resources/fonts/akrobat/Akrobat.ttf"
    }

    FontLoader {
        id: fontAkrobatBlack
        source: "qrc:/resources/fonts/akrobat/Akrobat-Black.ttf"
    }

    FontLoader {
        id: fontAkrobatExtraBold
        source: "qrc:/resources/fonts/akrobat/Akrobat-ExtraBold.ttf"
    }

    FontLoader {
        id: fontBloggerSans
        source: "qrc:/resources/fonts/blogger-sans/Blogger_Sans.ttf"
    }



    // Heading font (level1)
    readonly property font headingFont: Qt.font({
                                                    family: labelFontFamilyBlack,
                                                    pixelSize: 20,
                                                    weight: Font.Black
                                                });


    // Heading font (level2)
    readonly property font heading2Font: Qt.font({
                                                     family: textFontFamily,
                                                     pixelSize: 16
                                                 });

    // Normal font
    readonly property font normalFont: Qt.font({
                                                   family: textFontFamily,
                                                   pixelSize: 14
                                               });



    //-----------------------------------------
    //
    // Window
    //
    //-----------------------------------------

    // Background
    readonly property color windowBackgroundColor: "#E4E4E4"



    //-----------------------------------------
    //
    // Agents list UI elements
    //
    //-----------------------------------------


    // Background
    readonly property color leftPanelBackgroundColor: theme.blackColor
    readonly property color selectedTabsBackgroundColor: theme.veryDarkGreyColor

    // Labels
    readonly property color agentsListLabelColor: theme.whiteColor
    readonly property color agentsListLabel2Color: theme.whiteColor
    readonly property color agentsListPressedLabel2Color: theme.lightGreyColor
    readonly property color agentsListTextColor: theme.blueGreyColor

    readonly property color agentOFFLabelColor: theme.lightGreyColor
    readonly property color agentOFFLabel2Color: theme.lightGreyColor
    readonly property color agentOFFPressedLabel2Color: theme.greyColor
    readonly property color agentOFFTextColor: theme.greyColor2


    // List
    readonly property color agentsListItemBackgroundColor: theme.veryDarkGreyColor
    readonly property color agentsListItemRollOverBackgroundColor: theme.darkGreyColor2

    // Selected Agent
    readonly property color selectedAgentColor: theme.orangeColor



    //-----------------------------------------
    //
    // Agent Definition editors UI elements
    //
    //-----------------------------------------

    // Background
    readonly property color definitionEditorsBackgroundColor: "#282C88"

    // Labels
    readonly property color definitionEditorsLabelColor: theme.whiteColor



    //-----------------------------------------
    //
    // Scenario UI elements
    //
    //-----------------------------------------


    // Background
    readonly property color scenarioBackgroundColor: "#3C424F"



    //-----------------------------------------
    //
    // Agents mapping UI elements
    //
    //-----------------------------------------

    //
    // Background
    //
    readonly property color agentsMappingBackgroundColor: theme.blackColor
    // - grid
    readonly property color agentsMappingGridLineColor:  "#3E414B"
    readonly property color agentsMappingGridSublineColor: "#232830"

    //
    // Links
    //
    // - Default state
    readonly property int agentsMappingLinkDefaultWidth: 2
    readonly property int agentsMappingBrinDefaultWidth: 6


    readonly property color agentsMappingLinkDefaultColor: theme.whiteColor
    // - Hover state
    readonly property int agentsMappingLinkHoverFuzzyRadius: 4
    readonly property color agentsMappingLinkHoverFuzzyColor: "firebrick"

    // Press state
    readonly property color agentsMappingLinkPressColor: "firebrick"


    // links colors
    readonly property color orangeColor2 : "#e37724"
    readonly property color darkOrangeColor2 : "#53382c"

    readonly property color redColor2 : "#cf4f56"
    readonly property color darkRedColor2 : "#4d3033"

    readonly property color purpleColor : "#8e71b0"
    readonly property color darkPurpleColor : "#3b3649"

    readonly property color greenColor : "#14b2ad"
    readonly property color darkGreenColor : "#214848"


    //
    // Agents
    //
    readonly property color agentsONNameMappingColor : theme.whiteColor
    readonly property color agentsOFFNameMappingColor : theme.greyColor3

    readonly property color agentsONInputsOutputsMappingColor : theme.whiteColor
    readonly property color agentsOFFInputsOutputsMappingColor : theme.greyColor3

}

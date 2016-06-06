// Copyright (c) 2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import QtQuick.Controls 1.2
import "style.js" as Style

DefaultPanel {
    id: optionsPanel

    signal buttonClicked(string optionName, bool value)
    signal refreshOptions()

    Grid {
        id: optionsGrid
        columns: 3
        spacing: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.15
        anchors.horizontalCenter: parent.horizontalCenter

        CheckBox {
            id: windowBorders
            text: "Window borders"
            onClicked: buttonClicked("windowBorders", checked)
        }
        CheckBox {
            id: windowTitles
            text: "Window titles"
            onClicked: buttonClicked("windowTitles", checked)
        }
        CheckBox {
            id: autoFocusStreamers
            text: "Auto-Focus Streamers"
            onClicked: buttonClicked("autoFocusStreamers", checked)
        }
        CheckBox {
            id: statistics
            text: "Statistics"
            onClicked: buttonClicked("statistics", checked)
        }
        CheckBox {
            id: clock
            text: "Clock"
            onClicked: buttonClicked("clock", checked)
        }
        CheckBox {
            id: alphaBlending
            text: "Alpha blending"
            onClicked: buttonClicked("alphaBlending", checked)
        }
    }

    function updateCheckboxes(options) {
        windowBorders.checked = options.windowBorders
        windowTitles.checked = options.windowTitles
        autoFocusStreamers.checked = options.autoFocusStreamers
        statistics.checked = options.statistics
        clock.checked = options.clock
        alphaBlending.checked = options.alphaBlending
    }
    Component.onCompleted: refreshOptions()
}

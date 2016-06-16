// Copyright (c) 2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "style.js" as Style

DefaultPanel {
    id: optionsPanel

    signal buttonClicked(string optionName, bool value)
    signal refreshOptions()

    property int checkboxHeight: height * 0.025

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
            style: checkboxScalingStyle
        }
        CheckBox {
            id: windowTitles
            text: "Window titles"
            onClicked: buttonClicked("windowTitles", checked)
            style: checkboxScalingStyle
        }
        CheckBox {
            id: autoFocusStreamers
            text: "Auto-Focus Streamers"
            onClicked: buttonClicked("autoFocusStreamers", checked)
            style: checkboxScalingStyle
        }
        CheckBox {
            id: statistics
            text: "Statistics"
            onClicked: buttonClicked("statistics", checked)
            style: checkboxScalingStyle
        }
        CheckBox {
            id: clock
            text: "Clock"
            onClicked: buttonClicked("clock", checked)
            style: checkboxScalingStyle
        }
        CheckBox {
            id: alphaBlending
            text: "Alpha blending"
            onClicked: buttonClicked("alphaBlending", checked)
            style: checkboxScalingStyle
        }
    }

    Component {
        id: checkboxScalingStyle
        CheckBoxStyle {
            indicator: Rectangle {
                implicitHeight: checkboxHeight * 1.2
                implicitWidth: implicitHeight
                border.width: 0.05 * checkboxHeight
                radius: 0.1 * checkboxHeight
                Image {
                    source: "qrc:/images/check.svg"
                    anchors.fill: parent
                    anchors.margins: 0.15 * parent.width
                    sourceSize.width: width
                    sourceSize.height: height
                    opacity: control.checkedState === Qt.Checked ? control.enabled ? 1 : 0.5 : 0
                    Behavior on opacity {NumberAnimation {duration: 80}}
                }
            }
            spacing: checkboxHeight * 0.5
            label: Text {
                renderType: Text.NativeRendering
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: checkboxHeight
                text: control.text
            }
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

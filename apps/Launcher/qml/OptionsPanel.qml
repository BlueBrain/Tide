// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "style.js" as Style

DefaultPanel {
    id: optionsPanel

    signal buttonClicked(string optionName, bool value)
    signal exitClicked
    signal refreshOptions

    property int checkboxHeight: height * 0.025
    property int textSize: checkboxHeight * 0.8

    Grid {
        id: optionsGrid
        columns: 3
        spacing: checkboxHeight
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
            id: filePaths
            text: "File paths"
            onClicked: buttonClicked("filePaths", checked)
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
            text: "Transparent Windows"
            onClicked: buttonClicked("alphaBlending", checked)
            style: checkboxScalingStyle
        }
    }

    Rectangle {
        id: exitSlider
        color: Style.exitSliderBackgroundColor
        radius: height * Style.exitSliderRadius
        anchors.top: optionsGrid.bottom
        anchors.topMargin: height
        anchors.horizontalCenter: parent.horizontalCenter

        height: checkboxHeight * 2
        width: height * Style.exitSliderRelWidth

        Text {
            text: "Slide to exit"
            anchors.centerIn: parent
            font.pointSize: textSize
            color: Style.exitSliderTextColor
            opacity: slider.opacity
        }

        Rectangle {
            id: slider
            width: height
            height: parent.height
            radius: height * Style.exitSliderRadius
            color: Style.exitSliderColor
            opacity: (exitSlider.width - slider.x) / exitSlider.width
            MouseArea {
                anchors.fill: parent
                drag.target: slider
                drag.axis: Drag.XAxis
                drag.minimumX: 0
                drag.maximumX: exitSlider.width - slider.width
                onReleased: {
                    if (slider.x >= drag.maximumX)
                        exitClicked()
                    else
                        slider.x = 0.0
                }
            }
            Behavior on x {
                PropertyAnimation {
                }
            }
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
                    Behavior on opacity {
                        PropertyAnimation {
                            duration: 80
                        }
                    }
                }
            }
            spacing: checkboxHeight * 0.5
            label: Text {
                renderType: Text.NativeRendering
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: textSize
                text: control.text
            }
        }
    }

    function updateCheckboxes(options) {
        windowBorders.checked = options.windowBorders
        windowTitles.checked = options.windowTitles
        filePaths.checked = options.filePaths
        autoFocusStreamers.checked = options.autoFocusStreamers
        statistics.checked = options.statistics
        clock.checked = options.clock
        alphaBlending.checked = options.alphaBlending
    }
    Component.onCompleted: refreshOptions()
}

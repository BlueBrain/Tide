// Copyright (c) 2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import Tide 1.0
import "style.js" as Style

Item {
    property bool isWall: (typeof contentsync !== "undefined")

    ListView {
        id: buttons
        height: parent.height
        width: count * height
        orientation: ListView.Horizontal
        interactive: false // disable flickable behaviour
        delegate: ControlButton {
            height: buttons.height
            width: height
            image: action.checked ? action.iconChecked : action.icon
            opacity: action.enabled ? Style.buttonsEnabledOpacity : Style.buttonsDisabledOpacity
            MouseArea {
                visible: !isWall
                anchors.fill: parent
                onClicked: action.trigger()
            }
        }
        model: contentwindow.content.actions
    }
    Slider {
        id: progressBar
        anchors.right: parent.right
        anchors.left: buttons.right
        anchors.margins: Style.windowBorderWidth
        anchors.verticalCenter: parent.verticalCenter
        enabled: !isWall
        value: isWall ? contentsync.sliderPosition :
                        contentwindow.content.position / contentwindow.content.duration
        onValueChanged: {
            if (!isWall)
                contentwindow.content.position = value * contentwindow.content.duration
        }
        onPressedChanged: {
            if (!isWall)
                contentwindow.content.skipping = pressed
        }
        style: SliderStyle {
            groove: Rectangle {
                implicitWidth: 200 // Default size
                implicitHeight: Style.movieControlsLineThickness
                color: Style.contrastColor
            }
            handle: Rectangle {
                anchors.centerIn: parent
                color: contentwindow.content.skipping ? Style.highlightColor :
                                                        Style.contrastColor
                implicitWidth: Style.movieControlsHandleDiameter
                implicitHeight: Style.movieControlsHandleDiameter
                radius: 0.5 * Style.movieControlsHandleDiameter
            }
        }
    }
}

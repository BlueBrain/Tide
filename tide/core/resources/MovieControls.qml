// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import Tide 1.0
import "style.js" as Style

Item {
    property bool isMaster: (typeof contentsync === "undefined")

    height: buttons.height
    width: parent.width

    ContentActionsButtons {
        id: buttons
        orientation: ListView.Horizontal
        model: window.content.actions
    }
    Slider {
        id: progressBar
        anchors.right: parent.right
        anchors.left: buttons.right
        anchors.margins: Style.windowBorderWidth
        anchors.verticalCenter: parent.verticalCenter
        enabled: isMaster
        value: isMaster ? window.content.position
                          / window.content.duration : contentsync.sliderPosition
        onValueChanged: {
            if (isMaster)
                window.content.position = value * window.content.duration
        }
        onPressedChanged: {
            if (isMaster)
                window.content.skipping = pressed
        }
        style: SliderStyle {
            groove: Rectangle {
                implicitWidth: 200 // default size (unused)
                implicitHeight: Style.movieControlsLineThickness
                color: Style.contrastColor
            }
            handle: Rectangle {
                anchors.centerIn: parent
                color: window.content.skipping ? Style.highlightColor : Style.contrastColor
                implicitWidth: Style.movieControlsHandleDiameter
                implicitHeight: Style.movieControlsHandleDiameter
                radius: 0.5 * Style.movieControlsHandleDiameter
            }
        }
    }
}

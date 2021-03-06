// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import Tide 1.0
import "style.js" as Style

Item {
    property bool isMaster: (typeof contentsync === "undefined")

    height: playPauseButton.height
    width: parent.width

    PlayPauseButton {
        id: playPauseButton
        onClicked: contentcontroller.togglePlay()
    }
    Slider {
        id: progressBar
        anchors.right: parent.right
        anchors.left: playPauseButton.right
        anchors.leftMargin: Style.windowBorderWidth
        anchors.rightMargin: Style.windowBorderWidth
        anchors.verticalCenter: parent.verticalCenter
        enabled: isMaster
        value: isMaster ? window.content.position
                          / window.content.duration : contentsync.sliderPosition
        onValueChanged: {
            if (isMaster)
                contentcontroller.skipTo(value * window.content.duration)
        }
        onPressedChanged: {
            if (pressed)
                contentcontroller.startSkipping()
            else
                contentcontroller.stopSkipping()
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

// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import "style.js" as Style

Rectangle {
    id: defaultPanel

    anchors.fill: parent
    color: Style.defaultPanelColor

    property string tideVersion: "undef"
    property string tideRevision: "undef"

    Text {
        id: bigText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: smallText.top
        anchors.bottomMargin: 0.5 * smallText.height
        font.pixelSize: 2 * headerTextPixelSize
        text: "Tide"
        color: Style.defaultPanelTextColor
    }
    Text {
        id: smallText
        anchors.baseline: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: standardTextPixelSize
        text: "Tiled Interactive Display Environment"
        color: Style.defaultPanelTextColor
    }
    Text {
        id: version
        anchors.top: smallText.bottom
        anchors.topMargin: 0.5 * smallText.height
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: standardTextPixelSize
        text: "version " + tideVersion
        color: Style.defaultPanelTextColor
    }
    Text {
        id: revision
        anchors.top: version.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: standardTextPixelSize
        text: "rev. " + tideRevision
        color: Style.defaultPanelTextColor
    }
}

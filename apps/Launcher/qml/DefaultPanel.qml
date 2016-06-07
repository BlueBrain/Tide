// Copyright (c) 2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import "style.js" as Style

Rectangle {
    id: defaultPanel

    anchors.fill: parent
    color: Style.defaultPanelColor

    Text {
        id: bigText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.baseline: parent.verticalCenter
        font.pixelSize: 0.15 * parent.height
        text: "Tide"
        color: Style.defaultPanelTextColor
    }
    Text {
        id: smallText
        anchors.top: bigText.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 0.3 * bigText.font.pixelSize
        text: "Tiled Interactive DisplayWall Environment"
        color: Style.defaultPanelTextColor
    }
}

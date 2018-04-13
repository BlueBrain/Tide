// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0

Column {
    id: dialog
    property alias textInfo: infoText.text
    property alias textAccept: acceptText.text

    signal accepted()
    signal rejected()

    anchors.centerIn: parent
    spacing: curtain.width * 0.025
    Text {
        id: infoText
        text: "Do you want to proceed?"
        font.pixelSize: textPixelSize
        color: "white"
    }
    Rectangle {
        id: openAllButton
        color: "red"
        border.color: "white"
        border.width: 0.012 * width
        width: curtain.width * 0.2
        height: width * 0.35
        radius: height * 0.15
        anchors.horizontalCenter: parent.horizontalCenter

        Text {
            id: acceptText
            anchors.centerIn: parent
            text: "OK"
            font.pixelSize: textPixelSize
            font.bold: true
            color: "white"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dialog.accepted();
        }
    }
    Rectangle {
        id: closeButton
        color: "darkgrey"
        border.color: "white"
        border.width: 0.012 * width
        width: curtain.width * 0.2
        height: width * 0.35
        radius: height * 0.15
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            anchors.centerIn: parent
            text: "cancel"
            font.pixelSize: textPixelSize
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dialog.rejected();
        }
    }
}

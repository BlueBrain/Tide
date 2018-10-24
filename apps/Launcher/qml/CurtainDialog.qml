// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import "style.js" as Style

Column {
    id: dialog
    property alias textInfo: infoText.text
    property alias textAccept: acceptText.text

    signal accepted
    signal rejected

    anchors.centerIn: parent
    spacing: curtain.width * 0.025

    property real buttonHeight: standardTextPixelSize * Style.buttonHeightRelToTextHeight
    property real buttonWidth: curtain.width * 0.25

    Text {
        id: infoText
        text: "Do you want to proceed?"
        font.pixelSize: standardTextPixelSize
        color: "white"
    }
    Rectangle {
        id: openAllButton
        color: "red"
        border.color: "white"
        border.width: 0.012 * width
        width: buttonWidth
        height: buttonHeight
        radius: height * 0.15
        anchors.horizontalCenter: parent.horizontalCenter

        Text {
            id: acceptText
            anchors.centerIn: parent
            text: "OK"
            font.pixelSize: standardTextPixelSize
            color: "white"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dialog.accepted()
        }
    }
    Rectangle {
        id: closeButton
        color: "darkgrey"
        border.color: "white"
        border.width: 0.012 * width
        width: buttonWidth
        height: buttonHeight
        radius: height * 0.15
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            anchors.centerIn: parent
            text: "cancel"
            font.pixelSize: standardTextPixelSize
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dialog.rejected()
        }
    }
}

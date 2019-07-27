// Copyright (c) 2015-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQml.Models 2.1

import Launcher 1.0
import "style.js" as Style

Rectangle {
    id: listItem
    color: "transparent"

    signal clicked
    signal pressAndHold
    property string filePath
    FileBrowserImage {
        id: fileimage
        height: parent.height
        width: height
        file: filePath
    }

    Text {
        id: nameText
        text: fileName
        anchors.left: fileimage.right
        anchors.leftMargin: font.pixelSize
        anchors.right: sizeText.left
        elide: Text.ElideMiddle

        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: smallTextPixelSize
        color: Style.fileBrowserTextColor
    }

    Text {
        id: sizeText
        text: humanReadableFileSize(fileSize, true)
        anchors.right: modifiedText.left
        visible: !fileIsDir
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: smallTextPixelSize
        color: Style.fileBrowserTextColor
    }

    Text {
        id: modifiedText
        text: humanReadableModificationDate(fileModified)
        anchors.right: parent.right
        width: textColumnSize
        horizontalAlignment: Text.AlignRight

        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: smallTextPixelSize
        color: Style.fileBrowserTextColor
    }

    MouseArea {
        id: mouseRegion
        anchors.fill: parent
        onPressed: listItem.ListView.view.currentIndex = index
        onClicked: clickAnimation.start()
        onPressAndHold: listItem.pressAndHold()
    }

    ColorAnimation on color {
        id: clickAnimation
        running: false
        from: Style.fileBrowserBlinkColor
        to: "transparent"
        onStopped: listItem.clicked()
    }
}

// Copyright (c) 2015-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.4
import QtQuick.Controls 1.3

import Launcher 1.0
import "style.js" as Style

Rectangle {
    id: gridItem

    signal clicked
    signal pressAndHold

    color: "transparent"

    FileBrowserImage {
        anchors.fill: parent
        anchors.margins: 0.05 * gridItem.width
        file: filePath
    }

    Text {
        id: nameText
        text: fileName
        anchors.top: parent.bottom
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAnywhere
        maximumLineCount: 2
        elide: Text.ElideRight // ElideMiddle does not work with wrapped text
        color: Style.fileBrowserTextColor
        font.pixelSize: textPixelSize
    }

    Text {
        id: dateText
        text: humanReadableModificationDate(fileModified)
        visible: showModificationDate
        anchors.top: nameText.bottom
        anchors.topMargin: 0.5 * height
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        color: Style.fileBrowserTextColor
        font.pixelSize: textPixelSize
    }

    MouseArea {
        id: mouseRegion
        anchors.fill: parent
        onPressed: gridItem.GridView.view.currentIndex = index
        onClicked: clickAnimation.start()
        onPressAndHold: gridItem.pressAndHold()
    }

    ColorAnimation on color {
        id: clickAnimation
        running: false
        from: Style.fileBrowserBlinkColor
        to: "transparent"
        onStopped: gridItem.clicked()
    }
}

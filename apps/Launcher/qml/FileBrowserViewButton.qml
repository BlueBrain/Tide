// Copyright (c) 2015-2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import QtQuick.Controls 1.2
import "style.js" as Style

Rectangle {
    id: buttonItem
    height: 50
    width: height
    property alias icon: image.source

    property bool checked: false
    property var exclusiveGroup: null
    onExclusiveGroupChanged: {
        if (exclusiveGroup)
            exclusiveGroup.bindCheckable(buttonItem)
    }

    color: checked ? Style.fileBrowserDiscreteTextColor : Style.fileBrowserTitleBarColor
    border.color: "black"
    border.width: 0.05 * width
    radius: 0.1 * width

    Image {
        id: image
        anchors.fill: parent
        anchors.margins: parent.border.width
        source: ""
    }

    MouseArea {
        anchors.fill: parent
        onClicked: buttonItem.checked = true
    }
}

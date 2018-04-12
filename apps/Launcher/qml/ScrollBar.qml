// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>
//
// Note: a ScrollBar QML Type is available in Qt Quick Controls 2 starting from
// Qt 5.7. TODO deprectate this component when upgrading the minimum Qt version.

import QtQuick 2.0

Rectangle {
    id: scrollbar
    property variant flickable: parent

    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    width: 0.03 * parent.width
    radius: 0.5 * width
    color: "lightgrey"
    visible: flickable.visibleArea.heightRatio < 1.0
    clip: true

    Rectangle {
        id: handle
        width: parent.width
        height: Math.max(20, flickable.visibleArea.heightRatio * scrollbar.height)
        color: "black"
        opacity: clicker.drag.active ? 0.7 : 0.4
        radius: 0.5 * width
    }
    Binding {
        target: handle
        property: "y"
        value: flickable.visibleArea.yPosition * scrollbar.height
        when: !clicker.pressed
    }
    MouseArea {
        id: clicker
        anchors.fill: parent
        drag {
            target: handle
            minimumY: 0
            maximumY: scrollbar.height - handle.height
            axis: Drag.YAxis
        }
        onMouseYChanged: {
            flickable.contentY = handle.y / drag.maximumY * (flickable.contentHeight - flickable.height);
        }
        onClicked: {
            flickable.contentY = mouse.y / scrollbar.height * (flickable.contentHeight - flickable.height);
        }
    }
}

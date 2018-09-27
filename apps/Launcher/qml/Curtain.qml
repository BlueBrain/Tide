
// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0

Rectangle {
    id: infoRect
    color: "black"
    opacity: 0.7
    width: parent.width
    height: parent.height
    x: 0
    y: parent.height

    property bool open: false

    MouseArea {
        id: touchBarrier
        anchors.fill: parent
    }

    states: [
        State {
            name: "open"
            when: infoRect.open
            PropertyChanges {
                target: infoRect
                y: 0
            }
        }
    ]
    Behavior on y {
        PropertyAnimation {
            easing.type: Easing.InOutQuad
        }
    }
}

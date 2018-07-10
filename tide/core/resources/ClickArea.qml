// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0

Item {
    id: touchArea

    property bool active: true

    signal clicked

    Loader {
        active: (typeof groupcontroller !== "undefined") // only load on master
        anchors.fill: parent
        sourceComponent: MouseArea {
            onClicked: {
                if (touchArea.active)
                    touchArea.clicked()
            }
        }
    }
}

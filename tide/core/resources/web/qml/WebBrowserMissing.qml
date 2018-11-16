// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0

Rectangle {
    color: "gray"
    anchors.fill: parent
    property string url

    Text {
        text: "Qml web browser module is not installed, failed to load:\n\n" + url
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
    }
}

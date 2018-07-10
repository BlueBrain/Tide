// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import "style.js" as Style

ClickArea {
    property alias image: image.source
    property real size: Style.buttonsSize
    property real imageRelSize: Style.buttonsImageRelSize

    opacity: active ? Style.buttonsEnabledOpacity : Style.buttonsDisabledOpacity

    width: size
    height: size

    Image {
        id: image
        width: parent.width * imageRelSize
        height: parent.height * imageRelSize
        anchors.centerIn: parent
        // Force redraw the SVG
        sourceSize.width: width
        sourceSize.height: height
    }
}

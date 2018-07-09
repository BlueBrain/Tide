// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import "style.js" as Style

Item {
    id: button
    property alias image: image.source
    property real size: Style.buttonsSize
    property real imageRelSize: Style.buttonsImageRelSize
    property bool enabled: (typeof groupcontroller !== "undefined")
    property bool active: true

    opacity: active ? Style.buttonsEnabledOpacity : Style.buttonsDisabledOpacity

    signal clicked

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

    Component {
        id: touchArea
        MouseArea {
            onClicked: {
                if (button.active)
                    button.clicked()
            }
        }
    }

    Loader {
        sourceComponent: button.enabled ? touchArea : undefined
        anchors.fill: parent
    }
}

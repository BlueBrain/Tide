// Copyright (c) 2015-2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import "style.js" as Style

Item {
    id: wrapper
    property string file: ""

    Rectangle {
        id: placeholder
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Style.placeholderTopColor
            }
            GradientStop {
                position: 1.0
                color: Style.placeholderBottomColor
            }
        }
        visible: thumbnail.status !== Image.Ready
    }

    Image {
        id: thumbnail
        anchors.fill: placeholder
        fillMode: Image.PreserveAspectFit
        cache: false
        source: "image://thumbnail/" + wrapper.file
    }
}

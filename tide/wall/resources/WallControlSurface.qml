import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

ControlSurface {
    property alias frames: wallsurface.frames

    Clock {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 0.5 * width
        anchors.rightMargin: 0.5 * width
        visible: options.showClock
        displayedHeight: parent.height * Style.clockScale
        z: Style.overlayZorder
    }

    WallSurfaceElements {
        id: wallsurface
    }
}

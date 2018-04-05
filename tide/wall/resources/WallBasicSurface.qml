import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BasicSurface {
    property alias frames: walloverlay.frames

    WallOverlay {
        id: walloverlay
        anchors.fill: parent
        z: Style.overlayZorder
        showClock: false
    }
}

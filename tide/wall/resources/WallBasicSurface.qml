import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

Item {
    property alias frames: wallsurface.frames

    WallSurfaceElements {
        id: wallsurface
    }
}

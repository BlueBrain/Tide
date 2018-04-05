import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Text {
    property int frames: 0 // incremented each frame by the C++ backend

    text: timer.fps + " FPS"
    font.pixelSize: Style.fpsFontSize
    color: Style.fpsFontColor

    Timer {
        id: timer
        property real fps: 0
        interval: 1000 /*ms*/
        repeat: true
        running: parent.visible
        onTriggered: {
            fps = frames
            frames = 0
        }
        onRunningChanged: frames = 0
    }
}

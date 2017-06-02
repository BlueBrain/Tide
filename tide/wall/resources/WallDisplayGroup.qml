import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

DisplayGroup {
    PowerOffCountdown {
        anchors.fill: parent
        z: Style.countdownZorder
    }

    Clock {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 0.5 * width
        anchors.rightMargin: 0.5 * width
        visible: options.showClock
        displayedHeight: displaygroup.height * Style.clockScale
        z: Style.overlayZorder
    }

    property int frames: 0

    Timer {
        id: fpsTimer
        property real fps: 0
        interval: 1000
        repeat: true
        running: options.showStatistics
        onTriggered: {
            fps = frames
            frames = 0
        }
        onRunningChanged: frames = 0
    }

    Text {
        visible: options.showStatistics
        text: fpsTimer.fps + " FPS"
        font.pixelSize: Style.fpsFontSize
        color: Style.fpsFontColor
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: Style.fpsX
        anchors.topMargin: -parent.y + Style.fpsY
        z: Style.overlayZorder
    }

    Repeater {
        id: touchpointsMarker
        anchors.fill: parent
        model: markers
        delegate: TouchPointMarker {
        }
    }
}

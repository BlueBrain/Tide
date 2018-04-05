import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

Item {
    property alias frames: fpsCounter.frames
    property alias showClock: clock.show

    Clock {
        id: clock
        property bool show: false
        visible: show && options.showClock
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 0.5 * width
        anchors.rightMargin: 0.5 * width
        displayedHeight: parent.height * Style.clockScale
    }

    FpsCounter {
        id: fpsCounter
        visible: options.showStatistics
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: Style.fpsX
        anchors.topMargin: -parent.y + Style.fpsY
    }

    PowerOffCountdown {
        anchors.fill: parent
    }

    Repeater {
        id: touchpointsMarker
        anchors.fill: parent
        model: markers
        delegate: TouchPointMarker {
        }
    }
}

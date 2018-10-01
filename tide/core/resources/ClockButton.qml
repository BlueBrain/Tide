import QtQuick 2.0
import Tide 1.0

ControlButton {
    Clock {
        displayedHeight: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        showSeconds: false
    }
}

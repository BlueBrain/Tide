import QtQuick 2.0
import Tide 1.0

ControlButton {
    image: "qrc:///img/maximize.svg"
    onClicked: groupcontroller.showFullscreen(window.id)
}

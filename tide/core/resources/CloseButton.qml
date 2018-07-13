import QtQuick 2.0
import Tide 1.0

ControlButton {
    image: "qrc:///img/close.svg"
    onClicked: groupcontroller.removeLater(window.id)
}

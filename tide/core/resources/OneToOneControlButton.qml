import QtQuick 2.0
import Tide 1.0

ControlButton {
    image: "qrc:///img/oneToOne.svg"
    visible: !window.isPanel && !window.focused
}

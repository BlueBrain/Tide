import QtQuick 2.0
import Tide 1.0

ControlButton {
    image: "qrc:/img/launch.svg"

    size: parent.width

    onClicked: sidecontroller.toggleLauncher()
}

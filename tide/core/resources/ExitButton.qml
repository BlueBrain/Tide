import QtQuick 2.0
import Tide 1.0

ControlButton {
    image: "qrc:/img/exit.svg"

    visible: displaygroup.hasFullscreenWindows
             || displaygroup.hasFocusedWindows
    onClicked: {
        if (displaygroup.hasFullscreenWindows)
            groupcontroller.exitFullscreen()
        else if (displaygroup.hasFocusedWindows)
            groupcontroller.unfocusAll()
    }
}

import QtQuick 2.0
import Tide 1.0
import TideMaster 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

Scene {
    id: sceneitem

    anchors.fill: parent

    signal openLauncher

    sideControl.buttonDelegate: MultitouchArea {
        onTap: {
            if (buttonIndex == 0) {
                if (displaygroup.hasVisiblePanels)
                    groupcontroller.hidePanels()
                else
                    openLauncher()
            } else if (buttonIndex == 1)
                groupcontroller.unfocusAll()
            else if (buttonIndex == 2)
                groupcontroller.exitFullscreen()
            else if (buttonIndex == 3) {
                if (lock.locked)
                    lock.unlock()
                else
                    lock.lock()
            }
        }
    }
    streamNotificationArea.buttonDelegate: MultitouchArea {
        onTap: {
            if (buttonIndex == 0)
                lock.rejectStream(streamName)
            else if (buttonIndex == 1)
                lock.acceptStream(streamName)
        }
    }
}

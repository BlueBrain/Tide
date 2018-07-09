import QtQuick 2.0
import Tide 1.0

ControlButton {
    image: lock.locked ? "qrc:/img/lock.svg" : "qrc:/img/unlock.svg"
    onClicked: {
        if (lock.locked)
            lock.unlock()
        else
            lock.lock()
    }
}


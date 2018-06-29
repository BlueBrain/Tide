import QtQuick 2.0
import Tide 1.0

Column {
    property int buttonCount: window.isPanel ? 1 : window.focused ? 3 : 4

    CloseControlButton {
    }
    OneToOneControlButton {
        visible: !window.isPanel && !window.focused
    }
    FullscreenControlButton {
        visible: !window.isPanel
    }
    FocusControlButton {
        visible: !window.isPanel
        onClicked: {
            if (window.focused)
                groupcontroller.unfocus(window.id)
            else
                groupcontroller.focusSelected()
        }
    }
}

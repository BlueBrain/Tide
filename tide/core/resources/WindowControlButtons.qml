import QtQuick 2.0
import Tide 1.0

Column {
    width: childrenRect.width
    height: childrenRect.height

    CloseButton {
        visible: !window.focused
    }
    OneToOneButton {
        visible: !window.isPanel && !window.focused
    }
    FullscreenButton {
        visible: !window.isPanel
    }
    FocusButton {
        visible: !window.isPanel
        image: window.focused ? "qrc:/img/unfocus.svg" : "qrc:/img/focus.svg"
        onClicked: {
            if (window.focused)
                groupcontroller.unfocus(window.id)
            else
                groupcontroller.focusSelected()
        }
    }
    Loader {
        id: keyboardButton
        active: window.content.keyboard !== null
        sourceComponent: KeyboardButton {
            onClicked: contentcontroller.keyboard.toggle()
        }
    }
}

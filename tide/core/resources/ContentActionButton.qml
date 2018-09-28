import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: buttons
    property var model: displaygroup.fullscreenWindow
    property int buttonCount: (playPauseButton.active ? 1 : 0) + (keyboardButton.active ? 1 : 0)
                              + (rescaleButton.active ? 1 : 0)

    signal toggleKeyboard
    signal togglePlay

    visible: buttonCount > 0

    width: childrenRect.width
    height: visible ? childrenRect.height : 0

    Loader {
        id: keyboardButton
        active: buttons.model !== null
                && buttons.model.content.keyboard !== null
        sourceComponent: KeyboardButton {
            model: buttons.model !== null ? buttons.model.content : null
            onClicked: sidecontroller.toggleKeyboard()
        }
    }
    Loader {
        id: playPauseButton
        active: buttons.model !== null
                && (typeof buttons.model.content.playing !== "undefined")
        sourceComponent: PlayPauseButton {
            model: buttons.model !== null ? buttons.model.content : null
            onClicked: sidecontroller.togglePlay()
        }
    }
    Loader {
        id: rescaleButton
        active: buttons.model !== null
                && !buttons.model.content.captureInteraction
                && !playPauseButton.active && !keyboardButton.active
        sourceComponent: RescaleButton {
            model: buttons.model
            onClicked: sidecontroller.toggleResize()
            onTapAndHold: sidecontroller.setOneToOneSize()
        }
    }
}

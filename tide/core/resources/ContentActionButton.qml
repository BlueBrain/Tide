import QtQuick 2.0
import "style.js" as Style

ControlButton {
    image: action.checked ? action.iconChecked : action.icon
    opacity: action.enabled ? Style.buttonsEnabledOpacity : Style.buttonsDisabledOpacity
    onClicked: action.trigger()
}

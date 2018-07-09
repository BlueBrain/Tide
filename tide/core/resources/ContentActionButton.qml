import QtQuick 2.0
import "style.js" as Style

ControlButton {
    image: action.checked ? action.iconChecked : action.icon
    active: action.enabled
    onClicked: action.trigger()
}

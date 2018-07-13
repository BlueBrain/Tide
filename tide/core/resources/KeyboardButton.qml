import QtQuick 2.0
import "style.js" as Style

ControlButton {
    image: "qrc:/img/keyboard.svg"

    property var model: window.content

    active: model !== null && typeof model.keyboard !== "undefined"
}

import QtQuick 2.0
import "style.js" as Style

ControlButton {
    image: active && model.playing ? "qrc:/img/pause.svg" : "qrc:/img/play.svg"

    property var model: window.content

    active: model !== null && typeof model.playing !== "undefined"
}

import QtQuick 2.0
import "style.js" as Style

ControlButton {
    image: big ? "qrc:/img/reduce.svg" : "qrc:/img/enlarge.svg"

    property var model: window

    property bool big: model !== null
                       && (model.fullscreenCoordinates.width > displaygroup.width
                           || model.fullscreenCoordinates.height > displaygroup.height)
}

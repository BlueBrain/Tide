import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Rectangle {
    anchors.fill: parent
    color: "transparent"

    property alias delegate: repeater.delegate

    visible: !window.isPanel
             && window.selected
             && window.mode === Window.STANDARD
    opacity: Style.resizeCircleOpacity

    Repeater {
        id: repeater
        model: [Window.TOP_LEFT, Window.TOP,
                Window.TOP_RIGHT, Window.RIGHT,
                Window.BOTTOM_RIGHT, Window.BOTTOM,
                Window.BOTTOM_LEFT, Window.LEFT]
        delegate: ResizeCircle {
        }
    }
}

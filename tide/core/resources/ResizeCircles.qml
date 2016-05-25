import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Rectangle {
    anchors.fill: parent
    color: "transparent"

    property alias delegate: repeater.delegate

    visible: !contentwindow.isPanel
             && contentwindow.controlsVisible
             && contentwindow.mode === ContentWindow.STANDARD
    opacity: Style.resizeCircleOpacity

    Repeater {
        id: repeater
        model: [ContentWindow.TOP_LEFT, ContentWindow.TOP,
                ContentWindow.TOP_RIGHT, ContentWindow.RIGHT,
                ContentWindow.BOTTOM_RIGHT, ContentWindow.BOTTOM,
                ContentWindow.BOTTOM_LEFT, ContentWindow.LEFT]
        delegate: ResizeCircle {
        }
    }
}

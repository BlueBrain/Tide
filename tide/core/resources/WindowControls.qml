import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Item {
    property alias color: buttonsRectangle.color

    id: windowControls
    width: buttonsRectangle.width
    anchors.right: parent.left
    anchors.top: parent.top
    anchors.rightMargin: Style.controlsLeftMargin
    visible: opacity > 0
    opacity: 0

    Rectangle {
        id: buttonsRectangle
        width: buttons.width + 2 * Style.buttonsPadding
        height: buttons.height
        color: Style.controlsDefaultColor
    }
    Triangle {
        id: triangle
        width: buttonsRectangle.width
        height: 2 * width
        color: buttonsRectangle.color
        anchors.top: buttonsRectangle.bottom
    }

    WindowControlButtons {
        id: buttons
        anchors.horizontalCenter: buttonsRectangle.horizontalCenter
        anchors.topMargin: Style.buttonsPadding
        anchors.top: buttonsRectangle.top
    }

    states: [
        State {
            name: "focus_mode"
            when: window.focused
            extend: "opaque"
        },
        State {
            name: "opaque"
            when: window.selected &&
                  window.state !== Window.RESIZING &&
                  !window.fullscreen
            PropertyChanges {
                target: windowControls
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                target: windowControls
                property: "opacity"
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
    ]
}

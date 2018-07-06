import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Item {
    property alias buttons: buttons
    property alias fixedButtons: buttons.headerItem
    property alias color: buttonsRectangle.color
    property bool contentActionsVisible: true

    id: windowControls
    width: buttonsRectangle.width
    anchors.right: parent.left
    anchors.top: parent.top
    anchors.rightMargin: Style.controlsLeftMargin
    visible: opacity > 0
    opacity: 0

    Rectangle {
        id: buttonsRectangle
        width: buttons.width + 2 * Style.buttonsMargin
        height: buttons.height
        color: Style.controlsDefaultColor
    }
    Triangle {
        id: triangle
        width: buttonsRectangle.width
        color: buttonsRectangle.color
        anchors.top: buttonsRectangle.bottom
    }
    ListView {
        id: buttons
        width: Style.buttonsSize
        height: (count + headerItem.buttonCount) * Style.buttonsSize
        anchors.horizontalCenter: buttonsRectangle.horizontalCenter
        anchors.topMargin: Style.buttonsMargin
        anchors.top: buttonsRectangle.top
        orientation: ListView.Vertical
        header: FixedControlButtons {
        }
        interactive: false // disable flickable behaviour

        delegate: ContentActionButton {
        }
        model: contentActionsVisible ? window.content.actions : undefined
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

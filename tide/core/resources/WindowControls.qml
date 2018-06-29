import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Rectangle {
    property alias buttons: buttons
    property alias fixedButtons: buttons.headerItem
    property bool contentActionsVisible: true

    id: windowControls
    width: buttons.width + radius + (Style.buttonsSize * (1.0 - Style.buttonsImageRelSize))
    height: buttons.height + (Style.buttonsSize * (1.0 - Style.buttonsImageRelSize))
    color: Style.controlsDefaultColor
    border.color: color
    border.width: Style.controlsBorderWidth
    radius: Style.controlsRadius
    anchors.right: parent.left
    anchors.top: parent.top
    anchors.rightMargin: Style.controlsLeftMargin
    visible: opacity > 0
    opacity: 0

    ListView {
        id: buttons
        width: Style.buttonsSize
        height: (count + headerItem.buttonCount) * Style.buttonsSize
        anchors.centerIn: parent
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

import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Rectangle {
    property alias listview: buttons

    id: windowControls
    width: buttons.width + radius + (Style.buttonsSize - Style.buttonsImageSize)
    height: buttons.height + (Style.buttonsSize - Style.buttonsImageSize)
    color: Style.controlsDefaultColor
    border.color: Style.controlsDefaultColor
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
        property int fixed_buttons_count: contentwindow.isPanel ? 1 : 4
        height: (count + fixed_buttons_count) * Style.buttonsSize
        anchors.centerIn: parent
        orientation: ListView.Vertical
        header: fixedButtonsDelegate
        interactive: false // disable flickable behaviour

        Component {
            id: fixedButtonsDelegate
            Column {
                CloseControlButton {
                }
                OneToOneControlButton {
                }
                FullscreenControlButton {
                }
                FocusControlButton {
                }
            }
        }
        delegate: WindowControlsDelegate {
        }
        model: contentwindow.content.actions
    }

    states: [
        State {
            name: "focus_mode"
            when: contentwindow.focused
            extend: "opaque"
            PropertyChanges {
                target: windowControls
                color: Style.controlsFocusedColor
                border.color: Style.controlsFocusedColor
            }
            PropertyChanges {
                target: buttons
                fixed_buttons_count: 3
            }
        },
        State {
            name: "opaque"
            when: contentwindow.controlsVisible &&
                  contentwindow.state !== ContentWindow.RESIZING &&
                  !contentwindow.fullscreen
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
            ColorAnimation { duration: Style.focusTransitionTime }
            ColorAnimation { target: border; duration: Style.focusTransitionTime }
        }
    ]
}

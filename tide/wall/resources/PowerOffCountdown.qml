import QtQuick 2.0
import Tide 1.0

import "qrc:/qml/core/style.js" as Style

Item {
    Rectangle {
        id: countdown
        anchors.fill: parent
        color: Style.focusContextColor

        states: [
            State {
                name: "hidden"
                when: !timer.isCountdownActive()
                PropertyChanges {
                    target: countdown
                    opacity: 0.0
                }
            },
            State {
                name: "visible"
                when: timer.isCountdownActive()
                PropertyChanges {
                    target: countdown
                    opacity: 0.9
                }
            }
        ]

        transitions: [
            Transition {
                from: "hidden"
                to: "visible"
                NumberAnimation {
                    properties: "opacity"
                    easing.type: Easing.OutBack
                    duration: timer.getCountdownTimeout()
                }
            },
            Transition {
                from: "visible"
                to: "hidden"
                NumberAnimation {
                    properties: "opacity"
                    easing.type: Easing.OutBack
                    duration: Style.countdownTransitionTime
                }
            }
        ]

        Text {
            text: "Touch to prevent sleep!"
            visible: timer.isCountdownActive() ? 1 : 0
            font.pointSize: displaygroup.height * Style.countdownTextScale
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
            style: Text.Outline
            styleColor: "black"
        }
    }
}

import QtQuick 2.0
import Tide 1.0

import "qrc:/qml/core/style.js" as Style

Rectangle {
    id: countdown
    color: Style.focusContextColor

    states: [
        State {
            name: "hidden"
            when: !countdownStatus.active
            PropertyChanges {
                target: countdown
                opacity: 0.0
            }
        },
        State {
            name: "visible"
            when: countdownStatus.active
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
                duration: countdownStatus.duration
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
        visible: countdownStatus.active
        font.pointSize: countdown.height * Style.countdownTextScale
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        color: "white"
        style: Text.Outline
        styleColor: "black"
    }
}

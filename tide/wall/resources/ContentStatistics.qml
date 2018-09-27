import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

Text {
    id: statistics
    text: contentsync.statistics
    visible: options.showStatistics

    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.margins: Style.statisticsBorderMargin

    font.pixelSize: Style.statisticsFontSize
    color: Style.statisticsFontColor

    states: [
        State {
            name: "fullsceen"
            when: window.fullscreen
            AnchorChanges {
                target: statistics
                anchors.bottom: undefined
                anchors.left: undefined
            }
            PropertyChanges {
                target: statistics
                x: -parent.x + Style.statisticsBorderMargin
                y: displaygroup.height - height - parent.y - Style.statisticsBorderMargin
            }
        }
    ]
}

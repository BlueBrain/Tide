import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Rectangle {
    width: Style.controlPanelWidth
    height: Style.controlPanelHeight

    visible: options.showControlArea

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: Style.controlPanelPadding

    property Component buttonDelegate: ControlPanelDelegate {
    }

    border.color: Style.controlsFocusedColor
    border.width: Style.resizeCircleRadius
    color: Style.controlPanelBackground

    ListView {
        anchors.fill: parent
        anchors.margins: Style.controlPanelPadding
        spacing: Style.controlPanelSectionsMargin
        interactive: false // Don't let users scroll the list

        model: ListModel {
            ListElement {
                title: "CONTENT    "
                subItems: [
                    ListElement {
                        label: "Open file"
                        icon: "qrc:///img/add.svg"
                        action: QmlControlPanel.OPEN_CONTENT
                    },
                    ListElement {
                        label: "Start application"
                        icon: "qrc:///img/launch.svg"
                        action: QmlControlPanel.OPEN_APPLICATION
                    }
                ]
            }
            ListElement {
                title: "SESSION    "
                subItems: [
                    ListElement {
                        label: "Clear"
                        icon: "qrc:///img/close.svg"
                        action: QmlControlPanel.NEW_SESSION
                    },
                    ListElement {
                        label: "Open"
                        icon: "qrc:///img/play.svg"
                        action: QmlControlPanel.LOAD_SESSION
                    }
                ]
            }
        }
        delegate: sectionDelegate
    }

    Component {
        id: sectionDelegate

        ListView {
            width: parent.width
            height: Style.controlPanelTextSize * count + headerItemHeight
            spacing: Style.controlPanelTextSpacing
            interactive: false // Don't let users scroll the list
            // workaround for QtQuick1 which is missing the headerItem property
            property real headerItemHeight: Style.controlPanelTextSize
                                            + Style.controlPanelPadding

            model: subItems
            delegate: buttonDelegate
            header: Text {
                text: title
                font.underline: true
                font.pixelSize: Style.controlPanelTextSize
                height: font.pixelSize + Style.controlPanelPadding
            }
        }
    }
}

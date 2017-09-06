import QtQuick 2.0
import Tide 1.0
import QtQuick.Layouts 1.2
import "qrc:/qml/core/style.js" as Style

ListView {
    property Component buttonDelegate: Item {
    }
    property int itemHeight: Style.streamNotificationAreaBannerRelHeight * displaygroup.height

    width: childrenRect.width
    height: itemHeight * 10
    z: Style.streamNotificationZorder
    spacing: 3
    model: lock.streamList

    delegate: Rectangle {
        id: banner
        width: childrenRect.width
        height: itemHeight
        radius: height / 2
        color: Style.sideButtonColor

        RowLayout {
            id: row
            Text {
                id: streamName
                text: modelData
                Layout.leftMargin: sideControl.width
                Layout.preferredWidth: itemHeight * 4
                Layout.preferredHeight: banner.height / 2
                Layout.margins: banner.height / 4
                font.family: "Verdana"
                font.pixelSize: banner.height / 2
                minimumPixelSize: banner.height / 5
                wrapMode: Text.Wrap
                fontSizeMode: Text.VerticalFit
                maximumLineCount: 2
            }

            Image {
                source: "qrc:/img/close.svg"
                Layout.preferredHeight: banner.height / 2
                Layout.preferredWidth: banner.height / 2
                Layout.topMargin: banner.height / 4
                Layout.bottomMargin: banner.height / 4
                Loader {
                    property int buttonIndex: 0
                    property string streamName: modelData
                    anchors.fill: parent
                    sourceComponent: buttonDelegate
                }
            }

            Image {
                source: "qrc:/img/tick.svg"
                Layout.preferredHeight: banner.height / 2
                Layout.preferredWidth: banner.height / 2
                Layout.topMargin: banner.height / 4
                Layout.bottomMargin: banner.height / 4
                Layout.rightMargin: banner.height / 4
                Loader {
                    property int buttonIndex: 1
                    property string streamName: modelData
                    anchors.fill: parent
                    sourceComponent: buttonDelegate
                }
            }
        }
    }
}

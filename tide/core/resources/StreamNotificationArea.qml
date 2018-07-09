import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

ListView {
    id: notificationArea

    model: lock.streamList
    property real leftPadding: 0

    spacing: Style.buttonsPadding

    delegate: StreamNotificationBanner {
        leftPadding: notificationArea.leftPadding
    }
}

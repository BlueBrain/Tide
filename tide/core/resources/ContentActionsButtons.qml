import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

ListView {
    id: listView
    orientation: ListView.Vertical
    property real size: Style.buttonsSize
    height: (orientation === ListView.Horizontal ? count : 1) * size
    width: (orientation === ListView.Vertical ? count : 1) * size
    interactive: false // disable flickable behaviour
    delegate: ContentActionButton {
        size: listView.size
    }
}

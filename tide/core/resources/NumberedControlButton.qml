import QtQuick 2.0

ControlButton {
    property int itemCount: -1
    active: itemCount > 0
    Text {
        visible: parent.itemCount >= 0
        text: parent.itemCount
        font.pixelSize: 0.4 * parent.height
        anchors.right: parent.right
    }
}

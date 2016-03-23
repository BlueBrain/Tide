import QtQuick 2.0

Image {
    property alias mousearea: mousearea

    width: 0.4 * Math.min(parent.width, parent.height)
    height: width
    sourceSize.width: 512
    sourceSize.height: 512

    MouseArea {
        id: mousearea
        // Work around to prevent MouseArea from stealing touch events
        hoverEnabled: true
        acceptedButtons: containsMouse ? Qt.LeftButton : Qt.NoButton
        anchors.fill: parent
    }
}

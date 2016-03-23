import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: button
    width: parent.width
    height: Style.controlPanelTextSize

    Row {
        anchors.fill: parent
        spacing: Style.controlPanelTextSpacing
        Image {
            source: icon
            height: button.height
            width: height
        }
        Text {
            text: label
            font.capitalization: Font.AllUppercase
            font.pixelSize: button.height * 0.8 // Fill the button vertically
        }
    }
}

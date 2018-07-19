import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    property alias text: textBanner.text

    anchors.left: parent.left
    anchors.top: parent.top
    anchors.leftMargin: line.height
    anchors.topMargin: line.height * 3
    Rectangle {
        id: line
        color: Style.surfaceControlsColor
        height: Style.backgroundTextSize * 0.6
        width: textBanner.width
    }
    Text {
        id: textBanner
        anchors.top: line.bottom
        anchors.topMargin: Style.backgroundTextSize * 0.45
        FontLoader { id: gothamBook; source: "qrc:/fonts/Gotham-Book.otf"; name: "qrc::gotham-book" }
        font { family: "qrc::gotham-book"; bold: true; pixelSize: Style.backgroundTextSize }
    }
}

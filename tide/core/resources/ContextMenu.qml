import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Rectangle {
    color: Style.sideButtonColor
    height: row.height + 2 * row.spacing
    width: row.width + 2 * row.spacing

    Image {
        id: indicator
        source: "qrc:/img/context_menu_indicator.svg"
        width: Style.buttonsSize
        height: Style.buttonsSize
        anchors.horizontalCenter: parent.left
        anchors.verticalCenter: parent.top
        // Force redraw the SVG
        sourceSize.width: width
        sourceSize.height: height
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Style.buttonsMargin

        FocusControlButton {
            onClicked: {
                groupcontroller.toggleFocusAll()
                contextmenu.visible = false
            }
        }
        VerticalButtonSeparator {
            visible: copyButton.visible
        }
        CopyControlButton {
            id: copyButton
            itemCount: displaygroup.selectedUris.length
            onClicked: {
                contextmenu.copiedUris = displaygroup.selectedUris
                contextmenu.visible = false
            }
            visible: itemCount > 0
        }
        VerticalButtonSeparator {
            visible: pasteButton.visible
        }
        PasteControlButton {
            id: pasteButton
            itemCount: contextmenu.copiedUris.length
            onClicked: {
                scenecontroller.paste(contextmenu.copiedUris)
                contextmenu.copiedUris = {}
                contextmenu.visible = false
            }
            visible: itemCount > 0
        }
    }
}

import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BaseContentWindow {
    id: windowRect

    // for contents with alpha channel such as SVG or PNG
    color: options.alphaBlending ? "transparent" : "black"

    property string imagesource: "image://texture/" + contentwindow.content.uri

    contentComponent: Item {
        id: contentItem
        objectName: "TilesParent"

        // Used to let a background tile fill its parent
        width: contentsync.tilesArea.width
        height: contentsync.tilesArea.height

        // Tiles bind to this signal from c++ to toggle borders visibility
        property bool showTilesBorder: options.showContentTiles
        // The auto-generated notifier does not emit the new value, do it
        signal showTilesBordersValueChanged(bool value)
        onShowTilesBorderChanged: showTilesBordersValueChanged(showTilesBorder)

        transform: [
            // Adjust tiles to content area
            Scale {
                xScale: contentArea.width / contentsync.tilesArea.width
                yScale: contentArea.height / contentsync.tilesArea.height
            },
            // Apply content zoom
            Translate {
                x: -contentwindow.content.zoomRect.x * contentArea.width
                y: -contentwindow.content.zoomRect.y * contentArea.height
            },
            Scale {
                xScale: 1.0 / contentwindow.content.zoomRect.width
                yScale: 1.0 / contentwindow.content.zoomRect.height
            }
        ]
    }

    ZoomContext {
    }

    Text {
        id: statistics
        text: contentsync.statistics
        visible: options.showStatistics

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: Style.statisticsBorderMargin
        anchors.bottomMargin: Style.statisticsBorderMargin
        font.pointSize: Style.statisticsFontSize
        color: Style.statisticsFontColor
    }

    ParallelAnimation {
        id: openingAnimation
        NumberAnimation {
            target: windowRect
            property: "x"
            from: -contentwindow.width
            to: contentwindow.x
            duration: Style.panelsAnimationTime
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: windowRect
            property: "opacity"
            from: 0
            to: 1
            duration: Style.panelsAnimationTime
            easing.type: Easing.InOutQuad
        }
    }
    Component.onCompleted: if(contentwindow.isPanel) {openingAnimation.start()}
}

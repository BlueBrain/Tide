import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BaseWindow {
    id: windowRect

    // for contents with alpha channel such as SVG or PNG
    color: options.alphaBlending ? "transparent" : Style.transparentContentsBackgroundColor

    property string imagesource: "image://texture/" + window.content.uri

    virtualKeyboard.onLoaded: {
        // Display keyboard state from the master processes
        // Proxy functions are needed because window is a context property which
        // changes constantly, so regular property bindings would be inoperant
        virtualKeyboard.item.shiftActive = Qt.binding(function () {
            return window.content.keyboard.shift
        })
        virtualKeyboard.item.symbolsActive = Qt.binding(function () {
            return window.content.keyboard.symbols
        })
        virtualKeyboard.item.activeKeyId = Qt.binding(function () {
            return window.content.keyboard.activeKeyId
        })
    }

    contentComponent: Item {
        id: contentItem
        objectName: "TilesParent"

        // WindowRenderer (C++) expects these items to be the first children
        Repeater {
            model: contentsync.lodCount
            Item {
                // draw highest LOD (top of pyramid) below lowest LOD (bottom)
                property int lod: contentsync.lodCount - index - 1

                // For alpha blending to work only render current LOD (even when
                // option.alphaBlending is off)
                visible: !window.content.transparency || contentsync.lod === lod

                // connection + function call instead of automatic property
                // binding due to lack of support for QList<QSize> in Qml
                width: contentsync.getTilesArea(lod).width
                height: contentsync.getTilesArea(lod).height
                Connections {
                    target: contentsync
                    onTilesAreasChanged: {
                        width = contentsync.getTilesArea(lod).width
                        height = contentsync.getTilesArea(lod).height
                    }
                }

                transform: [
                    // Adjust tiles to content area
                    Scale {
                        xScale: contentArea.width / width
                        yScale: contentArea.height / height
                    },
                    // Apply content zoom
                    Translate {
                        x: -window.content.zoomRect.x * contentArea.width
                        y: -window.content.zoomRect.y * contentArea.height
                    },
                    Scale {
                        xScale: 1.0 / window.content.zoomRect.width
                        yScale: 1.0 / window.content.zoomRect.height
                    }
                ]
            }
        }

        // Tiles bind to this signal from c++ to toggle borders visibility
        property bool showTilesBorder: options.showContentTiles
        // The auto-generated notifier does not emit the new value, do it
        signal showTilesBordersValueChanged(bool value)
        onShowTilesBorderChanged: showTilesBordersValueChanged(showTilesBorder)
    }

    ZoomContext {
        Connections {
            onVisibleChanged: contentsync.zoomContextVisible = visible
        }
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
}

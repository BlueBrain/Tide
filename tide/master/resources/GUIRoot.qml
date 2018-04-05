import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    Rectangle {
        objectName: "Surface"
        id: surface
        color: Style.masterWindowBezelsColor
        width: surfaceWidth
        height: surfaceHeight

        // These properties are filled from the C++ Configuration class
        property int numberOfTilesX: 1
        property int numberOfTilesY: 1
        property int bezelWidth: 0
        property int bezelHeight: 0
        property int screenWidth: 640
        property int screenHeight: 480
        property int surfaceWidth: 640
        property int surfaceHeight: 480

        // Grid of screens that form the surface
        Grid {
            columns: surface.numberOfTilesX
            rows: surface.numberOfTilesY
            columnSpacing: surface.bezelWidth
            rowSpacing: surface.bezelHeight
            Repeater {
                model: parent.columns * parent.rows
                delegate: Rectangle {
                    // Generalized checker pattern computation
                    color: ((index%2 == 0) ^
                           (!(Math.floor(index / surface.numberOfTilesX)%2 == 0)
                           && (surface.numberOfTilesX%2 == 0))) ?
                           Style.masterWindowFirstCheckerColor :
                           Style.masterWindowSecondCheckerColor
                    width: surface.screenWidth
                    height: surface.screenHeight
                }
            }
        }

        // Center and scale the surface (+ its child DisplayGroup) into the view
        property bool higherAspectRatio: (view.width / view.height) >
                                         (surfaceWidth / surfaceHeight)

        property int marginWidth: higherAspectRatio ? 0 : (surfaceWidth
                                  * Style.masterWindowMarginFactor)
        property int marginHeight: higherAspectRatio ? (surfaceHeight
                                   * Style.masterWindowMarginFactor): 0

        property int totalWidth: surfaceWidth + marginWidth
        property int totalHeight: surfaceHeight + marginHeight

        property real scale: higherAspectRatio ? (view.height / totalHeight)
                                               : (view.width / totalWidth)

        property int offsetX: higherAspectRatio ? (view.width - surfaceWidth
                              * scale) / 2.0 : surfaceWidth
                              * Style.masterWindowMarginFactor * scale / 2.0
        property int offsetY: higherAspectRatio ? surfaceHeight
                              * Style.masterWindowMarginFactor* scale / 2.0
                              : (view.height - surfaceHeight * scale) / 2.0

        transform: [
            Scale {
                xScale: surface.scale
                yScale: surface.scale
            },
            Translate {
                x: surface.offsetX
                y: surface.offsetY
            }
        ]
    }
}

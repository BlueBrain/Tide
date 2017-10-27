import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Rectangle {
    objectName: "Wall"
    color: Style.masterWindowBezelsColor
    width: wallWidth
    height: wallHeight

    // These properties are input from the c++ Configuration class
    property int numberOfTilesX: 1
    property int numberOfTilesY: 1
    property int bezelWidth: 0
    property int bezelHeight: 0
    property int screenWidth: 640
    property int screenHeight: 480
    property int wallWidth: 640
    property int wallHeight: 480

    // Grid of screens that form the wall
    Grid {
        columns: numberOfTilesX
        rows: numberOfTilesY
        columnSpacing: bezelWidth
        rowSpacing: bezelHeight
        Repeater {
            model: numberOfTilesX * numberOfTilesY
            delegate: Rectangle {
                // Generalized checker pattern computation
                color: ((index%2 == 0) ^
                       (!(Math.floor(index / numberOfTilesX)%2 == 0)
                       && (numberOfTilesX%2 == 0))) ?
                       Style.masterWindowFirstCheckerColor :
                       Style.masterWindowSecondCheckerColor
                width: screenWidth
                height: screenHeight
            }
        }
    }

    // Center and scale the wall (+ its child DisplayGroup) into the Qml view
    property bool higherAspectRatio: (view.width / view.height) >
                                     (wallWidth / wallHeight)

    property int marginWidth: higherAspectRatio ? 0 : (wallWidth
                              * Style.masterWindowMarginFactor)
    property int marginHeight: higherAspectRatio ? (wallHeight
                               * Style.masterWindowMarginFactor): 0

    property int totalWidth: wallWidth + marginWidth
    property int totalHeight: wallHeight + marginHeight

    property real scale: higherAspectRatio ? (view.height / totalHeight)
                                           : (view.width / totalWidth)

    property int offsetX: higherAspectRatio ? ( view.width - wallWidth
                          * scale ) / 2.0 : wallWidth
                          * Style.masterWindowMarginFactor * scale / 2.0
    property int offsetY: higherAspectRatio ? wallHeight
                          * Style.masterWindowMarginFactor* scale / 2.0
                          : ( view.height - wallHeight * scale ) / 2.0

    transform: [
        Scale {
            xScale: scale
            yScale: scale
        },
        Translate {
            x : offsetX
            y : offsetY
        }
    ]
}

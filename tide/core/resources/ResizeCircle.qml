import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Item {
    property int border: modelData
    visible: isActive || contentwindow.border === ContentWindow.NOBORDER

    property bool isRight: border === ContentWindow.RIGHT
                           || border == ContentWindow.BOTTOM_RIGHT
                           || border === ContentWindow.TOP_RIGHT
    property bool isLeft: border === ContentWindow.LEFT
                          || border === ContentWindow.BOTTOM_LEFT
                          || border === ContentWindow.TOP_LEFT
    property bool isTop: border === ContentWindow.TOP
                         || border == ContentWindow.TOP_RIGHT
                         || border === ContentWindow.TOP_LEFT
    property bool isBottom: border === ContentWindow.BOTTOM
                            || border === ContentWindow.BOTTOM_LEFT
                            || border === ContentWindow.BOTTOM_RIGHT
    property bool isActive: contentwindow.border === border

    width: border === ContentWindow.TOP
           || border === ContentWindow.BOTTOM ? parent.width - Style.resizeCircleRadius
                                              : Style.resizeCircleRadius
    height: border === ContentWindow.RIGHT
            || border === ContentWindow.LEFT ? parent.height - Style.resizeCircleRadius
                                             : Style.resizeCircleRadius

    anchors.horizontalCenter: isRight ? parent.right
                                      : isLeft ? parent.left
                                               : parent.horizontalCenter
    anchors.verticalCenter: isTop ? parent.top
                                  : isBottom ? parent.bottom
                                             : parent.verticalCenter

    anchors.horizontalCenterOffset: isLeft ? Style.windowBorderWidth / 2.0
                                           : isRight ? -Style.windowBorderWidth / 2.0 : 0
    anchors.verticalCenterOffset: isTop ? Style.windowBorderWidth / 2.0
                                        : isBottom ? -Style.windowBorderWidth / 2.0 : 0

    Rectangle {
        id: controlCircle
        width: Style.resizeCircleRadius
        height: Style.resizeCircleRadius
        radius: Style.resizeCircleRadius
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        color: isActive ? Style.activeResizeCircleColor
                        : Style.inactiveResizeCircleColor
    }
}

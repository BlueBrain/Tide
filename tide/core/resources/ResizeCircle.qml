import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Item {
    property int handle: modelData

    property bool isRight: handle === Window.RIGHT
                           || handle == Window.BOTTOM_RIGHT
                           || handle === Window.TOP_RIGHT
    property bool isLeft: handle === Window.LEFT
                          || handle === Window.BOTTOM_LEFT
                          || handle === Window.TOP_LEFT
    property bool isTop: handle === Window.TOP
                         || handle == Window.TOP_RIGHT
                         || handle === Window.TOP_LEFT
    property bool isBottom: handle === Window.BOTTOM
                            || handle === Window.BOTTOM_LEFT
                            || handle === Window.BOTTOM_RIGHT

    property bool isActive: window.activeHandle === handle

    visible: isActive || window.activeHandle === Window.NOHANDLE

    width: Style.resizeCircleRadius
    height: Style.resizeCircleRadius

    anchors.horizontalCenter: isRight ? parent.right
                                      : isLeft ? parent.left
                                               : parent.horizontalCenter
    anchors.verticalCenter: isTop ? parent.top
                                  : isBottom ? parent.bottom
                                             : parent.verticalCenter

    Rectangle {
        id: controlCircle
        width: Style.resizeCircleRadius
        height: Style.resizeCircleRadius
        radius: Style.resizeCircleRadius
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        color: isActive ?
                   window.resizePolicy === Window.ADJUST_CONTENT ?
                       Style.resizeCircleFreeResizeColor :
                       Style.resizeCircleActiveColor :
                       Style.resizeCircleInactiveColor
    }
}

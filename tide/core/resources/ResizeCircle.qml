import QtQuick 2.0
import Tide 1.0
import "style.js" as Style

Item {
    property int handle: modelData

    property bool isRight: handle === ContentWindow.RIGHT
                           || handle == ContentWindow.BOTTOM_RIGHT
                           || handle === ContentWindow.TOP_RIGHT
    property bool isLeft: handle === ContentWindow.LEFT
                          || handle === ContentWindow.BOTTOM_LEFT
                          || handle === ContentWindow.TOP_LEFT
    property bool isTop: handle === ContentWindow.TOP
                         || handle == ContentWindow.TOP_RIGHT
                         || handle === ContentWindow.TOP_LEFT
    property bool isBottom: handle === ContentWindow.BOTTOM
                            || handle === ContentWindow.BOTTOM_LEFT
                            || handle === ContentWindow.BOTTOM_RIGHT

    property bool isActive: contentwindow.activeHandle === handle

    visible: isActive || contentwindow.activeHandle === ContentWindow.NOHANDLE

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
                   contentwindow.resizePolicy === ContentWindow.ADJUST_CONTENT ?
                       Style.resizeCircleFreeResizeColor :
                       Style.resizeCircleActiveColor :
                       Style.resizeCircleInactiveColor
    }
}

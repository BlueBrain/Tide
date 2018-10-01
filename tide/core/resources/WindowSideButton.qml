import QtQuick 2.0
import Tide 1.0

SideButton {
    property Component handleDelegate
    property Component pageDelegate

    property bool showHandle: window.content.captureInteraction
                              && (!window.focused && !window.fullscreen)
    property bool showPageAction: window.content.page !== undefined
                                  && flipRight ? window.content.page < window.content.pageCount
                                                 - 1 : window.content.page > 0

    color: parent.border.color
    icon: showPageAction ? "qrc:/img/triangle.svg" : "qrc:/img/dot.svg"
    delegate: showPageAction ? pageDelegate : handleDelegate
    delegateOverflow: parent.border.width
    visible: !window.isPanel && (showHandle || showPageAction)
}

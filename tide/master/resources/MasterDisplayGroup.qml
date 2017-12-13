import QtQuick 2.0
import Tide 1.0
import TideMaster 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

DisplayGroup {
    id: displaygroupitem
    showFocusContext: false

    MultitouchArea {
        anchors.fill: parent
        referenceItem: displaygroupitem

        property bool blockTap: true
        onTouchStarted: blockTap = false
        onTapAndHold: {
            blockTap = true
        }
        onTap: {
            if (!blockTap)
                groupcontroller.deselectAll()
        }
    }

    MultitouchArea {
        id: touchBarrier
        anchors.fill: parent
        visible: displaygroupitem.state !== ""
        z: displaygroupitem.focusContextZorder
        function goToDesktop() {
            if (displaygroup.hasFullscreenWindows)
                groupcontroller.exitFullscreen()
            else if (displaygroup.hasFocusedWindows)
                groupcontroller.unfocusAll()
            else if (displaygroup.hasVisiblePanels)
                groupcontroller.hidePanels()
        }
        onTap: goToDesktop()
    }
}

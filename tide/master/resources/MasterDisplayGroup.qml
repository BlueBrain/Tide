import QtQuick 2.0
import Tide 1.0
import TideMaster 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

DisplayGroup {
    id: dispGroup
    showFocusContext: false

    signal launcherControlPressed()
    signal settingsControlsPressed()

    MultitouchArea {
        anchors.fill: parent
        referenceItem: dispGroup
        z: controlPanel.z - 1

        property bool blockTap: true
        onTouchStarted: blockTap = false
        onTapAndHold: {
            view.backgroundTapAndHold(pos)
            blockTap = true;
        }
        onTap: {
            if( !blockTap )
                view.backgroundTap(pos)
        }
    }

    controlPanel.buttonDelegate: Component {
        ControlPanelDelegate {
            id: touchControlPanel
            MultitouchArea {
                anchors.fill: parent
                referenceItem: dispGroup

                onTap: {
                    var action = touchControlPanel.ListView.view.model.get(index).action
                    var absPos = mapToItem(dispGroup, controlPanel.width
                                 + Style.panelsLeftOffset, 0)
                    var position = Qt.point(absPos.x, absPos.y)
                    cppcontrolpanel.processAction(action, position)
                }
            }
        }
    }

    sideControl.buttonDelegate: MultitouchArea {
        onTap: {
            if(buttonIndex == 0)
                launcherControlPressed();
            else if(buttonIndex == 1)
                settingsControlsPressed();
        }
    }
}

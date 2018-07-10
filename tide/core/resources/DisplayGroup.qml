import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: displaygroupitem

    width: displaygroup.width
    height: displaygroup.height

    Rectangle {
        id: focuscontext
        objectName: "focuscontext"
        anchors.fill: parent
        color: Style.focusContextColor
        opacity: 0
        visible: opacity > 0
        z: Style.focusBackgroundZorder
        Behavior on opacity {
            NumberAnimation {
                target: focuscontext
                property: "opacity"
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
        ClickArea {
            anchors.fill: parent
            function goToDesktop() {
                if (displaygroup.hasFullscreenWindows)
                    groupcontroller.exitFullscreen()
                else if (displaygroup.hasFocusedWindows)
                    groupcontroller.unfocusAll()
                else if (displaygroup.hasVisiblePanels)
                    groupcontroller.hidePanels()
            }
            onClicked: goToDesktop()
        }
    }

    states: [
        State {
            name: "fullscreen"
            when: displaygroup.hasFullscreenWindows
            PropertyChanges {
                target: focuscontext
                opacity: Style.focusContextFullscreenOpacity
                z: Style.fullscreenBackgroundZorder
            }
        },
        State {
            name: "focused"
            when: displaygroup.hasFocusedWindows
            PropertyChanges {
                target: focuscontext
                opacity: Style.focusContextOpacity
            }
        },
        State {
            name: "panels"
            when: displaygroup.hasVisiblePanels
            PropertyChanges {
                target: focuscontext
                opacity: Style.focusContextPanelsOpacity
                z: Style.panelsBackgroundZorder
            }
        }
    ]
}

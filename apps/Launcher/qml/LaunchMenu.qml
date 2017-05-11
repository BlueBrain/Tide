// Copyright (c) 2014-2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import "style.js" as Style

Rectangle {
    id: menu

    signal clearSession()
    signal showFilesPanel()
    signal showSessionsPanel()
    signal showSaveSessionPanel()
    signal showOptionsPanel()
    signal showDemosPanel()
    signal startWebbrowser()
    signal startWhiteboard()
    signal poweroffScreens()

    color: Style.menuColor

    property bool demoItemVisible: false
    property bool poweroffItemVisible: false

    property real textSize: Style.menuRelTextSize * menu.width

    ListView {
        id: menuListView
        anchors.fill: parent

        model: menuList
        delegate: menuButtonDelegate

        highlight: highlightBar
        highlightFollowsCurrentItem: false
        currentIndex: -1

        boundsBehavior: Flickable.StopAtBounds
    }

    ListModel {
        id: menuList
        // ListElement can't store function pointers so use eval(model.action)
        ListElement {
            action: "showFilesPanel"
            name: "Open"
            image: "qrc:/images/file.svg"
            category: "Content"
            isPanel: true
        }
        ListElement {
            action: "clearSession"
            name: "Close all"
            image: "qrc:///img/close.svg"
            category: "Content"
            isPanel: false
        }
        ListElement {
            action: "showSessionsPanel"
            name: "Load"
            image: "qrc:/images/folder.svg"
            category: "Session"
            isPanel: true
        }
        ListElement {
            action: "showSaveSessionPanel"
            name: "Save"
            image: "qrc:/images/star.svg"
            category: "Session"
            isPanel: true
        }
        ListElement {
            action: "showOptionsPanel"
            name: "Settings"
            image: "qrc:/images/settings.svg"
            category: "Options"
            isPanel: true
        }
        ListElement {
            action: "startWhiteboard"
            name: "Whiteboard"
            image: "qrc:/images/whiteboard.svg"
            category: "Applications"
            isPanel: false
        }
        ListElement {
            action: "startWebbrowser"
            name: "Webbrowser"
            image: "qrc:/images/cloud.svg"
            category: "Applications"
            isPanel: false
        }
        ListElement {
            action: "showDemosPanel"
            name: "Visualization"
            image: "qrc:/images/book.svg"
            category: "Demos"
            isPanel: true
        }
        ListElement {
            action: "poweroffScreens"
            name: "Power off"
            image: "qrc:/images/poweroff.svg"
            category: "Control"
            isPanel: false
        }
    }

    Component {
        id: highlightBar
        Rectangle {
            color: Style.menuHighlightColor
            opacity: Style.menuHighlightOpactiy
            width: menuListView.width
            height: menuListView.currentItem.height
            y: menuListView.currentItem.y
            Behavior on y { SmoothedAnimation { velocity: 1000 } }
        }
    }

    Component {
        id: menuButtonDelegate
        Item {
            id: button
            visible: model.category === "Control" ? menu.poweroffItemVisible :
                     model.category === "Demos" ? menu.demoItemVisible : true
            height: visible === true ? image.height + caption.height +
                                       0.15 * menuListView.width : 0
            width: menuListView.width * 0.8
            anchors.horizontalCenter: parent.horizontalCenter
            Column {
                spacing: 0.05 * menuListView.width
                anchors.centerIn: parent
                Image {
                    id: image
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: button.width * 0.7
                    height: width
                    source: model.image
                    fillMode: Image.PreserveAspectFit
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if(model.isPanel)
                                menuListView.currentIndex = index;
                            eval(model.action)();
                        }
                    }
                }
                Text {
                    id: caption
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: model.name
                    font.bold: true
                    font.pixelSize: menu.textSize
                    color: Style.menuTextColor
                }
            }
        }
    }
}

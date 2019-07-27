// Copyright (c) 2014-2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
import "style.js" as Style

Rectangle {
    id: menu

    signal clearSession
    signal showFilesPanel
    signal showSessionsPanel
    signal showSearchPanel
    signal showSaveSessionPanel
    signal showOptionsPanel
    signal showDemosPanel
    signal startWebbrowser
    signal startWhiteboard
    signal poweroffScreens

    color: Style.menuColor

    property bool demoItemVisible: false
    property bool poweroffItemVisible: false

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
            name: "Add"
            image: "qrc:/launcher/images/add.svg"
            category: "Content"
            isPanel: true
        }
        ListElement {
            action: "clearSession"
            name: "Close all"
            image: "qrc:/launcher/images/close.svg"
            category: "Content"
            isPanel: false
        }
        ListElement {
            action: "showSessionsPanel"
            name: "Open"
            image: "qrc:/launcher/images/open.svg"
            category: "Session"
            isPanel: true
        }
        ListElement {
            action: "showSaveSessionPanel"
            name: "Save"
            image: "qrc:/launcher/images/save.svg"
            category: "Session"
            isPanel: true
        }
        ListElement {
            action: "showSearchPanel"
            name: "Search"
            image: "qrc:/launcher/images/search.svg"
            category: "Content"
            isPanel: true
        }
        ListElement {
            action: "showOptionsPanel"
            name: "Settings"
            image: "qrc:/launcher/images/settings.svg"
            category: "Options"
            isPanel: true
        }
        ListElement {
            action: "startWhiteboard"
            name: "Whiteboard"
            image: "qrc:/launcher/images/whiteboard.svg"
            category: "Applications"
            isPanel: false
        }
        ListElement {
            action: "startWebbrowser"
            name: "Web browser"
            image: "qrc:/launcher/images/webbrowser.svg"
            category: "Applications"
            isPanel: false
        }
        ListElement {
            action: "showDemosPanel"
            name: "Visualization"
            image: "qrc:/launcher/images/visualisation.svg"
            category: "Demos"
            isPanel: true
        }
        ListElement {
            action: "poweroffScreens"
            name: "Power off"
            image: "qrc:/launcher/images/poweroff.svg"
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
            Behavior on y {
                SmoothedAnimation {
                    velocity: 1000
                }
            }
        }
    }

    Component {
        id: menuButtonDelegate
        Item {
            id: button
            visible: model.category === "Control" ? menu.poweroffItemVisible : model.category
                                                    === "Demos" ? menu.demoItemVisible : true
            height: visible === true ? image.height + caption.height + 0.15 * menuListView.width : 0
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
                            if (model.isPanel)
                                menuListView.currentIndex = index
                            eval(model.action)()
                        }
                    }
                }
                Text {
                    id: caption
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: model.name
                    font.pixelSize: smallTextPixelSize
                    color: Style.menuTextColor
                }
            }
        }
    }
}

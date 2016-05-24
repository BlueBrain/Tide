// Copyright (c) 2014-2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import "style.js" as Style

Rectangle {
    id: menu

    signal clearSession()
    signal showFilesPanel()
    signal showSessionsPanel()
    signal showDemosPanel()
    signal startWebbrowser()

    color: Style.menuColor

    property real textSize: Style.menuRelTextSize * menu.width

    ListView {
        id: appListView
        anchors.fill: parent
        model: appList
        delegate: appButtonDelegate
        spacing: 0.1 * width

        highlight: highlightBar
        highlightFollowsCurrentItem: false
        currentIndex: -1

        section.delegate: sectionHeading
        section.property: "appCategory"
        section.criteria: ViewSection.FullString

        boundsBehavior: Flickable.StopAtBounds
    }

    ListModel {
        id: appList
        // ListElement can't store function pointers so use eval(appAction)
        ListElement {
            appAction: "clearSession"
            appName: "Close all contents"
            appImage: "qrc:/images/clear-icon.png"
            appCategory: "Actions"
        }
        ListElement {
            appAction: "showFilesPanel"
            appName: "Open content"
            appImage: "qrc:/images/file.svg"
            appCategory: "Actions"
        }
        ListElement {
            appAction: "showSessionsPanel"
            appName: "Load session"
            appImage: "qrc:/images/folder.svg"
            appCategory: "Actions"
        }
        ListElement {
            appAction: "startWebbrowser"
            appName: "Webbrowser"
            appImage: "qrc:/images/browser-icon.png"
            appCategory: "Applications"
        }
        ListElement {
            appAction: "showDemosPanel"
            appName: "Visualization"
            appImage: "qrc:/images/book.svg"
            appCategory: "Demos"
        }
    }

    Component {
        id: highlightBar
        Rectangle {
            color: Style.menuHighlightColor
            opacity: Style.menuHighlightOpactiy
            width: appListView.width
            height: appListView.currentItem.height
            y: appListView.currentItem.y
            Behavior on y { SmoothedAnimation { velocity: 1000 } }
        }
    }

    Component {
        id: sectionHeading
        Rectangle {
            width: appListView.width
            height: 1.3 * sectionHeadingText.height
            color: Style.menuSectionHeadingColor
            Text {
                id: sectionHeadingText
                text: section
                font.bold: true
                font.pixelSize: menu.textSize
                anchors.centerIn: parent
            }
        }
    }

    Component {
        id: appButtonDelegate
        Item {
            id: button
            height: childrenRect.height
            width: appListView.width * 0.8
            anchors.horizontalCenter: parent.horizontalCenter
            Column {
                spacing: 0.05 * width
                Image {
                    id: image
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: button.width
                    height: button.width
                    source: appImage
                    fillMode: Image.PreserveAspectFit
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if(appCategory === "Actions" || appCategory === "Demos")
                                appListView.currentIndex = index;
                            eval(appAction)();
                        }
                    }
                }
                Text {
                    id: caption
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: appName
                    font.bold: true
                    font.pixelSize: menu.textSize
                    color: Style.menuTextColor
                }
            }
        }
    }
}

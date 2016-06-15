// Copyright (c) 2015-2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
//                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>

import QtQuick 2.4
import Qt.labs.folderlistmodel 2.0
import "style.js" as Style

Rectangle {
    id: fileBrowser

    color: Style.fileBrowserBackgroundColor

    property string rootfolder: ""
    property alias nameFilters: folders.nameFilters // list<string>
    property alias currentFolder: folders.folder
    property alias titleBarHeight: titleBar.height
    property int itemSize: height * Style.fileBrowserItemSizeRel

    signal itemSelected(string file)

    function selectFile(file) {
        if (file !== "")
            itemSelected(file)
    }

    function goDown(path) {
        folders.folder = path;
    }

    function goUp() {
        var path = folders.parentFolder;
        if (path.toString().length === 0 || path.toString() === 'file:')
            return;
        folders.folder = path;
    }

    FolderListModel {
        id: folders
        rootFolder: "file:" + fileBrowser.rootfolder
        folder: "file:" + fileBrowser.rootfolder
        showDirsFirst: true
        showDotAndDotDot: false
        // Extra property needed to silence Qml warning:
        // "[...] depends on non-NOTIFYable properties: [...] rootFolder"
        readonly property string constRootFolder: "file://" + fileBrowser.rootfolder
    }

    Component {
        id: fileDelegate
        Rectangle {
            id: wrapper
            width: itemSize
            height: itemSize
            color: "transparent"

            Rectangle {
                id: placeholder
                anchors.fill: parent
                anchors.margins: 0.05 * wrapper.width
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Style.placeholderTopColor }
                    GradientStop { position: 1.0; color: Style.placeholderBottomColor }
                }
                visible: thumbnail.status !== Image.Ready
            }
            Image {
                id: thumbnail
                anchors.fill: placeholder
                fillMode: Image.PreserveAspectFit
                cache: false
                source: "image://thumbnail/" + filePath
            }

            Text {
                id: nameText
                text: fileName.length > 15 ? fileName.substr(0,15) + "..." : fileName
                font.bold: true
                anchors.top: parent.bottom
                anchors.horizontalCenter: wrapper.horizontalCenter
                color: Style.fileBrowserTextColor
                font.pixelSize: 0.1 * wrapper.height
            }

            MouseArea {
                id: mouseRegion
                anchors.fill: parent
                onPressed: wrapper.GridView.view.currentIndex = index
                onClicked: clickAnimation.start()
            }

            function openCurrentItem() {
                var path = "file://";
                if (filePath.length > 2 && filePath[1] === ':')
                    path += '/';
                path += filePath;

                if (folders.isFolder(index))
                    fileBrowser.goDown(path);
                else
                    fileBrowser.selectFile(path.replace("file://", ""));
            }

            ColorAnimation on color {
                id: clickAnimation
                running: false
                from: Style.fileBrowserBlinkColor
                to: "transparent"
                onStopped: openCurrentItem()
            }
        }
    }

    GridView {
        id: view
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: itemSize * Style.mainPanelRelMargin

        cellWidth: itemSize * 1.33
        cellHeight: cellWidth
        model: folders
        delegate: fileDelegate
        focus: true
    }

    Rectangle {
        id: titleBar
        width: parent.width
        height: parent.height * Style.titleBarRelHeight
        color: Style.fileBrowserTitleBarColor
        anchors.top: parent.top

        Row {
            anchors.fill: parent
            anchors.leftMargin: 10
            spacing: 10

            move: Transition {
                NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad }
            }

            Rectangle {
                id: upButton
                width: 0.75 * titleBar.height
                height: width
                anchors.verticalCenter: parent.verticalCenter
                color: "transparent"
                Image {
                    anchors.fill: parent
                    source: "qrc:/images/left.svg"
                }
                MouseArea {
                    id: upRegion
                    anchors.fill: parent
                    onClicked: fileBrowser.goUp()
                }
                states: [
                    State {
                        name: "pressed"
                        when: upRegion.pressed
                        PropertyChanges {
                            target: upButton
                            color: Style.fileBrowserBlinkColor
                        }
                    }
                ]
            }
            Text {
                id: titleText
                height: titleBar.height
                color: Style.fileBrowserTextColor
                font.pixelSize: 0.5 * height
                verticalAlignment: Text.AlignVCenter
                text: folders.folder.toString().replace(folders.constRootFolder+"/", "")
                Behavior on color { PropertyAnimation{ }}
            }
            states: [
                State {
                    name: "showRootPath"
                    when: folders.folder.toString() === folders.constRootFolder
                    PropertyChanges {
                        target: titleText
                        color: Style.fileBrowserDiscreteTextColor
                        text: folders.constRootFolder.replace("file://", "")
                        anchors.leftMargin: titleBar.height * 0.5
                    }
                    PropertyChanges {
                        target: upButton
                        visible: false
                    }
                }
            ]
        }
    }
}

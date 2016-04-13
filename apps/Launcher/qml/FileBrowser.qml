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
    property int buttonHeight: height / 10
    property int itemSize: height / 5

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
    }

    SystemPalette {
        id: palette
    }

    Component {
        id: fileDelegate
        Rectangle {
            id: wrapper
            width: folderImage.width
            height: folderImage.height
            color: "transparent"

            Item {
                id: folderImage
                width: itemSize
                height: itemSize

                Image {
                    id: folderPicture
                    source: "image://thumbnail/" + filePath
                    width: itemSize * 0.9
                    height: itemSize * 0.9
                    fillMode: Image.PreserveAspectFit
                    anchors.horizontalCenter: folderImage.horizontalCenter
                    anchors.verticalCenter: folderImage.verticalCenter
                }
            }

            Text {
                id: nameText
                text: fileName.length > 15 ? fileName.substr(0,15) + "..." : fileName
                font.bold: true
                anchors.top: parent.bottom
                anchors.horizontalCenter: folderImage.horizontalCenter
                color: Style.fileBrowserTextColor
                font.pixelSize: 0.1 * itemSize
            }

            MouseArea {
                id: mouseRegion
                anchors.fill: parent
                onPressed: wrapper.GridView.view.currentIndex = index
                onClicked: {
                    wrapper.color = Style.fileBrowserSelectionColor
                    animateColor.start()
                    var path = "file://";
                    if (filePath.length > 2 && filePath[1] === ':')
                        path += '/';

                    path += filePath;

                    if (folders.isFolder(index))
                        fileBrowser.goDown(path);
                    else
                        fileBrowser.selectFile(path.replace("file://", ""))
                }
            }
            PropertyAnimation {
                id: animateColor
                to: "transparent"
                target: wrapper
                properties: "color"
                easing.type: Easing.Linear
                duration: 1000
            }
        }
    }

    GridView {
        id: view
        anchors.top: titleBar.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width - 0.5 * itemSize
        cellWidth: itemSize * 1.2
        cellHeight: itemSize * 1.2
        model: folders
        delegate: fileDelegate
        focus: true
    }

    Rectangle {
        id: titleBar
        width: parent.width
        height: buttonHeight + 10
        anchors.top: parent.top
        color: Style.fileBrowserBackgroundColor

        Rectangle {
            width: parent.width
            height: buttonHeight
            color: Style.fileBrowserTitleBarColor
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            radius: buttonHeight / 15

            Row {
                anchors.fill: parent
                anchors.leftMargin: 10
                spacing: 10

                move: Transition {
                    NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad }
                }

                Rectangle {
                    id: upButton
                    width: 0.75 * buttonHeight
                    height: 0.75 * buttonHeight
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
                                color: palette.highlight
                            }
                        }
                    ]
                }
                Text {
                    id: titleText
                    height: buttonHeight
                    color: Style.fileBrowserTextColor
                    font.pixelSize: 0.5 * buttonHeight
                    verticalAlignment: Text.AlignVCenter
                    text: folders.folder.toString().replace(folders.rootFolder.toString()+"/", "")
                    Behavior on color { PropertyAnimation{ }}
                }
                states: [
                    State {
                        name: "showRootPath"
                        when: folders.folder === folders.rootFolder
                        PropertyChanges {
                            target: titleText
                            color: Style.fileBrowserDiscreteTextColor
                            text: folders.rootFolder.toString().replace("file://", "")
                            anchors.leftMargin: buttonHeight * 0.5
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
}

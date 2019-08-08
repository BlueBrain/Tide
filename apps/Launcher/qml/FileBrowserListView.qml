// Copyright (c) 2015-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQml.Models 2.1

import Launcher 1.0
import "style.js" as Style

Item {
    id: listviewTable

    signal openItem(string filePath, bool fileIsDir)
    signal folderAction(int filesInDir, string filePath)

    property variant foldersModel
    property int listItemSize


    ListView {
        id: listview

        anchors.top: sortBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: scrollBar.visible ? 0.5 * scrollBar.width
                                                 + anchors.leftMargin : anchors.leftMargin
        anchors.leftMargin: listItemSize * Style.mainPanelRelMargin
        spacing: listItemSize * Style.fileBrowserListItemSpacing
        clip: true

        model: DelegateModel {
            id: listDelegateModel
            model: foldersModel
            rootIndex: foldersModel.getRootIndex()

            Connections {
                target: foldersModel
                onRootFolderChanged: rootIndex = foldersModel.getRootIndex()
            }

            // Mysterious hack to prevent rootIndex from being reset
            // asynchronously when using a QFileSystemModel:
            // http://lists.qt-project.org/pipermail/interest/2015-August/018369.html
            onRootIndexChanged: {
                rootIndex = foldersModel.getRootIndex()
            }
            delegate: FileBrowserListItem {
                width: parent.width
                height: listItemSize * Style.fileBrowserListItemRelSize
                onClicked: listviewTable.openItem(filePath, fileIsDir)
                onPressAndHold: {
                    if (fileIsDir && filesInDir > 0) {
                        listviewTable.folderAction(filesInDir, filePath)
                    }
                }
            }
        }

        section.property: foldersModel.sortCategory === FolderModel.Name ? "fileName" : ""
        section.criteria: ViewSection.FirstCharacter
        section.delegate: Item {
            width: parent.width
            height: 1.5 * smallTextPixelSize
            Text {
                text: section
                font.capitalization: Font.Capitalize
                font.pixelSize: smallTextPixelSize
            }
        }
    }

    ScrollBar {
        id: scrollBar
        anchors.top: listview.top
        anchors.right: parent.right
        flickable: listview
    }

    Rectangle {
        id: sortBar
        height: titleBar.height / 2
        anchors.left: listview.left
        anchors.right: listview.right
        color: Style.fileBrowserBackgroundColor

        Text {
            id: nameText
            property bool active: foldersModel.sortCategory === FolderModel.Name
            text: (active ? foldersModel.sortIcon + " " : "") + "name"
            horizontalAlignment: Text.AlignHCenter
            anchors.left: sortBar.left
            anchors.right: sizeText.left

            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: smallTextPixelSize
            color: Style.fileBrowserTextColor

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (parent.active)
                        foldersModel.toggleSortOrder()
                    else
                        foldersModel.sortCategory = FolderModel.Name
                }
            }
        }
        Text {
            id: sizeText
            property bool active: foldersModel.sortCategory === FolderModel.Size
            text: (active ? foldersModel.sortIcon + " " : "") + "size"
            anchors.right: modifiedText.left
            width: textColumnSize
            horizontalAlignment: Text.AlignRight

            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: smallTextPixelSize
            color: Style.fileBrowserTextColor

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (parent.active)
                        foldersModel.toggleSortOrder()
                    else
                        foldersModel.sortCategory = FolderModel.Size
                }
            }
        }
        Text {
            id: modifiedText
            property bool active: foldersModel.sortCategory === FolderModel.Date
            text: (active ? foldersModel.sortIcon + " " : "") + "modified"
            anchors.right: sortBar.right
            width: textColumnSize
            horizontalAlignment: Text.AlignRight

            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: smallTextPixelSize
            color: Style.fileBrowserTextColor

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (parent.active)
                        foldersModel.toggleSortOrder()
                    else
                        foldersModel.sortCategory = FolderModel.Date
                }
            }
        }
    }
}

// Copyright (c) 2015-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
//                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQml.Models 2.1

import Launcher 1.0
import "style.js" as Style

Rectangle {
    id: fileBrowser

    color: Style.fileBrowserBackgroundColor

    signal itemSelected(string file)

    // options
    property string rootfolder: ""
    property alias nameFilters: folders.nameFilters // list<string>
    property alias listViewMode: listViewButton.checked
    property alias hideExtensions: folders.hideExtensions
    property bool allowOpeningFolder: false
    property bool gridViewSortByDate: false

    // infos
    property alias currentFolder: folders.rootFolder
    property alias titleBarHeight: titleBar.height

    // internal
    property int itemSize: height * Style.fileBrowserItemSizeRel

    Connections {
        target: deflectgestures
        onSwipeLeft: fileBrowser.goUp()
    }

    function selectFile(file) {
        if (file !== "")
            itemSelected(file)
    }

    function goDown(path) {
        folders.rootFolder = path
    }

    function goUp() {
        if (folders.rootFolder !== fileBrowser.rootfolder)
            folders.rootFolder = folders.getParentFolder()
    }

    function openItem(filePath, fileIsDir) {
        if (fileIsDir)
            goDown(filePath)
        else
            selectFile(filePath)
    }

    function humanReadableFileSize(bytes, si) {
        var thresh = si ? 1000 : 1024
        if (Math.abs(bytes) < thresh)
            return bytes + ' bytes'
        var units = si ? ['kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'] : ['KiB', 'MiB', 'GiB', 'TiB', 'PiB', 'EiB', 'ZiB', 'YiB']
        var u = -1
        do {
            bytes /= thresh
            ++u
        } while (Math.abs(bytes) >= thresh && u < units.length - 1)
        return bytes.toFixed(1) + ' ' + units[u]
    }

    function humanReadableModificationDate(date) {
        var now = new Date()
        if (now - date < 1000 * 3600 * 24)
            // less than one day
            return date.toLocaleTimeString()
        if (date.getFullYear() == now.getFullYear())
            return Qt.formatDate(date, "dd MMMM")
        return Qt.formatDate(date, "dd MMM yyyy")
    }

    FolderModel {
        id: folders
        rootFolder: fileBrowser.rootfolder
        property string sortIcon: sortOrder == FolderModel.Ascending ? "▲" : "▼"
        function setGridSortOrder() {
            if (fileBrowser.gridViewSortByDate) {
                sortCategory = FolderModel.Date
                sortOrder = FolderModel.Descending
            } else {
                sortCategory = FolderModel.Name
                sortOrder = FolderModel.Ascending
            }
        }
    }

    FileBrowserGridView {
        visible: gridViewButton.checked

        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: itemSize * Style.mainPanelRelMargin

        foldersModel: folders
        sectionModel: gridViewSortByDate ? folders.years : undefined
        gridItemSize: itemSize

        onOpenItem: fileBrowser.openItem(filePath, fileIsDir)
        onFolderAction: {
            if (fileBrowser.allowOpeningFolder) {
                curtain.showDialog(filesInDir, filePath)
            }
        }
    }

    FileBrowserListView {
        visible: listViewButton.checked

        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        foldersModel: folders
        listItemSize: itemSize

        onOpenItem: fileBrowser.openItem(filePath, fileIsDir)
        onFolderAction: {
            if (fileBrowser.allowOpeningFolder) {
                curtain.showDialog(filesInDir, filePath)
            }
        }
    }

    Rectangle {
        id: titleBar
        width: parent.width
        height: parent.height * Style.titleBarRelHeight
        color: Style.fileBrowserTitleBarColor
        anchors.top: parent.top

        property real spacing: Style.fileBrowserTitleBarSpacing * height

        Rectangle {
            id: upButton
            anchors.top: titleBar.top
            anchors.bottom: titleBar.bottom
            anchors.left: titleBar.left
            anchors.margins: titleBar.spacing
            width: height
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
            anchors.left: upButton.right
            anchors.right: viewButtons.left
            anchors.margins: titleBar.spacing
            color: Style.fileBrowserTextColor
            font.pixelSize: standardTextPixelSize
            verticalAlignment: Text.AlignVCenter
            text: folders.rootFolder.replace(fileBrowser.rootfolder + "/", "")
            elide: Text.ElideMiddle
            Behavior on color {
                PropertyAnimation {
                }
            }
        }
        Row {
            id: viewButtons
            anchors.top: titleBar.top
            anchors.bottom: titleBar.bottom
            anchors.right: titleBar.right
            anchors.margins: titleBar.spacing
            spacing: titleBar.spacing
            ExclusiveGroup {
                id: viewButtonsGroup
            }
            FileBrowserViewButton {
                id: gridViewButton
                exclusiveGroup: viewButtonsGroup
                checked: !listViewMode
                height: parent.height
                icon: "qrc:/images/gridview.svg"
                onCheckedChanged: {
                    if (checked) {
                        folders.setGridSortOrder()
                    }
                }
            }
            FileBrowserViewButton {
                id: listViewButton
                exclusiveGroup: viewButtonsGroup
                height: parent.height
                icon: "qrc:/images/listview.svg"
            }
        }
        states: [
            State {
                name: "showRootPath"
                when: folders.rootFolder === fileBrowser.rootfolder
                PropertyChanges {
                    target: titleText
                    color: Style.fileBrowserDiscreteTextColor
                    text: folders.rootFolder.replace("file://", "")
                }
                AnchorChanges {
                    target: titleText
                    anchors.left: titleBar.left
                }
                PropertyChanges {
                    target: upButton
                    visible: false
                }
            }
        ]
        transitions: Transition {
            // smoothly reanchor titleText and move into new position
            AnchorAnimation {
                easing.type: Easing.OutQuad
            }
        }
    }

    Curtain {
        id: curtain

        function showDialog(itemCount, folderToOpen) {
            dialog.itemCount = itemCount
            dialog.folderToOpen = folderToOpen
            open = true
        }

        CurtainDialog {
            id: dialog
            property int itemCount: 0
            property string folderToOpen: ""

            textInfo: "Do you want to open all the supported files in this folder?"
            textAccept: "open " + itemCount + " item" + (itemCount > 1 ? "s" : "")

            onAccepted: {
                fileBrowser.itemSelected(folderToOpen)
                curtain.open = false
            }
            onRejected: {
                curtain.open = false
            }
        }
    }
}

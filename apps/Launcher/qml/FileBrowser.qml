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
    property bool allowOpeningFolder: false

    // infos
    property alias currentFolder: folders.rootFolder
    property alias titleBarHeight: titleBar.height

    // internal
    property int itemSize: height * Style.fileBrowserItemSizeRel
    property real textPixelSize: itemSize * Style.fileBrowserTextSizeRelToItem
    property real textColumnSize: textPixelSize * 10

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

    function humanReadableModificationDate(modDate) {
        var now = new Date()
        var date = new Date(modDate)
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
        function toggleSortOrder() {
            sortOrder = (sortOrder
                         == FolderModel.Ascending) ? FolderModel.Descending : FolderModel.Ascending
        }
        property string sortIcon: sortOrder == FolderModel.Ascending ? "▲" : "▼"
    }

    Component {
        id: gridFileDelegate
        Rectangle {
            id: wrapper
            width: itemSize
            height: itemSize
            color: "transparent"

            FileBrowserImage {
                anchors.fill: parent
                anchors.margins: 0.05 * wrapper.width
                file: filePath
            }

            Text {
                id: nameText
                text: fileName
                anchors.top: parent.bottom
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 2
                elide: Text.ElideRight // ElideMiddle does not work with wrapped text
                color: Style.fileBrowserTextColor
                font.pixelSize: textPixelSize
            }

            MouseArea {
                id: mouseRegion
                anchors.fill: parent
                onPressed: wrapper.GridView.view.currentIndex = index
                onClicked: clickAnimation.start()
                onPressAndHold: {
                    if (fileBrowser.allowOpeningFolder && fileIsDir
                            && filesInDir > 0) {
                        curtain.showDialog(filesInDir, filePath)
                    }
                }
            }

            ColorAnimation on color {
                id: clickAnimation
                running: false
                from: Style.fileBrowserBlinkColor
                to: "transparent"
                onStopped: fileBrowser.openItem(filePath, fileIsDir)
            }
        }
    }

    GridView {
        id: gridview
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: itemSize * Style.mainPanelRelMargin
        visible: gridViewButton.checked

        cellWidth: itemSize * 1.33
        cellHeight: cellWidth
        model: DelegateModel {
            id: gridDelegateModel
            model: folders
            rootIndex: folders.getPathIndex(folders.rootFolder)

            Connections {
                target: folders
                onRootFolderChanged: rootIndex = folders.getPathIndex(
                                         folders.rootFolder)
            }

            // Mysterious hack to prevent rootIndex from being reset
            // asynchronously when using a QFileSystemModel:
            // http://lists.qt-project.org/pipermail/interest/2015-August/018369.html
            onRootIndexChanged: {
                rootIndex = folders.getPathIndex(folders.rootFolder)
            }
            delegate: gridFileDelegate
        }

        ScrollBar {
        }
    }

    Component {
        id: listFileDelegate
        Rectangle {
            id: wrapper
            width: parent.width
            height: itemSize * Style.fileBrowserListItemRelSize
            color: "transparent"

            FileBrowserImage {
                id: fileimage
                height: parent.height
                width: height
                file: filePath
            }

            Text {
                id: nameText
                text: fileName
                anchors.left: fileimage.right
                anchors.leftMargin: font.pixelSize
                anchors.right: sizeText.left
                elide: Text.ElideMiddle

                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor
            }

            Text {
                id: sizeText
                text: humanReadableFileSize(fileSize, true)
                anchors.right: modifiedText.left
                visible: !fileIsDir

                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor
            }

            Text {
                id: modifiedText
                text: humanReadableModificationDate(fileModified)
                anchors.right: wrapper.right
                width: textColumnSize
                horizontalAlignment: Text.AlignRight

                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor
            }

            MouseArea {
                id: mouseRegion
                anchors.fill: parent
                onPressed: wrapper.ListView.view.currentIndex = index
                onClicked: clickAnimation.start()
                onPressAndHold: {
                    if (fileBrowser.allowOpeningFolder && fileIsDir
                            && filesInDir > 0) {
                        curtain.showDialog(filesInDir, filePath)
                    }
                }
            }

            ColorAnimation on color {
                id: clickAnimation
                running: false
                from: Style.fileBrowserBlinkColor
                to: "transparent"
                onStopped: fileBrowser.openItem(filePath, fileIsDir)
            }
        }
    }

    Item {
        id: listviewTable
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: listViewButton.checked

        ListView {
            id: listview
            anchors.top: sortBar.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: scrollBar.visible ? 0.5 * scrollBar.width
                                                     + anchors.leftMargin : anchors.leftMargin
            anchors.leftMargin: itemSize * Style.mainPanelRelMargin
            spacing: itemSize * Style.fileBrowserListItemSpacing
            clip: true

            model: DelegateModel {
                id: listDelegateModel
                model: folders
                rootIndex: folders.getPathIndex(folders.rootFolder)

                Connections {
                    target: folders
                    onRootFolderChanged: rootIndex = folders.getPathIndex(
                                             folders.rootFolder)
                }

                // Mysterious hack to prevent rootIndex from being reset
                // asynchronously when using a QFileSystemModel:
                // http://lists.qt-project.org/pipermail/interest/2015-August/018369.html
                onRootIndexChanged: {
                    rootIndex = folders.getPathIndex(folders.rootFolder)
                }
                delegate: listFileDelegate
            }

            section.property: folders.sortCategory == FolderModel.Name ? "fileName" : ""
            section.criteria: ViewSection.FirstCharacter
            section.delegate: Item {
                width: parent.width
                height: 1.5 * textPixelSize
                Text {
                    text: section
                    font.capitalization: Font.Capitalize
                    font.pixelSize: textPixelSize
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
                property bool active: folders.sortCategory == FolderModel.Name
                text: (active ? folders.sortIcon + " " : "") + "name"
                horizontalAlignment: Text.AlignHCenter
                anchors.left: sortBar.left
                anchors.right: sizeText.left

                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (parent.active)
                            folders.toggleSortOrder()
                        else
                            folders.sortCategory = FolderModel.Name
                    }
                }
            }
            Text {
                id: sizeText
                property bool active: folders.sortCategory == FolderModel.Size
                text: (active ? folders.sortIcon + " " : "") + "size"
                anchors.right: modifiedText.left
                width: textColumnSize
                horizontalAlignment: Text.AlignRight

                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (parent.active)
                            folders.toggleSortOrder()
                        else
                            folders.sortCategory = FolderModel.Size
                    }
                }
            }
            Text {
                id: modifiedText
                property bool active: folders.sortCategory == FolderModel.Date
                text: (active ? folders.sortIcon + " " : "") + "modified"
                anchors.right: sortBar.right
                width: textColumnSize
                horizontalAlignment: Text.AlignRight

                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (parent.active)
                            folders.toggleSortOrder()
                        else
                            folders.sortCategory = FolderModel.Date
                    }
                }
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
            font.pixelSize: 0.3 * height
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
                        folders.sortCategory = FolderModel.Name
                        folders.sortOrder = FolderModel.Ascending
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
            textAccept: "open " + itemCount + " items"

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

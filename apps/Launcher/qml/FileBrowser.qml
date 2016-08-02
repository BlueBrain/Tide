// Copyright (c) 2015-2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
//                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>

import QtQuick 2.4
import QtQuick.Controls 1.0
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
    property real textPixelSize: itemSize * Style.fileBrowserTextSizeRelToItem

    signal itemSelected(string file)

    property alias listViewMode: listViewButton.checked

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

    function openCurrentItem(filePath, fileIsDir) {
        var path = "file://";
        if (filePath.length > 2 && filePath[1] === ':')
            path += '/';
        path += filePath;

        if (fileIsDir)
            goDown(path);
        else
            selectFile(path.replace("file://", ""));
    }

    function humanReadableFileSize(bytes, si) {
        var thresh = si ? 1000 : 1024;
        if(Math.abs(bytes) < thresh)
            return bytes + ' bytes';
        var units = si ? ['kB','MB','GB','TB','PB','EB','ZB','YB']
                       : ['KiB','MiB','GiB','TiB','PiB','EiB','ZiB','YiB'];
        var u = -1;
        do {
            bytes /= thresh;
            ++u;
        } while(Math.abs(bytes) >= thresh && u < units.length - 1);
        return bytes.toFixed(1) + ' ' + units[u];
    }

    function humanReadableModificationDate(modDate) {
        var now = new Date();
        var date = new Date(modDate);
        if (now - date < 1000 * 3600 * 24) // less than one day
            return date.toLocaleTimeString();
        if (date.getFullYear() == now.getFullYear())
            return Qt.formatDate(date, "dd MMMM");
        return Qt.formatDate(date, "dd MMM yyyy");
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
                font.bold: true
                color: Style.fileBrowserTextColor
                font.pixelSize: textPixelSize
            }

            MouseArea {
                id: mouseRegion
                anchors.fill: parent
                onPressed: wrapper.GridView.view.currentIndex = index
                onClicked: clickAnimation.start()
            }

            ColorAnimation on color {
                id: clickAnimation
                running: false
                from: Style.fileBrowserBlinkColor
                to: "transparent"
                onStopped: fileBrowser.openCurrentItem(filePath, fileIsDir)
            }
        }
    }

    GridView {
        id: gridview
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: itemSize * Style.mainPanelRelMargin
        visible: gridViewButton.checked

        cellWidth: itemSize * 1.33
        cellHeight: cellWidth
        model: folders
        delegate: gridFileDelegate
    }

    Component {
        id: listFileDelegate
        Rectangle {
            id: wrapper
            width: parent.width
            height: 0.5 * itemSize
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
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: sizeText.left
                elide: Text.ElideMiddle
                font.bold: true
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor
            }

            Text {
                id: sizeText
                text: humanReadableFileSize(fileSize, true)
                anchors.right: modifiedText.left
                anchors.rightMargin: font.pixelSize
                anchors.verticalCenter: parent.verticalCenter
                visible: !fileIsDir
                font.bold: true
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor
            }

            Text {
                id: modifiedText
                text: humanReadableModificationDate(fileModified)
                anchors.right: wrapper.right
                anchors.verticalCenter: parent.verticalCenter
                width: 10 * font.pixelSize
                horizontalAlignment: Text.AlignRight
                font.bold: true
                font.pixelSize: textPixelSize
                color: Style.fileBrowserTextColor
            }

            MouseArea {
                id: mouseRegion
                anchors.fill: parent
                onPressed: wrapper.ListView.view.currentIndex = index
                onClicked: clickAnimation.start()
            }

            ColorAnimation on color {
                id: clickAnimation
                running: false
                from: Style.fileBrowserBlinkColor
                to: "transparent"
                onStopped: fileBrowser.openCurrentItem(filePath, fileIsDir)
            }
        }
    }

    ListView {
        id: listview
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: itemSize * Style.mainPanelRelMargin
        visible: listViewButton.checked
        model: folders
        spacing: anchors.margins
        delegate: listFileDelegate
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
            font.pixelSize: 0.5 * height
            verticalAlignment: Text.AlignVCenter
            text: folders.folder.toString().replace(folders.constRootFolder+"/", "")
            elide: Text.ElideMiddle
            Behavior on color { PropertyAnimation{ }}
        }
        Row {
            id: viewButtons
            anchors.top: titleBar.top
            anchors.bottom: titleBar.bottom
            anchors.right: titleBar.right
            anchors.margins: titleBar.spacing
            spacing: titleBar.spacing
            ExclusiveGroup { id: viewButtonsGroup }
            FileBrowserViewButton {
                id: gridViewButton
                exclusiveGroup: viewButtonsGroup
                checked: !listViewMode
                height: parent.height
                icon: "qrc:/images/gridview.svg"
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
                when: folders.folder.toString() === folders.constRootFolder
                PropertyChanges {
                    target: titleText
                    color: Style.fileBrowserDiscreteTextColor
                    text: folders.constRootFolder.replace("file://", "")
                }
                AnchorChanges { target: titleText; anchors.left: titleBar.left }
                PropertyChanges {
                    target: upButton
                    visible: false
                }
            }
        ]
        transitions: Transition {
            // smoothly reanchor titleText and move into new position
            AnchorAnimation { easing.type: Easing.OutQuad }
        }
    }
}

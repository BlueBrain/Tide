// Copyright (c) 2016-2019, EPFL/Blue Brain Project
//                     Pawel Podhajski <pawel.podhajski@epfl.ch>
import QtQuick 2.3
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import "style.js" as Style

import Launcher 1.0

Rectangle {

    color: "grey"
    id: searchPanel
    anchors.fill: parent

    property int listItemSize: height * Style.fileBrowserItemSizeRel
    property string rootfolder: ""
    property alias titleBarHeight: titleBar.height
    property string info: ""
    property bool inProgress: false
    property string previousSearchPhase

    signal itemSelected(string file)

    BusyIndicator {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width / 3
        height: parent.height / 3
        running: inProgress;
    }

    Text {
        width: parent.width
        visible: info.length > 0
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        color: Style.fileBrowserTextColor
        font.pixelSize: smallTextPixelSize
        text: info

    }

    ListView {
        id: listview
        anchors.top: sortBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: listItemSize * Style.mainPanelRelMargin
        anchors.rightMargin: listItemSize * Style.mainPanelRelMargin
        spacing: 10
        model: fileList
        delegate: FileBrowserListItem {
            width: parent.width
            height: listview.height * Style.searchListItemRelSize
            onClicked: openItem(filePath)
        }
    }

    ListModel {
        id: fileList
    }

    ScrollBar {
        id: scrollBar
        anchors.top: listview.top
        anchors.right: parent.right
        flickable: listview
    }

    Rectangle {
        id: sortBar
        height: parent.titleBarHeight / 2
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: fileList.count > 0
        color: Style.fileBrowserBackgroundColor

        Text {
            id: nameText
            text: "name"
            horizontalAlignment: Text.AlignHCenter
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: smallTextPixelSize
            color: Style.fileBrowserTextColor
            width: textColumnSize
        }
        Text {
            id: sizeText
            text: "size"
            anchors.right: modifiedText.left
            width: textColumnSize
            horizontalAlignment: Text.AlignRight
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: smallTextPixelSize
            color: Style.fileBrowserTextColor
        }
        Text {
            id: modifiedText
            text: "modified"
            anchors.right: sortBar.right
            anchors.rightMargin: listItemSize * Style.mainPanelRelMargin
            width: textColumnSize
            horizontalAlignment: Text.AlignRight
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: smallTextPixelSize
            color: Style.fileBrowserTextColor
        }
    }

    Rectangle {
        id: textBackground
        width: parent.width
        height: parent.height * 0.05
        anchors.bottom: virtualKeyboard.top
        color: Style.fileBrowserTitleBarColor

        TextField {
            id: textInput
            text: ''
            placeholderText: "Search for a file (minimum 3 characters)"
            height: parent.height
            anchors.left: parent.left
            anchors.right: saveButton.left
            style: TextFieldStyle {
                font.pixelSize: standardTextPixelSize
            }
            onFocusChanged: {
                if (focus)
                    Qt.inputMethod.show()
                else
                    Qt.inputMethod.hide()
            }
            Connections {
                target: Qt.inputMethod
                onVisibleChanged: {
                    if (!Qt.inputMethod.visible)
                        textInput.focus = false
                }
            }
            selectByMouse: true
            validator: RegExpValidator {
                regExp: /[\w.]*/
            }
            onAccepted: search()
        }

        Button {
            id: saveButton
            height: parent.height
            width: parent.width * 0.15
            anchors.right: parent.right
            style: ButtonStyle {
                label: Text {
                    renderType: Text.NativeRendering
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: standardTextPixelSize
                    text: control.text
                    color: control.enabled ? "black" : "gray"
                }
            }

            text: "Search"
            enabled: textInput.text.length > 2 && !inProgress
            onClicked: search()
        }
    }
    Component.onCompleted: {
        textInput.selectAll()
        textInput.focus = true
    }

    Loader {
        id: virtualKeyboard

        source: "qrc:/virtualkeyboard/InputPanel.qml"
        width: parent.width
        height: width / 4
        y: Qt.inputMethod.visible ? parent.height - height : parent.height
        Behavior on y {
            PropertyAnimation {
                easing.type: Easing.InOutQuad
                duration: 150
            }
        }
    }

    function fillFileList(files) {
        fileList.clear()
        if (files.length === 0) {
            info = "No results for: " + previousSearchPhase
            return;
        }
        for (var i = 0; i < files.length; ++i) {
            fileList.append({
                                "fileName": files[i].path,
                                "filePath": rootfolder + "/" + files[i].path,
                                "fileSize": files[i].size,
                                "fileModified": new Date(files[i].lastModified),
                                "fileIsDir": files[i].isDir
                            })
        }
    }

    Rectangle {
        id: titleBar
        width: parent.width
        height: parent.height * Style.titleBarRelHeight
        color: Style.fileBrowserTitleBarColor
        anchors.top: parent.top
        property real spacing: Style.fileBrowserTitleBarSpacing * height

        Text {
            id: titleText
            anchors.left: parent.left
            anchors.margins: titleBar.spacing
            height: titleBar.height
            color: Style.fileBrowserTextColor
            font.pixelSize: standardTextPixelSize
            verticalAlignment: Text.AlignVCenter
            text: "Search file system"
        }
    }

    function openItem(filePath) {
        itemSelected(filePath)
    }

    function search() {
        // In order to ignore virtualKeyboard enter press
        if (inProgress)
            return;
        searchFile(textInput.text)
        previousSearchPhase = textInput.text
        info = ""
        textInput.text = ""
        textInput.focus = false
    }

    function searchFile(fileName) {
        var request = new XMLHttpRequest()
        var url = "http://" + restHost + ":" + restPort + "/tide/find/?file=" + fileName
        inProgress = true
        request.onreadystatechange = function () {
            if (request.readyState === XMLHttpRequest.DONE
                    && request.status === 200) {
                var files = JSON.parse(request.response)
                fillFileList(files)
                inProgress = false;
            }
        }
        request.open('GET', url, true)
        request.send()
    }
}

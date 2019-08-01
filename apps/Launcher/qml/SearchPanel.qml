// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.3
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import "style.js" as Style

import Launcher 1.0


Rectangle {
    color: "grey"
    id: searchPanel
    anchors.fill: parent

    function searchFileLocalRpc(endpointuri) {

        var request = new XMLHttpRequest()
        var fileName = textInput.text
        // var url = "http://" + restHost + ":" + restPort + "/tide/find/?file=" + fileName
        var url = "http://" + 'localhost' + ":" + '8888' + "/tide/find/?file=" + fileName

        request.onreadystatechange = function () {
            if (request.readyState === XMLHttpRequest.DONE
                    && request.status == 200) {
                    var demos = JSON.parse(request.response)
                    fillDemoList(demos)
            }
        }
        request.open('GET', url, true)
        request.send()
    }

    signal searchFile(string filename)
    signal refreshSessionName
    function save() {
        // saveSession(browser.currentFolder.toString().replace(
        // "file://", "") + '/' + textInput.text)
        // searchFile()
        searchFileLocalRpc()
        textInput.text = ""
        textInput.focus = false
    }

      FolderModel {
        id: folders
        rootFolder: '/nfs4/bbp.epfl.ch/media/DisplayWall'
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


 FileBrowserListView {
        visible: true;

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        foldersModel: folders
        // listItemSize: itemSize

        onOpenItem: fileBrowser.openItem(filePath, fileIsDir)
        onFolderAction: {
            if (fileBrowser.allowOpeningFolder) {
                curtain.showDialog(filesInDir, filePath)
            }
        }
    }


    // ListView {
    //     id: demoView
    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     spacing: 10
    //     model: demoList
    //     delegate: demoButtonDelegate
    // }

    // ListModel {
    //     id: demoList
    // }

    // Component {
    //     id: demoButtonDelegate
    //     Item {
    //         width: parent.width
    //         height: 25
    //         Rectangle {
    //             color: "red"
    //             id: placeholder
    //             width: parent.width
    //             height: 25
    //             Text {
    //                 id: caption
    //                 text: demoName
    //                 color: "black"
    //             }
    //         }
    //     }
    // }

    Rectangle {
        id: textBackground
        width: parent.width
        height: parent.height * 0.05
        anchors.bottom: virtualKeyboard.top
        color: Style.fileBrowserTitleBarColor

        TextField {
            id: textInput
            text: 'pawel'
            placeholderText: "Search for a file"
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
            onAccepted: save()
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
            text: "save"
            enabled: textInput.text.length > 0
            onClicked: save()
        }
    }
    Component.onCompleted: {
        textInput.selectAll()
        textInput.focus = true
        refreshSessionName()
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

    function fillDemoList(demos) {
        // folders.clear();
        // for (var i = 0; i < demos.length; ++i) {
        //     folders.append({
        //         "demoName": demos[i].name
        //     })
        // }
    }
}

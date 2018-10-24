// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.3
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import "style.js" as Style

Item {
    id: savePanel
    anchors.fill: parent

    property alias rootfolder: browser.rootfolder
    property alias nameFilters: browser.nameFilters
    property alias listViewMode: browser.listViewMode
    property alias gridViewSortByDate: browser.gridViewSortByDate
    property alias hideExtensions: browser.hideExtensions

    signal saveSession(string filename)
    signal refreshSessionName

    function save() {
        saveSession(browser.currentFolder.toString().replace(
                        "file://", "") + '/' + textInput.text)
        textInput.text = ""
        textInput.focus = false
    }

    FileBrowser {
        id: browser
        anchors.top: parent.top
        anchors.bottom: textBackground.top
        width: parent.width
        itemSize: parent.height * Style.fileBrowserItemSizeRel
        titleBarHeight: parent.height * Style.titleBarRelHeight
        onItemSelected: textInput.text = fileInfo.baseNameFromPath(file)
    }
    Rectangle {
        id: textBackground
        width: parent.width
        height: parent.height * 0.05
        anchors.bottom: virtualKeyboard.top
        color: Style.fileBrowserTitleBarColor

        TextField {
            id: textInput
            placeholderText: "Session name"
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

    function updateSessionName(session) {
        textInput.text = session.filename
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
}

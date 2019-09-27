// Copyright (c) 2019, EPFL/Blue Brain Project
//                     Pawel Podhajski <pawel.podhajski@epfl.ch>

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

    function filter() {
        gridViewSortByDate =  false
        browser.hideFolders(true);
        nameFilters = ["*" + textInput.text + "*.dcx"]
    }

    function clearFilter()
    {
        browser.hideFolders(false);
        textInput.text = ""
        textInput.focus = false
        nameFilters = ["*.dcx"]
    }

    FileBrowser {
        id: browser
        anchors.top: parent.top
        anchors.bottom: textBackground.top
        width: parent.width
        itemSize: parent.height * Style.fileBrowserItemSizeRel
        onItemSelected: sendJsonRpc("application", "load", file)
        titleBarHeight: parent.height * Style.titleBarRelHeight
    }

    Rectangle {
        id: textBackground
        width: parent.width
        height: parent.height * 0.05
        anchors.bottom: virtualKeyboard.top
        color: Style.fileBrowserTitleBarColor

        TextField {
            id: textInput
            placeholderText: "Search for a session"
            height: parent.height
            anchors.left: parent.left
            anchors.right: searchButton.left
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
            onAccepted: filter()
        }
        Button {
            id: clearButton
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
            text: "clear"
            enabled: {
                return nameFilters != "*.dcx"
            }
            onClicked: clearFilter()
        }
        Button {
            id: searchButton
            height: parent.height
            width: parent.width * 0.15
            anchors.right: clearButton.left
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
            enabled: textInput.text.length > 0
            onClicked: filter()
        }

    }
    Component.onCompleted: {
        textInput.selectAll()
        textInput.focus = true
        Qt.inputMethod.hide()
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

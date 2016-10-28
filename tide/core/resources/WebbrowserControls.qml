// Copyright (c) 2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0
import Tide 1.0
import "style.js" as Style

Item {
    property bool isWall: (typeof contentsync !== "undefined")
    property string restHost: "localhost"
    property int restPort: contentwindow.content.restPort

    ListView {
        id: buttons
        anchors.left: parent.left
        anchors.margins: 0.5 * Style.windowBorderWidth
        height: parent.height
        width: count * height
        orientation: ListView.Horizontal
        interactive: false // disable flickable behaviour
        delegate: ControlButton {
            height: buttons.height
            width: height
            image: action.checked ? action.iconChecked : action.icon
            opacity: action.enabled ? Style.buttonsEnabledOpacity : Style.buttonsDisabledOpacity
            MouseArea {
                visible: !isWall
                anchors.fill: parent
                onClicked: action.trigger()
            }
        }
        model: contentwindow.content.actions
    }

    RectangularGlow {
        id: focusEffect
        anchors.fill: addressBar
        cornerRadius: glowRadius
        color: Style.windowFocusGlowColor
        glowRadius: Style.windowFocusGlowRadius
        spread: Style.windowFocusGlowSpread
        visible: contentwindow.content.addressBar.focused
        onVisibleChanged: {
            contentwindow.content.keyboard.visible = visible
            addressBar.focus = visible
        }
    }

    TextField {
        id: addressBar
        anchors.left: buttons.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: Style.windowBorderWidth
        font.pixelSize: 0.5 * (parent.height - 2 * Style.windowBorderWidth)

        readOnly: true
        placeholderText: "enter url"
        text: contentwindow.content.addressBar.url

        Rectangle {
            color: "white"
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 0.1 * parent.height
            width: height
            ControlButton {
                id: goButton
                anchors.fill: parent
                anchors.margins: 0.1 * parent.height
                image: "qrc:/img/right_arrow.svg"
                opacity: addressBar.text.length > 0 ? Style.buttonsEnabledOpacity :
                                                      Style.buttonsDisabledOpacity
                MouseArea {
                    visible: !isWall
                    anchors.fill: parent
                    onClicked: addressBar.enterKeyPressed()
                }
            }
        }

        function activateAddressBar() {
            contentwindow.content.addressBar.focused = true
        }
        function deactivateAddressBar() {
            contentwindow.content.addressBar.focused = false
            focus = false
        }

        onCursorPositionChanged: {
            if (!isWall && focus) {
                activateAddressBar()
                contentwindow.content.addressBar.cursorPosition = cursorPosition
            }
        }
        onSelectionStartChanged: {
            if (!isWall)
                contentwindow.content.addressBar.selectionStart = selectionStart
        }
        onSelectionEndChanged: {
            if (!isWall)
                contentwindow.content.addressBar.selectionEnd = selectionEnd
        }
        property int selectionStartProxy: contentwindow.content.addressBar.selectionStart
        onSelectionStartProxyChanged: {
            if (isWall)
                addressBar.select(selectionStartProxy, selectionEndProxy)
        }
        property int selectionEndProxy: contentwindow.content.addressBar.selectionEnd
        onSelectionEndProxyChanged: {
            if (isWall)
                addressBar.select(selectionStartProxy, selectionEndProxy)
        }
        Component.onCompleted: {
            if (!isWall) {
                contentcontroller.keyboardInput.connect(keyboardInput)
                contentcontroller.deleteKeyPressed.connect(deleteKeyPressed)
                contentcontroller.enterKeyPressed.connect(enterKeyPressed)
            }
        }
        function keyboardInput(key) {
            if (selectionEnd > selectionStart) {
                var positionBackup = selectionStart
                contentwindow.content.addressBar.url = text.substring(0,selectionStart) + key +
                                                       text.substring(selectionEnd,text.length)
            }
            else {
                var positionBackup = cursorPosition
                contentwindow.content.addressBar.url = text.substring(0,cursorPosition) + key +
                                                       text.substring(cursorPosition,text.length)
            }
            cursorPosition = positionBackup + key.length
        }
        function deleteKeyPressed() {
            if (selectionEnd > selectionStart)
                keyboardInput("")
            else {
                var positionBackup = cursorPosition
                contentwindow.content.addressBar.url = text.substring(0,cursorPosition-1) +
                                                       text.substring(cursorPosition,text.length)
                cursorPosition = positionBackup - 1
            }
        }
        function enterKeyPressed() {
            sendRestCommand("load", contentwindow.content.addressBar.url, deactivateAddressBar)
        }
    }
    function sendRestCommand(action, file, callback) {
        sendRestData(action, "uri", file, callback);
    }
    function sendRestData(action, key, value, callback) {
        var request = new XMLHttpRequest();
        var url = "http://"+restHost+":"+restPort+"/"+action;
        var payload = typeof(value) === 'string' ? '{ "'+key+'" : "'+value+'" }'
                                                 : '{ "'+key+'" : '+value+' }';
        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE && request.status == 200) {
                if (typeof callback !== 'undefined')
                    callback();
            }
        }
        request.open("PUT", url, true);
        request.send(payload);
    }
}

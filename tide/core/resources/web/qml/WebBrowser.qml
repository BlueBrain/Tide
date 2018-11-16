// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0

Loader {
    property string url

    // Pass javascript messages by signal to Tide's custom C++ log handler
    signal jsMessage(int level, string message, int line, string source)

    source: "qrc:/web/qml/WebBrowserOffscreen.qml"

    // Note: always compare 'source' with '==' (not '===') using full path
    property bool valid: source != "qrc:/web/qml/WebBrowserMissing.qml"

    function goBack() {
        if (valid) {
            item.goBack()
        }
    }
    function goForward() {
        if (valid) {
            item.goForward()
        }
    }

    focus: true // let loaded components get focus

    onStatusChanged: {
        if (status == Loader.Error) {
            if (source == "qrc:/web/qml/WebBrowserOffscreen.qml") {
                source = "qrc:/web/qml/WebBrowserLegacy.qml"
            } else {
                source = "qrc:/web/qml/WebBrowserMissing.qml"
            }
        }
    }

    onLoaded: {
        item.url = Qt.binding(function () {
            return url
        })
        item.urlChanged.connect(function () {
            url = item.url
        })
        if (valid) {
            item.onJavaScriptConsoleMessage.connect(jsMessage)
            item.settings.localContentCanAccessRemoteUrls = true
            item.settings.pluginsEnabled = true
        }
    }
}

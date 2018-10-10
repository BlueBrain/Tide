// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
//                          Pawel Podhajski <pawel.podhajski@epfl.ch>
import QtQuick 2.4
import QtQuick.Window 2.2
import QtWebEngine 1.1
import "style.js" as Style

Rectangle {
    id: root

    property string tideVersion: "undef"
    property string tideRevision: "undef"
    property string restHost: "localhost"
    property int restPort: 0
    property variant filesFilter: [] // list<string>
    property string rootFilesFolder: ""
    property string rootSessionsFolder: ""

    property string demoServiceUrl: ""
    property string demoServiceImageFolder: ""
    property string demoServiceDeflectHost: ""

    property alias powerButtonVisible: menu.poweroffItemVisible // set from cpp
    property bool useListViewMode: false // retain presentation mode, shared for all panels

    width: Style.windowDefaultSize.width
    height: Style.windowDefaultSize.height

    color: Style.windowBackgroundColor

    Row {
        LaunchMenu {
            id: menu
            width: Style.menuWidth * root.width
            height: root.height
            demoItemVisible: demoServiceUrl && demoServiceDeflectHost
            onClearSession: sendJsonRpc("controller", "clear-all")
            onStartWebbrowser: sendJsonRpc("application", "browse", "")
            onPoweroffScreens: sendJsonRpc("application", "poweroff")
            onStartWhiteboard: sendJsonRpc("application", "whiteboard")
            onShowFilesPanel: centralWidget.sourceComponent = fileBrowser
            onShowSessionsPanel: centralWidget.sourceComponent = sessionsBrowser
            onShowSaveSessionPanel: centralWidget.sourceComponent = saveSessionPanel
            onShowDemosPanel: {
                centralWidget.sourceComponent = defaultPanel
                demoLauncherWidget.visible = true
            }
            onShowOptionsPanel: centralWidget.sourceComponent = optionsPanel
        }
        Loader {
            id: demoLauncherWidget
            width: root.width - menu.width
            height: root.height
            sourceComponent: demoLauncher
            visible: false
            focus: true // let loaded components get focus
        }
        Loader {
            id: centralWidget
            width: root.width - menu.width
            height: root.height
            sourceComponent: defaultPanel
            focus: true // let loaded components get focus
            onSourceComponentChanged: demoLauncherWidget.visible = false
        }
    }

    Component {
        id: defaultPanel
        DefaultPanel {
            tideVersion: root.tideVersion
            tideRevision: root.tideRevision
        }
    }

    Component {
        id: fileBrowser
        FileBrowser {
            onItemSelected: sendJsonRpc("application", "open", file)
            rootfolder: rootFilesFolder
            nameFilters: filesFilter
            listViewMode: useListViewMode
            onListViewModeChanged: useListViewMode = listViewMode
            allowOpeningFolder: true
        }
    }

    Component {
        id: sessionsBrowser
        FileBrowser {
            onItemSelected: sendJsonRpc("application", "load", file)
            rootfolder: rootSessionsFolder
            nameFilters: ["*.dcx"]
            listViewMode: useListViewMode
            onListViewModeChanged: useListViewMode = listViewMode
        }
    }

    Component {
        id: saveSessionPanel
        SavePanel {
            rootfolder: rootSessionsFolder
            nameFilters: ["*.dcx"]
            onSaveSession: sendJsonRpc("application", "save", filename)
            listViewMode: useListViewMode
            onListViewModeChanged: useListViewMode = listViewMode
            onRefreshSessionName: sendRestQuery("session", updateSessionName)
        }
    }

    Component {
        id: optionsPanel
        OptionsPanel {
            tideVersion: root.tideVersion
            tideRevision: root.tideRevision
            onButtonClicked: sendRestOption(optionName, value)
            onExitClicked: sendJsonRpc("application", "exit")
            onRefreshOptions: sendRestQuery("options", updateCheckboxes)
        }
    }

    Component {
        id: demoLauncher
        WebEngineView {
            id: webview
            url: demoServiceUrl + demoServiceDeflectHost
            anchors.fill: parent
            onCertificateError: {
                error.ignoreCertificateError()
            }
        }
    }

    function sendJsonRpc(endpoint, action, uri) {
        var obj = {
            "jsonrpc": "2.0",
            "method": action
        }
        if (typeof uri !== "undefined") {
            obj.params = {
                "uri": uri
            }
        }
        sendRestData(endpoint, "POST", JSON.stringify(obj))
    }

    function makeJsonString(key, value) {
        var obj = new Object
        obj[key] = value
        return JSON.stringify(obj)
    }

    function sendRestOption(optionName, value) {
        sendRestData("options", "PUT", makeJsonString(optionName, value))
    }

    function sendRestData(action, method, payload, callback) {
        var request = new XMLHttpRequest()
        var url = "http://" + restHost + ":" + restPort + "/tide/" + action

        request.onreadystatechange = function () {
            if (request.readyState === XMLHttpRequest.DONE
                    && request.status == 200) {
                if (typeof callback !== 'undefined')
                    callback()
            }
        }
        request.open(method, url, true)
        if (payload) {
            request.send(payload)
        } else {
            request.send()
        }
    }

    function sendRestQuery(action, callback) {
        var request = new XMLHttpRequest()
        var url = "http://" + restHost + ":" + restPort + "/tide/" + action

        request.onreadystatechange = function () {
            if (request.readyState === XMLHttpRequest.DONE
                    && request.status == 200) {
                callback(JSON.parse(request.responseText))
            }
        }
        request.open("GET", url, true)
        request.send()
    }
}

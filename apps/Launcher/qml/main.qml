// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
//                          Pawel Podhajski <pawel.podhajski@epfl.ch>
import QtQuick 2.4
import QtQuick.Window 2.2
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

    property real headerTextPixelSize: Style.headerRelTextSize * menu.width
    property real standardTextPixelSize: Style.sectionRelTextSize * menu.width
    property real smallTextPixelSize: Style.menuRelTextSize * menu.width
    property real textColumnSize: smallTextPixelSize * 10

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
            onShowSearchPanel: centralWidget.sourceComponent = searchPanel
            onShowDemosPanel: {
                centralWidget.sourceComponent = defaultPanel
                demoLauncherWidget.active = true
                demoLauncherWidget.visible = true
            }
            onShowOptionsPanel: centralWidget.sourceComponent = optionsPanel
        }
        Loader {
            id: demoLauncherWidget
            width: root.width - menu.width
            height: root.height
            source: "qrc:/web/qml/WebBrowser.qml"
            active: false
            visible: false
            focus: true // let loaded components get focus
            onLoaded: item.url = demoServiceUrl + demoServiceDeflectHost
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
            gridViewSortByDate: true
            hideExtensions: true
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
            gridViewSortByDate: true
            hideExtensions: true
            onListViewModeChanged: useListViewMode = listViewMode
            onRefreshSessionName: sendRestQuery("session", updateSessionName)
        }
    }

    Component {
        id: searchPanel
        SearchPanel {
            onItemSelected: sendJsonRpc("application", "open", file)
            rootfolder: rootFilesFolder
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

    function humanReadableModificationDate(date) {
        var now = new Date()

        if (now - date < 1000 * 3600 * 24)
            // less than one day
            return date.toLocaleTimeString()
        if (date.getFullYear() == now.getFullYear())
            return Qt.formatDate(date, "dd MMMM")
        return Qt.formatDate(date, "dd MMM yyyy")
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
}

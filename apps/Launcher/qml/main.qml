// Copyright (c) 2016, EPFL/Blue Brain Project
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.4
import QtQuick.Window 2.2

import "DemoLauncher"
import "style.js" as Style

Rectangle {
    id: root

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
            demoItemVisible: demoServiceUrl && demoServiceImageFolder && demoServiceDeflectHost
            onClearSession: sendJsonRpc("controller", "clear");
            onStartWebbrowser: sendJsonRpc("application", "browse", "");
            onPoweroffScreens: sendJsonRpc("application", "poweroff");
            onStartWhiteboard: sendJsonRpc("application", "whiteboard");
            onShowFilesPanel: centralWidget.sourceComponent = fileBrowser
            onShowSessionsPanel: centralWidget.sourceComponent = sessionsBrowser
            onShowSaveSessionPanel: centralWidget.sourceComponent = saveSessionPanel
            onShowDemosPanel: centralWidget.sourceComponent = demoLauncher
            onShowOptionsPanel: centralWidget.sourceComponent = optionsPanel
        }
        Loader {
            id: centralWidget
            width: root.width - menu.width
            height: root.height
            sourceComponent: defaultPanel
            focus: true // let loaded components get focus
        }
    }

    Component {
        id: defaultPanel
        DefaultPanel {
        }
    }

    Component {
        id: fileBrowser
        FileBrowser {
            onItemSelected: sendJsonRpc("application", "open", file);
            rootfolder: rootFilesFolder
            nameFilters: filesFilter
            listViewMode: useListViewMode
            onListViewModeChanged: useListViewMode = listViewMode
        }
    }

    Component {
        id: sessionsBrowser
        FileBrowser {
            onItemSelected: sendJsonRpc("application", "load", file);
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
            onSaveSession: sendJsonRpc("application", "save", filename);
            listViewMode: useListViewMode
            onListViewModeChanged: useListViewMode = listViewMode
        }
    }

    Component {
        id: optionsPanel
        OptionsPanel {
            onButtonClicked: sendRestOption(optionName, value)
            onExitClicked: sendJsonRpc("application", "exit")
            onRefreshOptions: getRestOptions(updateCheckboxes)
        }
    }

    Component {
        id: demoLauncher
        DemoLauncher {
            serviceUrl: demoServiceUrl
            imagesFolder: demoServiceImageFolder
            deflectStreamHost: demoServiceDeflectHost
        }
    }

    function sendJsonRpc(endpoint, action, uri) {
        var obj = {jsonrpc: "2.0", method: action};
        if (typeof uri !== "undefined") {
            obj.params = {uri: uri};
        }
        sendRestData(endpoint, "POST", JSON.stringify(obj));
    }

    function makeJsonString(key, value) {
        var obj = new Object;
        obj[key] = value;
        return JSON.stringify(obj);
    }

    function sendRestOption(optionName, value) {
        sendRestData("options", "PUT", makeJsonString(optionName, value));
    }

    function sendRestData(action, method, payload, callback) {
        var request = new XMLHttpRequest();
        var url = "http://"+restHost+":"+restPort+"/tide/"+action;

        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE && request.status == 200) {
                if (typeof callback !== 'undefined')
                    callback();
            }
        }
        request.open(method, url, true);
        if (payload) {
            request.send(payload);
        }
        else {
            request.send();
        }
    }

    function getRestOptions(callback) {
        sendRestQuery("options", callback);
    }

    function sendRestQuery(action, callback) {
        var request = new XMLHttpRequest()
        var url = "http://"+restHost+":"+restPort+"/tide/"+action;

        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE && request.status == 200) {
                callback(JSON.parse(request.responseText))
            }
        }
        request.open("GET", url, true)
        request.send()
    }
}

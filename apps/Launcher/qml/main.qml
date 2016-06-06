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

    width: Style.windowDefaultSize.width
    height: Style.windowDefaultSize.height

    color: Style.windowBackgroundColor

    Row {
        Menu {
            id: menu
            width: Style.menuWidth * root.width
            height: root.height
            onClearSession: sendRestCommand("load", "");
            onStartWebbrowser: sendRestCommand("browse", "");
            onShowFilesPanel: centralWidget.sourceComponent = fileBrowser
            onShowSessionsPanel: centralWidget.sourceComponent = sessionsBrowser
            onShowDemosPanel: centralWidget.sourceComponent = demoLauncher
            onShowOptionsPanel: centralWidget.sourceComponent = optionsPanel
        }
        Loader {
            id: centralWidget
            width: root.width - menu.width
            height: root.height
            sourceComponent: defaultPanel
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
            onItemSelected: sendRestCommand("open", file);
            rootfolder: rootFilesFolder
            nameFilters: filesFilter
        }
    }

    Component {
        id: sessionsBrowser
        FileBrowser {
            onItemSelected: sendRestCommand("load", file);
            rootfolder: rootSessionsFolder
            nameFilters: ["*.dcx"]
        }
    }

    Component {
        id: optionsPanel
        OptionsPanel {
            onButtonClicked: sendRestOption(optionName, value)
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

    function sendRestCommand(action, file) {
        sendRestData(action, "uri", file);
    }

    function sendRestOption(optionName, value) {
        sendRestData("options", optionName, value);
    }

    function sendRestData(action, key, value) {
        var request = new XMLHttpRequest();
        var url = "http://"+restHost+":"+restPort+"/tide/"+action;
        var payload = typeof(value) === 'string' ? '{ "'+key+'" : "'+value+'" }'
                                                 : '{ "'+key+'" : '+value+' }';
        request.open("PUT", url, true);
        request.send(payload);
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

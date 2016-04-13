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
        id: demoLauncher
        DemoLauncher {
            serviceUrl: demoServiceUrl
            imagesFolder: demoServiceImageFolder
        }
    }

    function sendRestCommand(action, file) {
        var xmlhttp = new XMLHttpRequest();
        var url = "http://"+restHost+":"+restPort+"/tide/"+action;

        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState === XMLHttpRequest.DONE) {
                if (xmlhttp.status == 200) {
                    //var arr = JSON.parse(xmlhttp.responseText);
                }
            }
        }
        xmlhttp.open("PUT", url, true);
        xmlhttp.send('{ "uri" : "'+file+'" }');
    }
}

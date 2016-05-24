// Copyright (c) 2014-2016, EPFL/Blue Brain Project
//                          Pawel Podhajski <pawel.podhajski@epfl.ch>
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import "../style.js" as Style
import "RenderingResourcesManager.js" as RRM

Rectangle {
    id: demoLauncher

    signal startApplication(string name)
    signal loadSession(string file)
    signal showFilesPanel()
    signal showSessionsPanel()

    property string serviceUrl: ""
    property string imagesFolder: ""
    property var demosComm: ({})

    property int itemSize: height / 5
    property int gridSize: 1.5 * itemSize
    property real textPixelSize: 0.1 * itemSize

    color: Style.defaultPanelColor

    ListModel {
        id: demoList
    }

    Text {
        width: parent.width
        anchors.centerIn: parent
        font.pixelSize: textPixelSize
        text: "Demos provided by:\n" + serviceUrl
        color: Style.defaultPanelTextColor
        wrapMode: Text.WrapAnywhere
        horizontalAlignment: Text.AlignHCenter
    }

    GridView {
        id: demoView
        anchors.fill: parent
        model: demoList
        delegate: demoButtonDelegate
        cellWidth: gridSize
        cellHeight: gridSize
    }

    Component {
        id: demoButtonDelegate
        Item {
            width: demoView.cellWidth
            height: demoView.cellHeight
            Column {
                anchors.fill: parent
                spacing: 0.1 * image.height
                Image {
                    id: image
                    width: itemSize
                    height: itemSize
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: demoImage
                    asynchronous: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: demosComm.launch(demoName)
                    }
                }
                Text {
                    id: caption
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: demoName
                    font.pixelSize: textPixelSize
                    font.bold: true
                    color: Style.defaultPanelTextColor
                }
            }
        }
    }

    Rectangle {
        id: infoRect
        color: "darkgrey"
        anchors.bottom: parent.bottom
        width: parent.width
        height: 0.1 * parent.height
        Text {
            id: infoText
            font.pixelSize: textPixelSize
        }
        function displayStatusCallback(status) {
           infoText.text = status
        }
        Timer {
            onTriggered: demosComm.querySessionStatus(displayStatusCallback)
        }
    }

    function fillDemoList(demos) {
        for(var i = 0; i < demos.length; ++i)
        {
            demoList.append(
            {
                demoName: demos[i].command_line,
                demoImage: "file://" + imagesFolder + "/" + demos[i].id + ".png"
            });
        }
    }
    Component.onCompleted : {
        demosComm = new RRM.Communicator(serviceUrl);
        demosComm.queryDemos(fillDemoList);
    }
}

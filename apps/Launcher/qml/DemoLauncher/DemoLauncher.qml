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
    property string deflectStreamHost: ""
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
                        onClicked: {
                            infoRect.demo = demoId
                            infoRect.open = true
                            demosComm.launch(demoId)
                        }
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
        color: "black"
        opacity: 0.7
        width: parent.width
        height: parent.height
        x: 0
        y: parent.height

        property bool open: false
        property string demo: ""

        MouseArea {
            id: touchBarrier
            anchors.fill: parent
        }

        Column {
            anchors.centerIn: parent
            spacing: textPixelSize
            Text {
                id: title
                color: "white"
                text: "Starting demo '" + infoRect.demo + "' - please wait..."
                font.pixelSize: textPixelSize
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                id: infoText
                color: "white"
                font.pixelSize: textPixelSize
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Rectangle {
                id: closeButton
                color: "darkgray"
                border.color: "white"
                border.width: 0.05 * width
                width: infoRect.width * 0.10
                height: width * 0.4
                radius: height * 0.25
                anchors.horizontalCenter: parent.horizontalCenter
                Text {
                    anchors.centerIn: parent
                    text: "cancel"
                    font.pixelSize: textPixelSize
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        demosComm.closeCurrentSession()
                        infoRect.open = false
                        infoRect.demo = ""
                    }
                }
            }
        }

        Timer {
            repeat: true
            running: infoRect.open
            onTriggered: demosComm.queryStatus( function (status) { infoText.text = status })
        }
        states: [
            State {
                name: "open"
                when: infoRect.open
                PropertyChanges {
                    target: infoRect
                    y: 0
                }
            }
        ]
        Behavior on y {
            NumberAnimation {
                easing.type: Easing.InOutQuad
            }
        }
    }

    function fillDemoList(demos) {
        for(var i = 0; i < demos.length; ++i)
        {
            demoList.append(
            {
                demoId: demos[i].id,
                demoName: demos[i].command_line,
                demoImage: "file://" + imagesFolder + "/" + demos[i].id + ".png"
            });
        }
    }
    Component.onCompleted : {
        demosComm = new RRM.Communicator(serviceUrl, deflectStreamHost);
        demosComm.queryDemos(fillDemoList);
    }
    Component.onDestruction: {
        if (!demosComm.isRunning())
            demosComm.closeCurrentSession();
    }
}

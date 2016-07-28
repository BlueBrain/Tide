// Copyright (c) 2014-2016, EPFL/Blue Brain Project
//                          Pawel Podhajski <pawel.podhajski@epfl.ch>
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>

import QtQuick 2.0
import QtQuick.Controls 1.2
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

    property int itemSize: height * Style.fileBrowserItemSizeRel
    property real textPixelSize: itemSize * Style.fileBrowserTextSizeRelToItem

    color: Style.defaultPanelColor

    ListView {
        id: demoView
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: itemSize * Style.mainPanelRelMargin
        spacing: itemSize * Style.demoLauncherItemSpacingRel

        model: demoList
        delegate: demoButtonDelegate
    }

    ListModel {
        id: demoList
    }

    Component {
        id: demoButtonDelegate
        Item {
            width: parent.width
            height: itemSize
            Rectangle {
                id: placeholder
                width: itemSize
                height: itemSize

                gradient: Gradient {
                    GradientStop { position: 0.0; color: Style.placeholderTopColor }
                    GradientStop { position: 1.0; color: Style.placeholderBottomColor }
                }
                Image {
                    id: image
                    anchors.fill: parent

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
            }
            Column {
                anchors.left: placeholder.right
                anchors.leftMargin: demoView.spacing
                anchors.right: parent.right
                Text {
                    id: caption
                    text: demoName
                    font.pixelSize: textPixelSize
                    font.bold: true
                    color: Style.defaultPanelTextColor
                }
                Text {
                    id: description
                    text: demoDesc
                    anchors.left: parent.left
                    anchors.right: parent.right
                    font.pixelSize: textPixelSize
                    wrapMode: Text.Wrap
                    color: Style.defaultPanelTextColor
                }
            }
        }
    }

    Rectangle {
        id: titleBar
        width: parent.width
        height: parent.height * Style.titleBarRelHeight
        anchors.top: parent.top
        color: Style.fileBrowserTitleBarColor

        Text {
            id: titleText
            anchors.fill: parent
            anchors.leftMargin: font.pixelSize
            font.pixelSize: parent.height * Style.demoLauncherTitleBarTextHeightRel
            color: Style.fileBrowserDiscreteTextColor
            verticalAlignment: Text.AlignVCenter
            text: "Demos provided by: " + serviceUrl
            elide: Text.ElideRight
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
        property int status: RRM.SESSION_STATUS_STOPPED

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
                text: "Launching '" + infoRect.demo + "'"
                font.pixelSize: textPixelSize
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                id: infoText
                text: demosComm.toStatusString(infoRect.status)
                color: "white"
                font.pixelSize: textPixelSize
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Item {
                height: 3 * infoText.height
                width: height
                anchors.horizontalCenter: parent.horizontalCenter
                Image {
                    id: runningIcon
                    source: "qrc:/images/check_white.svg"
                    anchors.fill: parent
                    sourceSize: Qt.size(width,height)
                    visible: infoRect.status === RRM.SESSION_STATUS_RUNNING
                }
                BusyIndicator {
                    running: infoRect.status < RRM.SESSION_STATUS_RUNNING
                    anchors.fill: parent
                }
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
            onTriggered: demosComm.queryStatus( function (status) { infoRect.status = status })
            onRunningChanged: if(!running) { infoRect.status = RRM.SESSION_STATUS_STOPPED }
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
                demoName: demos[i].name,
                demoDesc: demos[i].description,
                demoImage: "file://" + imagesFolder + "/" + demos[i].id + ".png"
            });
        }
    }
    Component.onCompleted : {
        demosComm = new RRM.Communicator(serviceUrl, deflectStreamHost);
        demosComm.queryDemos(fillDemoList);
    }
    Component.onDestruction: {
        demosComm.closeCurrentSession();
    }
}

// Copyright (c) 2016, EPFL/Blue Brain Project
//                     Pawel Podhajski <pawel.podhajski@epfl.ch>

import QtQuick 2.4
import QtQuick.Window 2.2
import QtQml 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Item {
    id: root

    objectName: "Whiteboard"

    width: 1920
    height: 1080

    property int headerHeight: 100
    property int oldWidth: width
    property int oldHeight: height

    property int brushSize: 15
    property string brushColor: "#FF0000"
    property string saveURL

    property int lastX
    property int lastY
    property int offsetY: 0
    property int offsetX: 0

    property var singleTouchPoint: []
    property var singleLine: []
    property var allCurves: []

    onWidthChanged: {
        offsetX = (width - oldWidth) / 2
        oldWidth = width

        for (var j = 0; j < allCurves.length; j++) {
            var singleCurve = allCurves[j]
            singleCurve[3][0] += offsetX
        }
        canvas.requestPaint()
    }

    onHeightChanged: {
        offsetY = (height - oldHeight) / 2
        oldHeight = height
        for (var j = 0; j < allCurves.length; j++) {
            var singleCurve = allCurves[j]
            singleCurve[3][1] += offsetY
        }
        canvas.requestPaint()
    }

    ListModel {
        id: colorModel
        ListElement {
            color: "#FF4444"
        }
        ListElement {
            color: "#000000"
        }
        ListElement {
            color: "#FFFFFF"
        }
        ListElement {
            color: "#33B5E5"
        }
        ListElement {
            color: "#99CC00"
        }
        ListElement {
            color: "#FFBB33"
        }
    }

    Component {
        id: colorDelegate
        Rectangle {
            width: viewColor.buttonWidth
            height: width
            color: model.color
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    viewColor.currentIndex = index
                    brushColor = model.color
                }
            }
        }
    }

    ListModel {
        id: brushModel
        ListElement {
            size: 1
        }
        ListElement {
            size: 15
        }
        ListElement {
            size: 25
        }
        ListElement {
            size: 40
        }
    }

    Component {
        id: brushDelegate
        Rectangle {
            width: 75
            height: 75

            Rectangle {
                width: model.size
                height: model.size
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "black"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    viewSize.currentIndex = index
                    brushSize = model.size
                }
            }
        }
    }

    Rectangle {
        id: header
        width: root.width
        height: headerHeight
        color: "lightsteelblue"
        anchors.top: parent.top

        Row {
            id: colorTools
            spacing: 5
            property color paintColor: "#33B5E5"

            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.top
                topMargin: 10
            }

            Image {
                id: imgSave
                opacity: 1
                source: "qrc:/images/save.png"
                width: 75
                height: 75
                anchors.leftMargin: 75
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        saveMenu.toggle()
                    }
                }
            }

            Flickable {
                width: viewColor.model.count * (viewColor.buttonWidth + colorTools.spacing)
                height: 75
                anchors.rightMargin: 75
                ListView {
                    anchors.fill: parent
                    id: viewColor
                    model: colorModel
                    delegate: colorDelegate
                    spacing: 5
                    boundsBehavior: Flickable.StopAtBounds
                    property int buttonWidth: 75
                    orientation: ListView.Horizontal
                    highlight: Rectangle {
                        opacity: 0.3
                        z: viewColor.currentItem.z + 1
                        border.color: "grey"
                        border.width: 5
                    }
                }
            }

            Flickable {
                width: viewSize.model.count * (viewSize.buttonWidth + viewSize.spacing)
                height: 75
                anchors.rightMargin: 75

                ListView {
                    anchors.fill: parent
                    id: viewSize
                    model: brushModel
                    delegate: brushDelegate
                    spacing: 5
                    boundsBehavior: Flickable.StopAtBounds
                    property int buttonWidth: 75
                    orientation: ListView.Horizontal
                    highlight: Rectangle {
                        opacity: 0.3
                        z: viewSize.currentItem.z + 1
                        border.color: "grey"
                        border.width: 5
                    }
                    Component.onCompleted: viewSize.currentIndex = 1
                }
            }

            Button {
                iconSource: pressed ? "qrc:/images/full.png" : "qrc:/images/clear.png"
                height: 75
                onClicked: {
                    allCurves = []
                    canvas.getContext("2d").reset()
                    canvas.requestPaint()
                    canvasTmp.getContext("2d").reset()
                    canvasTmp.requestPaint()
                }
                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth: 100
                        implicitHeight: 50
                        border.width: 0
                        border.color: "#7b899b"
                        color: "lightsteelblue"
                        radius: 0
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: root.width
        height: root.height - headerHeight
        Rectangle {
            id: freezePane
            anchors.fill: parent
            anchors.topMargin: saveMenu.height
            color: "lightsteelblue"
            visible: false
            opacity: 0.5
            z: 99
        }

        Text {
            id: infoBox
            text: ""
            font.family: "Helvetica"
            font.pointSize: 60
            color: "red"

            anchors.top: saveMenu.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            z: 100
            visible: false
        }

        Canvas {
            id: canvas
            anchors.fill: parent
            renderTarget: Canvas.FramebufferObject
            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                ctx.fillStyle = "#FFFFFF"
                ctx.fillRect(0, 0, canvas.width, canvas.height)
                for (var j = 0; j < allCurves.length; j++) {
                    var singleCurve = allCurves[j]
                    ctx.lineCap = "round"
                    ctx.beginPath()
                    ctx.strokeStyle = singleCurve[1]
                    ctx.lineWidth = singleCurve[2]
                    ctx.lineCap = "round"
                    ctx.beginPath()
                    ctx.strokeStyle = singleCurve[1]
                    ctx.lineWidth = singleCurve[2]

                    if (singleCurve[0].length < 4) {
                        ctx.arc(singleCurve[0][0][0] + singleCurve[3][0],
                                singleCurve[0][0][1] + singleCurve[3][1], 1, 0,
                                Math.PI * 2, false)
                    } else {
                        for (var i = 0; i < singleCurve[0].length - 1; i++) {
                            ctx.moveTo(singleCurve[0][i][0] + singleCurve[3][0],
                                       singleCurve[0][i][1] + singleCurve[3][1])
                            ctx.lineTo(singleCurve[0][i + 1][0] + singleCurve[3][0],
                                       singleCurve[0][i + 1][1] + singleCurve[3][1])
                        }
                    }
                    ctx.stroke()
                    ctx.closePath()
                    ctx.save()
                }
            }

            MultiPointTouchArea {
                id: area
                enabled: true
                anchors.fill: parent
                property var paths: []
                touchPoints: [
                    TouchPoint {
                        id: point0
                    }
                ]
                onPressed: {
                    if (touchPoints[0] === point0) {
                        singleLine = new Array(0)
                        lastX = point0.x
                        lastY = point0.y
                        singleLine.push([point0.x, point0.y])
                    }
                }
                onUpdated: {
                    singleLine.push([point0.x, point0.y])
                    canvasTmp.requestPaint()
                }
                onReleased: {
                    if (touchPoints[0] === point0) {
                        var singleCurve = new Array(0)
                        singleCurve.push(singleLine)
                        singleCurve.push(brushColor)
                        singleCurve.push(brushSize)
                        singleCurve.push([0, 0])
                        allCurves.push(singleCurve)
                        canvas.requestPaint()
                        paths = []
                        canvasTmp.getContext("2d").reset()
                    }
                }
            }

            Canvas {
                id: canvasTmp
                anchors.fill: parent
                onPaint: {
                    var ctx2 = getContext("2d")
                    ctx2.lineWidth = brushSize
                    ctx2.lineCap = "round"
                    ctx2.strokeStyle = brushColor
                    ctx2.beginPath()
                    ctx2.moveTo(lastX, lastY)
                    lastX = point0.x
                    lastY = point0.y
                    ctx2.lineTo(lastX, lastY)
                    ctx2.stroke()
                    ctx2.closePath()
                }
            }
        }

        Rectangle {
            color: "#e7edf5"
            function toggle() {
                if (saveMenu.state == "on") {
                    saveMenu.state = "off"
                    area.enabled = true
                    freezePane.visible = false

                } else {
                    saveMenu.state = "on"
                    area.enabled = false
                    freezePane.visible = true

                }
            }
            id: saveMenu
            width: root.width
            height: 50
            anchors.top: parent.top
            visible: false
            states: [
                State {
                    name: "on"
                    PropertyChanges {
                        target: saveMenu
                        visible: true
                    }
                    PropertyChanges {
                        target: imgSave
                        opacity: 0.1
                    }
                },
                State {
                    name: "off"
                    PropertyChanges {
                        target: saveMenu
                        visible: false
                    }
                }
            ]

            Rectangle {
                color: "#e7edf5"
                anchors.centerIn: parent
                width: 600
                height: parent.height

                Rectangle {
                    color: "#7b899b"
                    anchors.centerIn: parent
                    id: rect5
                    anchors.fill: parent
                    anchors.leftMargin: 50
                    anchors.rightMargin: 50
                    height: parent.height
                    TextInput {
                        wrapMode: TextInput.Wrap
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignHCenter
                        id: inputFileName
                        color: "black"
                        font.pixelSize: 24
                        text: ""
                        anchors.fill: parent
                        focus: true
                    }

                    Button {
                        anchors.left: inputFileName.right
                        text: "Save!"
                        onClicked: {
                            if (inputFileName.text != "") {
                                var path = saveURL + inputFileName.text + ".png"
                                if (canvas.save(path)) {
                                    infoBox.text = "SAVED as " + path
                                    infoBox.visible = true
                                    timer1.start()
                                } else {
                                    infoBox.text = "Error saving!"
                                    infoBox.visible = true
                                    timer1.start()
                                }
                            }
                        }

                        style: ButtonStyle {
                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 50
                                border.width: 0
                                border.color: "#7b899b"
                                radius: 0
                                gradient: Gradient {
                                    GradientStop {
                                        position: 0
                                        color: control.pressed ? "#7b899b" : "#b0c4de"
                                    }
                                    GradientStop {
                                        position: 1
                                        color: control.pressed ? "#b0c4de" : "#7b899b"
                                    }
                                }
                            }
                        }

                        Timer {
                            id: timer1
                            interval: 750
                            running: false
                            repeat: false
                            onTriggered: infoBox.visible = false
                        }
                    }
                }
            }
        }
    }
}

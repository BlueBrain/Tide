// Copyright (c) 2016-2018, EPFL/Blue Brain Project
//                          Pawel Podhajski <pawel.podhajski@epfl.ch>
//                          Raphael Dumusc <raphael.dumusc@epfl.ch>
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

    onWindowChanged: {
        // size constraints for the stream window
        window.minimumWidth = 640
        window.minimumHeight = 480
        window.maximumWidth = 3840
        window.maximumHeight = 2160
    }

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
    property var path
    property var singleTouchPoint: []
    property var singleLine: []
    property var allCurves: []
    property var fileList: []
    property bool pathAvail: false

    function checkFileExists() {
        if (textInput.text != "") {
            path = saveURL + textInput.text + ".png"
            if (fileInfo.fileExists(path)) {
                infoBox.z = 2
                infoText.text = "Are you sure? File already exists"
                textInput.focus = false
                buttonOK.visible = true
            } else {
                saveCanvas()
            }
        }
    }

    function saveCanvas() {
        buttonOK.visible = false
        if (canvas.save(path))
            infoText.text = "Saved as: \n" + path
        else
            infoText.text = "Error saving as: \n" + path
        infoBox.z = 2
    }

    function cancelSave() {
        buttonOK.visible = false
        infoText.text = "Save canceled"
        infoBox.z = 2
    }

    function toggleSavePanel() {
        if (savePanel.state == "on")
            savePanel.state = "off"
        else
            savePanel.state = "on"
    }

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
                radius: width * 0.5
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
                source: "qrc:/whiteboard/images/save.png"
                width: 75
                height: 75
                anchors.leftMargin: 75
                MouseArea {
                    anchors.fill: parent
                    onClicked: toggleSavePanel()
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
                iconSource: pressed ? "qrc:/whiteboard/images/full.png" : "qrc:/whiteboard/images/clear.png"
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
                id: savePanelBackground
                anchors.fill: parent
                enabled: false
                onTouchUpdated: {
                    if (savePanel.visible)
                        toggleSavePanel()
                }
                Rectangle {
                    anchors.fill: parent
                    color: "black"
                    opacity: 0.7
                    visible: parent.enabled
                }
            }
            MultiPointTouchArea {
                id: area
                enabled: !savePanelBackground.enabled
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
            id: savePanel
            width: 800
            height: 250
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            visible: false
            states: [
                State {
                    name: "on"
                    PropertyChanges {
                        target: savePanel
                        visible: true
                    }
                    PropertyChanges {
                        target: imgSave
                        opacity: 0.1
                    }
                    PropertyChanges {
                        target: savePanelBackground
                        enabled: true
                    }
                    PropertyChanges {
                        target: textInput
                        focus: true
                    }
                    PropertyChanges {
                        target: infoBox
                        z: 0
                    }
                },
                State {
                    name: "off"
                    PropertyChanges {
                        target: savePanel
                        visible: false
                    }
                    PropertyChanges {
                        target: savePanelBackground
                        enabled: false
                    }
                    PropertyChanges {
                        target: textInput
                        focus: false
                    }
                }
            ]

            TextField {
                id: textInput
                placeholderText: "File name"
                focus: false
                width: 700
                height: 50
                z: 1
                anchors.left: parent.left
                style: TextFieldStyle {
                    font.pointSize: control.height * 0.5
                }
                onFocusChanged: {
                    if (focus)
                        Qt.inputMethod.show()
                    else
                        Qt.inputMethod.hide()
                }
                selectByMouse: true
                validator: RegExpValidator {
                    regExp: /[\w.]*/
                }
                onAccepted: {
                    checkFileExists()
                    if (textInput.text == "")
                        Qt.inputMethod.show()
                }
                Button {
                    anchors.left: textInput.right
                    text: "Save"
                    implicitWidth: 100
                    implicitHeight: 50
                    onClicked: {
                        checkFileExists()
                    }
                    enabled: (textInput.text == "") ? false : true
                    style: ButtonStyle {
                        label: Text {
                            renderType: Text.NativeRendering
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pointSize: control.height * 0.4
                            text: control.text
                            color: control.enabled ? "black" : "gray"
                        }
                    }
                }
            }
            Rectangle {
                id: infoBox
                anchors.fill: parent
                width: 800
                height: 250
                color: "black"
                Text {
                    id: infoText
                    anchors.fill: parent
                    anchors.margins: 10
                    text: ""
                    font.family: "Verdana"
                    font.pointSize: 35
                    color: "white"
                    wrapMode: Text.Wrap
                }
                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 10
                    height: 50

                    Button {
                        id: buttonOK
                        implicitWidth: 100
                        implicitHeight: 50
                        style: ButtonStyle {
                            label: Text {
                                renderType: Text.NativeRendering
                                horizontalAlignment: Text.AlignHCenter
                                font.pointSize: control.height * 0.3
                                verticalAlignment: Text.AlignVCenter
                                text: "OK"
                                color: control.enabled ? "black" : "gray"
                            }
                        }
                        onClicked: saveCanvas()
                    }
                    Button {
                        id: buttonCancel
                        visible: buttonOK.visible
                        implicitWidth: 100
                        implicitHeight: 50
                        style: ButtonStyle {
                            label: Text {
                                renderType: Text.NativeRendering
                                font.pointSize: control.height * 0.3
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                text: "Cancel"
                                color: control.enabled ? "black" : "gray"
                            }
                        }
                        onClicked: cancelSave()
                    }
                }
            }
            Loader {
                id: virtualKeyboard
                source: "qrc:/virtualkeyboard/InputPanel.qml"
                anchors.top: textInput.bottom
                anchors.right: savePanel.right
                anchors.left: savePanel.left
                anchors.bottom: savePanel.bottom
                visible: Qt.inputMethod.visible ? true : false
            }
        }
    }
}

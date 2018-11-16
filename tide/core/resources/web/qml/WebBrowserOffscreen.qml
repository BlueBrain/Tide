// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
// Qt 5.8
import QtWebEngine 1.4

import "qrc:/web/js/HtmlSelectReplacer.js" as HSR

WebEngineView {
    id: webengine
    objectName: "webengineview"
    anchors.fill: parent

    onCertificateError: {
        error.ignoreCertificateError()
    }
    onAuthenticationDialogRequested: {
        request.accepted = true
        request.dialogReject()
    }
    onContextMenuRequested: {
        request.accepted = true
    }
    onColorDialogRequested: {
        request.accepted = true
        request.dialogReject()
    }
    onFileDialogRequested: {
        request.accepted = true
        request.dialogReject()
    }
    onJavaScriptDialogRequested: {
        request.accepted = true
        request.dialogReject()
    }

    property var replacer
    Component.onCompleted: replacer = new HSR.HtmlSelectReplacer(webengine)
    onLoadingChanged: {
        if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
            webengine.replacer.process()
        }
    }
}

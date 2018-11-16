// Copyright (c) 2018, EPFL/Blue Brain Project
//                     Raphael Dumusc <raphael.dumusc@epfl.ch>
import QtQuick 2.0
// Qt 5.6
import QtWebEngine 1.2

import "qrc:/web/js/HtmlSelectReplacer.js" as HSR

WebEngineView {
    id: webengine
    objectName: "webengineview"
    anchors.fill: parent

    onCertificateError: {
        error.ignoreCertificateError()
    }

    property var replacer
    Component.onCompleted: replacer = new HSR.HtmlSelectReplacer(webengine)
    onLoadingChanged: {
        if (loadRequest.status === WebEngineView.LoadSucceededStatus)
            webengine.replacer.process()
    }
}

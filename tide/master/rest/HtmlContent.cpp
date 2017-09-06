/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "HtmlContent.h"

#include <zeroeq/http/helpers.h>

#include <QFile>
#include <QMap>
#include <QStringList>

using namespace zeroeq;

namespace
{
const QString resourcePath = ":///html/";
// clang-format off
const QMap<std::string, QStringList> resources {
    {
        "application/javascript",
        {
            "js/bootstrap-treeview.min.js",
            "js/jquery-3.1.1.min.js",
            "js/notify.min.js",
            "js/jquery-ui.min.js",
            "js/sweetalert.min.js",
            "js/tide.js",
            "js/tideVars.js",
            "js/underscore-min.js"
        }
    },
    {
        "image/gif",
        {
            "img/loading.gif"
        }
    },
    {
        "image/png",
        {
            "img/screenOn.png",
            "img/screenOff.png"
        }
    },
    {
        "image/svg+xml",
        {
            "img/close.svg",
            "img/focus.svg",
            "img/lock.svg",
            "img/maximize.svg",
            "img/unlock.svg"
        }
    },
    {
        "text/css",
        {
            "css/bootstrap.min.css",
            "css/jquery-ui.css",
            "css/styles.css",
            "css/sweetalert.min.css"
        }
    },
    {
        "text/html",
        {
            "index.html"
        }
    }
};
// clang-format on

std::string _readFile(const QString& uri)
{
    QFile file(resourcePath + uri);
    file.open(QIODevice::ReadOnly);
    return file.readAll().toStdString();
}

std::future<http::Response> _makeResponse(const QString& file,
                                          const std::string& type)
{
    using namespace http;
    const auto body = _readFile(file);
    if (body.empty())
        return make_ready_response(Code::INTERNAL_SERVER_ERROR);
    return make_ready_response(Code::OK, body, type);
}
}

HtmlContent::HtmlContent(http::Server& server)
{
    server.handle(http::Method::GET, "", [](const http::Request&) {
        return _makeResponse("index.html", "text/html");
    });

    for (auto it = resources.begin(); it != resources.end(); ++it)
    {
        const auto& type = it.key();
        const auto& files = it.value();
        for (const auto& file : files)
        {
            server.handle(http::Method::GET, file.toStdString(),
                          [file, type](const http::Request&) {
                              return _makeResponse(file, type);
                          });
        }
    }
}

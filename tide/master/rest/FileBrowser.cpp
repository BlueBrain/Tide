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

#include "FileBrowser.h"

#include "json.h"

#include <QDir>
#include <QUrl>

namespace
{
QJsonObject _toJsonObject(const QFileInfo& entry)
{
    return QJsonObject{{"name", entry.fileName()}, {"dir", entry.isDir()}};
}

QJsonArray _toJsonArray(const QFileInfoList& list)
{
    QJsonArray array;
    for (const auto& entry : list)
        array.append(_toJsonObject(entry));
    return array;
}
}

FileBrowser::FileBrowser(const QString& baseDir, const QStringList& filters)
    : _baseDir{baseDir}
    , _filters{filters}
{
}

std::future<zeroeq::http::Response> FileBrowser::list(
    const zeroeq::http::Request& request)
{
    using namespace zeroeq::http;
    auto path = QString::fromStdString(request.path);
    QUrl url;
    url.setPath(path, QUrl::StrictMode);
    path = url.path();

    const QString fullpath = _baseDir + "/" + path;
    const QDir absolutePath(fullpath);
    if (!absolutePath.canonicalPath().startsWith(_baseDir))
        return make_ready_response(Code::BAD_REQUEST);

    if (!absolutePath.exists())
        return make_ready_response(Code::NO_CONTENT);

    const auto body = json::toString(_toJsonArray(_contents(fullpath)));
    return make_ready_response(Code::OK, body, "application/json");
}

QFileInfoList FileBrowser::_contents(const QDir& directory) const
{
    const auto filters = QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot;
    const auto sortFlags = QDir::DirsFirst | QDir::IgnoreCase;
    return directory.entryInfoList(_filters, filters, sortFlags);
}

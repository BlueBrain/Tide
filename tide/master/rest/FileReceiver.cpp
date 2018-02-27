/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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

#include "FileReceiver.h"

#include "log.h"
#include "scene/ContentFactory.h"
#include "json/json.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

using namespace rockets;

namespace
{
inline QString _urlEncode(const QString& filename)
{
    return QUrl(filename).fileName(QUrl::FullyEncoded);
}

inline QString _urlDecode(const QString& filename)
{
    return QUrl(filename).fileName(QUrl::FullyDecoded);
}

QString _getAvailableFileName(const QFileInfo& fileInfo)
{
    auto filename = fileInfo.fileName();

    int nSuffix = 0;
    while (QFile(QDir::tempPath() + "/" + filename).exists())
    {
        filename = QString("%1_%2.%3")
                       .arg(fileInfo.baseName(), QString::number(++nSuffix),
                            fileInfo.suffix());
    }
    return filename;
}

std::future<http::Response> _makeResponse(const http::Code code,
                                          const QString& key,
                                          const QString& info)
{
    const auto body = json::dump(QJsonObject{{key, info}});
    return make_ready_response(code, body, "application/json");
}
}

struct FileReceiver::UploadParameters
{
    UploadParameters() = default;
    UploadParameters(const QJsonObject& obj)
        : filename{obj["filename"].toString()}
        , surfaceIndex{static_cast<uint>(obj["surfaceIndex"].toInt())}
        , position{obj["x"].toDouble(), obj["y"].toDouble()}
    {
    }
    QString filename;
    uint surfaceIndex = 0;
    QPointF position;
};

FileReceiver::FileReceiver()
{
}

FileReceiver::~FileReceiver()
{
}

std::future<http::Response> FileReceiver::prepareUpload(
    const http::Request& request)
{
    const auto obj = json::parse(request.body);
    if (obj.empty())
        return make_ready_response(http::Code::BAD_REQUEST);

    const auto params = UploadParameters{obj};
    if (params.filename.contains('/'))
        return make_ready_response(http::Code::NOT_SUPPORTED);

    const auto fileInfo = QFileInfo(params.filename);
    const auto fileSuffix = fileInfo.suffix();
    if (fileSuffix.isEmpty() || fileInfo.baseName().isEmpty())
        return make_ready_response(http::Code::NOT_SUPPORTED);

    const auto& filters = ContentFactory::getSupportedExtensions();
    if (!filters.contains(fileSuffix.toLower()))
        return make_ready_response(http::Code::NOT_SUPPORTED);

    const auto name = _getAvailableFileName(fileInfo);
    _preparedPaths[name] = params;
    return _makeResponse(http::Code::OK, "url", _urlEncode(name));
}

std::future<http::Response> FileReceiver::handleUpload(
    const http::Request& request)
{
    const auto name = _urlDecode(QString::fromStdString(request.path));
    if (!_preparedPaths.count(name))
        return _makeResponse(http::Code::FORBIDDEN, "info",
                             "upload not prepared");

    const auto filePath = QDir::tempPath() + "/" + name;

    const auto params = _preparedPaths[name];
    _preparedPaths.erase(name);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly) ||
        !file.write(request.body.c_str(), request.body.size()))
    {
        print_log(LOG_ERROR, LOG_REST, "file not created as %s",
                  filePath.toLocal8Bit().constData());
        return _makeResponse(http::Code::INTERNAL_SERVER_ERROR, "info",
                             "could not upload");
    }
    file.close();

    print_log(LOG_INFO, LOG_REST, "file created as %s",
              filePath.toLocal8Bit().constData());

    auto promise = std::make_shared<std::promise<Response>>();
    emit open(
        params.surfaceIndex, filePath, params.position,
        [promise, filePath](const bool success) {
            if (success)
            {
                print_log(LOG_INFO, LOG_REST, "file uploaded and saved as: %s",
                          filePath.toLocal8Bit().constData());
                promise->set_value(Response{http::Code::CREATED, "info", "OK"});
                return;
            }

            QFile(filePath).remove();

            print_log(LOG_ERROR, LOG_REST,
                      "file uploaded but could not be opened: %s",
                      filePath.toLocal8Bit().constData());
            promise->set_value(Response{http::Code::NOT_SUPPORTED, "info",
                                        "file could not be opened"});
        });
    return promise->get_future();
}

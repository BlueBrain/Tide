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

#include "scene/ContentFactory.h"
#include "session/SessionSaver.h"
#include "utils/log.h"
#include "json/json.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

using namespace rockets;

namespace
{
constexpr auto JSON_TYPE = "application/json";
constexpr auto INFO_KEY = "info";
constexpr auto URL_KEY = "url";

inline QString _urlEncode(const QString& filename)
{
    return QUrl(filename).fileName(QUrl::FullyEncoded);
}

inline QString _urlDecode(const QString& filename)
{
    return QUrl(filename).fileName(QUrl::FullyDecoded);
}

std::string _toJson(const QString& key, const QString& value)
{
    return json::dump(QJsonObject{{key, value}});
}

http::Response _makeResponse(const http::Code code, const QString& key,
                             const QString& info)
{
    return http::Response{code, _toJson(key, info), JSON_TYPE};
}

std::future<http::Response> _makeReadyResponse(const http::Code code,
                                               const QString& key,
                                               const QString& info)
{
    return make_ready_response(_makeResponse(code, key, info));
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

FileReceiver::FileReceiver(const QString& tmpDir)
    : _tmpDir{tmpDir}
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

    const auto path =
        SessionSaver::findAvailableFilePath(params.filename, _tmpDir);
    const auto name = QFileInfo{path}.fileName();
    _preparedPaths[name] = params;
    return _makeReadyResponse(http::Code::OK, URL_KEY, _urlEncode(name));
}

std::future<http::Response> FileReceiver::handleUpload(
    const http::Request& request)
{
    const auto name = _urlDecode(QString::fromStdString(request.path));
    if (!_preparedPaths.count(name))
        return _makeReadyResponse(http::Code::FORBIDDEN, INFO_KEY,
                                  "upload not prepared");

    const auto filePath = _tmpDir + "/" + name;

    const auto params = _preparedPaths[name];
    _preparedPaths.erase(name);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly) ||
        !file.write(request.body.c_str(), request.body.size()))
    {
        print_log(LOG_ERROR, LOG_REST, "file not created as %s",
                  filePath.toLocal8Bit().constData());
        return _makeReadyResponse(http::Code::INTERNAL_SERVER_ERROR, INFO_KEY,
                                  "could not upload");
    }
    file.close();

    print_log(LOG_INFO, LOG_REST, "file created as %s",
              filePath.toLocal8Bit().constData());

    auto promise = std::make_shared<std::promise<Response>>();
    emit open(params.surfaceIndex, filePath, params.position,
              [promise, filePath](const bool success) {
                  if (success)
                  {
                      print_log(LOG_INFO, LOG_REST,
                                "file uploaded and saved as: %s",
                                filePath.toLocal8Bit().constData());
                      promise->set_value(
                          _makeResponse(http::Code::CREATED, INFO_KEY, "OK"));
                      return;
                  }

                  QFile(filePath).remove();

                  print_log(LOG_ERROR, LOG_REST,
                            "file uploaded but could not be opened: %s",
                            filePath.toLocal8Bit().constData());
                  promise->set_value(_makeResponse(http::Code::NOT_SUPPORTED,
                                                   INFO_KEY,
                                                   "file could not be opened"));
              });
    return promise->get_future();
}

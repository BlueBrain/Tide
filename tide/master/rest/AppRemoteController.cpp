/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
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

#include "AppRemoteController.h"

#include "config.h"
#include "configuration/Configuration.h"
#include "json/json.h"
#include "json/serialization.h"

#include <QDir>
#include <QJsonObject>

using namespace rockets;

namespace
{
QString _makeAbsPath(const QString& baseDir, const QString& uri)
{
    return QDir::isRelativePath(uri) ? baseDir + "/" + uri : uri;
}
} // anonymous namespace

template <typename Obj>
bool from_json(Obj& object, const std::string& json)
{
    return object.fromJson(json::parse(json));
}

struct Uri
{
    QString uri;

    bool fromJson(const QJsonObject& object)
    {
        json::deserialize(object["uri"], uri);
        return !uri.isNull();
    }
};

struct SurfaceIndex
{
    uint surfaceIndex = 0;

    bool fromJson(const QJsonObject& object)
    {
        json::deserialize(object["surfaceIndex"], surfaceIndex);
        return true;
    }
};

struct UriAndSurface : Uri, SurfaceIndex
{
    bool fromJson(const QJsonObject& object)
    {
        SurfaceIndex::fromJson(object);
        return Uri::fromJson(object);
    }
};

struct BrowseParams : UriAndSurface
{
    bool fromJson(const QJsonObject& object)
    {
        UriAndSurface::fromJson(object);
        return true;
    }
};

jsonrpc::Response makeJsonRpcResponse(const bool success)
{
    return success ? jsonrpc::Response{"\"OK\""}
                   : jsonrpc::Response{
                         jsonrpc::Response::Error{"operation failed", -1}};
}

AppRemoteController::AppRemoteController(const Configuration& config)
{
    const auto& contentsDir = config.folders.contents;
    const auto& sessionsDir = config.folders.sessions;
    const auto& defaultUrl = config.webbrowser.defaultUrl;

    // WAR: GCC 5.4 needs "this->" to emit signals from lambdas with auto params
    bindAsync<UriAndSurface>(
        "open",
        [this, contentsDir](const auto params, jsonrpc::AsyncResponse respond) {
            auto boolCallback = [respond](const bool result) {
                respond(makeJsonRpcResponse(result));
            };
            emit this->open(params.surfaceIndex,
                            _makeAbsPath(contentsDir, params.uri), QPointF(),
                            boolCallback);
        });
    bindAsync<Uri>("load", [this, sessionsDir](const auto params,
                                               jsonrpc::AsyncResponse respond) {
        auto boolCallback = [respond](const bool result) {
            respond(makeJsonRpcResponse(result));
        };
        emit this->load(_makeAbsPath(sessionsDir, params.uri), boolCallback);
    });
    bindAsync<Uri>("save", [this, sessionsDir](const auto params,
                                               jsonrpc::AsyncResponse respond) {
        auto boolCallback = [respond](const bool result) {
            respond(makeJsonRpcResponse(result));
        };
        emit this->save(_makeAbsPath(sessionsDir, params.uri), boolCallback);
    });

    using rpc = jsonrpc::Receiver; // Disambiguating from QObject::connect
    rpc::connect<BrowseParams>("browse", [this, defaultUrl](auto params) {
        if (params.uri.isEmpty())
            params.uri = defaultUrl;
        emit this->browse(params.surfaceIndex, params.uri, QSize(), QPointF(),
                          0);
    });
    rpc::connect<UriAndSurface>("screenshot", [this](const auto params) {
        emit this->takeScreenshot(params.surfaceIndex, params.uri);
    });
    rpc::connect<SurfaceIndex>("whiteboard", [this](const auto params) {
        emit this->openWhiteboard(params.surfaceIndex);
    });
    rpc::connect("exit", [this] { emit this->exit(); });
#if TIDE_ENABLE_PLANAR_CONTROLLER
    bindAsync("poweroff",
              [this](const jsonrpc::Request&, jsonrpc::AsyncResponse respond) {
                  auto boolCallback = [respond](const bool result) {
                      respond(makeJsonRpcResponse(result));
                  };
                  emit this->powerOff(boolCallback);
              });
    bindAsync("poweron",
              [this](const jsonrpc::Request&, jsonrpc::AsyncResponse respond) {
                  auto boolCallback = [respond](const bool result) {
                      respond(makeJsonRpcResponse(result));
                  };
                  emit this->powerOn(boolCallback);
              });
#endif
}

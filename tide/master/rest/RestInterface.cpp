/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include "RestInterface.h"

#include "FileBrowser.h"
#include "FileReceiver.h"
#include "HtmlContent.h"
#include "LoggingUtility.h"
#include "MasterConfiguration.h"
#include "RestServer.h"
#include "SceneController.h"
#include "ScreenLock.h"
#include "ThumbnailCache.h"
#include "scene/ContentFactory.h"
#include "serialization.h"

#include <tide/master/version.h>

#include <QDir>

using namespace std::placeholders;
using namespace zeroeq;

namespace
{
template <typename T>
std::future<http::Response> processJsonRpc(T* controller,
                                           const http::Request& request)
{
    // Package json-rpc response in http::Response when ready
    auto promise = std::make_shared<std::promise<http::Response>>();
    auto callback = [promise](const std::string& body) {
        if (body.empty())
            promise->set_value(http::Response{http::Code::OK});
        else
            promise->set_value(
                http::Response{http::Code::OK, body, "application/json"});
    };
    controller->processJsonRpc(request.body, callback);
    return promise->get_future();
}
}

struct LockState
{
    bool locked;
};
QJsonObject to_json_object(const LockState& lockState)
{
    return QJsonObject{{"locked", lockState.locked}};
}

/** Overload to serialize QSize as an array instead of an object. */
std::string to_json(const QSize& size)
{
    return json::toString(QJsonArray{{size.width(), size.height()}});
}

namespace tide
{
std::string to_json(const Version& object)
{
    return object.toJSON();
}
}

class RestInterface::Impl
{
public:
    Impl(const uint16_t port, OptionsPtr options_, DisplayGroup& group,
         const MasterConfiguration& config, const bool locked)
        : server{port}
        , options{options_}
        , size{config.getTotalSize()}
        , thumbnailCache{group}
        , appController{config}
        , sceneController{group}
        , contentBrowser{config.getContentDir(),
                         ContentFactory::getSupportedFilesFilter()}
        , sessionBrowser{config.getSessionsDir(), QStringList{"*.dcx"}}
        , htmlContent{server}
        , lockState{locked}
    {
    }

    std::future<http::Response> getWindowInfo(
        const http::Request& request) const
    {
        const auto path = QString::fromStdString(request.path);
        if (path.endsWith("/thumbnail"))
        {
            const auto pathSplit = path.split("/");
            if (pathSplit.size() == 2 && pathSplit[1] == "thumbnail")
                return thumbnailCache.getThumbnail(url_decode(pathSplit[0]));
        }
        return make_ready_response(http::Code::BAD_REQUEST);
    }

    RestServer server;
    OptionsPtr options;
    QSize size;
    ThumbnailCache thumbnailCache;
    AppController appController;
    SceneController sceneController;
    FileBrowser contentBrowser;
    FileBrowser sessionBrowser;
    FileReceiver fileReceiver;
    HtmlContent htmlContent;
    LockState lockState;
};

RestInterface::RestInterface(const uint16_t port, OptionsPtr options,
                             DisplayGroup& group,
                             const MasterConfiguration& config)
    : _impl(new Impl(port, options, group, config, false))
{
    // Note: using same formatting as TUIO instead of put_flog() here
    std::cout << "listening to REST messages on TCP port "
              << _impl->server.getPort() << std::endl;

    QObject::connect(&_impl->fileReceiver, &FileReceiver::open,
                     &_impl->appController, &AppController::open);

    auto& server = _impl->server;

    static tide::Version version;
    server.handleGET("tide/version", version);
    server.handleGET("tide/config", config);
    server.handleGET("tide/lock", _impl->lockState);
    server.handleGET("tide/size", _impl->size);
    server.handleGET("tide/options", *_impl->options);
    server.handlePUT("tide/options", *_impl->options);
    server.handleGET("tide/windows", group);

    server.handle(http::Method::GET, "tide/windows/",
                  std::bind(&Impl::getWindowInfo, _impl.get(), _1));

    server.handle(http::Method::POST, "tide/application",
                  std::bind(processJsonRpc<AppController>,
                            &_impl->appController, _1));

    server.handle(http::Method::POST, "tide/controller",
                  std::bind(processJsonRpc<SceneController>,
                            &_impl->sceneController, _1));

    server.handle(http::Method::POST, "tide/upload",
                  std::bind(&FileReceiver::prepareUpload, &_impl->fileReceiver,
                            _1));

    server.handle(http::Method::PUT, "tide/upload/",
                  std::bind(&FileReceiver::handleUpload, &_impl->fileReceiver,
                            _1));

    server.handle(http::Method::GET, "tide/files/",
                  std::bind(&FileBrowser::list, &_impl->contentBrowser, _1));

    server.handle(http::Method::GET, "tide/sessions/",
                  std::bind(&FileBrowser::list, &_impl->sessionBrowser, _1));
}

RestInterface::~RestInterface()
{
}

void RestInterface::exposeStatistics(const LoggingUtility& logger) const
{
    _impl->server.handleGET("tide/stats", logger);
}

const AppController& RestInterface::getAppController() const
{
    return _impl->appController;
}

void RestInterface::lock(const bool lock_)
{
    _impl->lockState.locked = lock_;
    if (lock_)
    {
        _impl->server.block(http::Method::PUT);
        _impl->server.block(http::Method::POST);
    }
    else
    {
        _impl->server.unblock(http::Method::POST);
        _impl->server.unblock(http::Method::PUT);
    }
}

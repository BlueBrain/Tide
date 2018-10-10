/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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
#include "RestServer.h"
#include "SceneRemoteController.h"
#include "ThumbnailCache.h"
#include "configuration/Configuration.h"
#include "rest/serialization.h"
#include "scene/ContentFactory.h"
#include "scene/Scene.h"
#include "session/Session.h"
#include "tools/ActivityLogger.h"
#include "utils/log.h"
#include "json/serialization.h"
// include last
#include "rest/templates.h"

#include <tide/master/version.h>

#include <rockets/jsonrpc/http.h>

using namespace rockets;

namespace tide
{
std::string to_json(const Version& version)
{
    return version.toJSON();
}
}

namespace
{
struct LockState
{
    bool locked;
};
std::string to_json(const LockState& lockState)
{
    return json::dump(QJsonObject{{"locked", lockState.locked}});
}
}

/** Use REST-specific serialization of Configuration. */
std::string to_json(const Configuration& config)
{
    return json::dump(json::serializeForRest(config));
}

class RestInterface::Impl
{
public:
    Impl(const uint16_t port, OptionsPtr options_, Scene& scene,
         const Configuration& config, const bool locked)
        : server{port}
        , options{options_}
        , size{config.surfaces[0].getTotalSize()}
        , thumbnailCache{scene}
        , appRemoteController{config}
        , sceneRemoteController{scene}
        , contentBrowser{config.folders.contents,
                         ContentFactory::getSupportedFilesFilter()}
        , sessionBrowser{config.folders.sessions, QStringList{"*.dcx"}}
        , fileReceiver{config.folders.tmp}
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
            {
                const auto uuid = json::url_decode(pathSplit[0]);
                return thumbnailCache.getThumbnail(uuid);
            }
        }
        return make_ready_response(http::Code::BAD_REQUEST);
    }

    RestServer server;
    OptionsPtr options;
    QSize size;
    ThumbnailCache thumbnailCache;
    AppRemoteController appRemoteController;
    SceneRemoteController sceneRemoteController;
    FileBrowser contentBrowser;
    FileBrowser sessionBrowser;
    FileReceiver fileReceiver;
    HtmlContent htmlContent;
    LockState lockState;
};

RestInterface::RestInterface(const uint16_t port, OptionsPtr options,
                             Session& session, Configuration& config)
    : _impl(new Impl(port, options, *session.getScene(), config, false))
{
    put_log(LOG_INFO, LOG_REST, "listening to REST messages on TCP port %hu",
            _impl->server.getPort());

    QObject::connect(&_impl->fileReceiver, &FileReceiver::open,
                     &_impl->appRemoteController, &AppRemoteController::open);

    auto& server = _impl->server;

    jsonrpc::connect(server, "tide/application", _impl->appRemoteController);
    jsonrpc::connect(server, "tide/controller", _impl->sceneRemoteController);

    static tide::Version version;
    server.handleGET("tide/version", version);
    server.handleGET("tide/background", *config.surfaces[0].background);
    server.handlePUT("tide/background", *config.surfaces[0].background);
    server.handleGET("tide/config", config);
    server.handleGET("tide/lock", _impl->lockState);
    server.handleGET("tide/size", _impl->size);
    server.handleGET("tide/options", *_impl->options);
    server.handlePUT("tide/options", *_impl->options);
    server.handleGET("tide/windows", *session.getScene());
    server.handleGET("tide/session", session.getInfo());

    using namespace std::placeholders;

    server.handle(http::Method::GET, "tide/windows/",
                  std::bind(&Impl::getWindowInfo, _impl.get(), _1));

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

RestInterface::~RestInterface() = default;

void RestInterface::exposeStatistics(const ActivityLogger& logger) const
{
    _impl->server.handleGET("tide/stats", logger);
}

const AppRemoteController& RestInterface::getAppRemoteController() const
{
    return _impl->appRemoteController;
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

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

#include "FileReceiver.h"
#include "FileSystemQuery.h"
#include "HtmlContent.h"
#include "JsonOptions.h"
#include "json.h"
#include "MasterConfiguration.h"
#include "RestCommand.h"
#include "RestConfiguration.h"
#include "RestController.h"
#include "RestLogger.h"
#include "RestServer.h"
#include "RestWindows.h"
#include "scene/ContentFactory.h"

#include <tide/master/version.h>

#include <zeroeq/http/helpers.h>

#include <functional>
#include <QDir>

using namespace std::placeholders;
using namespace zeroeq;

namespace
{
struct HtmlInterface
{
    HtmlInterface( http::Server& server, DisplayGroup& group,
                   const MasterConfiguration& config )
        : htmlContent{ server }
        , jsonConfig{ config }
        , windowsContent{ group }
        , sceneController{ server, group }
        , contentDirQuery{ config.getContentDir(),
                           ContentFactory::getSupportedFilesFilter() }
        , sessionDirQuery{ config.getSessionsDir(), QStringList{ "*.dcx" }}
    {
        server.handleGET( jsonConfig );

        server.handle( http::Method::GET, "tide/windows",
                       std::bind( &RestWindows::getWindowList, &windowsContent,
                                  std::placeholders::_1 ) );

        server.handle( http::Method::GET, "tide/windows/",
                       std::bind( &RestWindows::getWindowInfo, &windowsContent,
                                  std::placeholders::_1 ));

        server.handle( http::Method::POST, "tide/upload",
                       std::bind( &FileReceiver::prepareUpload, &fileReceiver,
                                  std::placeholders::_1 ));

        server.handle( http::Method::PUT, "tide/upload/",
                       std::bind( &FileReceiver::handleUpload, &fileReceiver,
                                  std::placeholders::_1 ));

        server.handle( http::Method::GET, "tide/files/",
                       std::bind( &FileSystemQuery::list, &contentDirQuery,
                                  std::placeholders::_1 ));

        server.handle( http::Method::GET, "tide/sessions/",
                       std::bind( &FileSystemQuery::list, &sessionDirQuery,
                                  std::placeholders::_1 ));
    }

    HtmlContent htmlContent;
    RestConfiguration jsonConfig;
    RestWindows windowsContent;
    RestController sceneController;
    FileSystemQuery contentDirQuery;
    FileSystemQuery sessionDirQuery;
    FileReceiver fileReceiver;
};

using AsyncAction = std::function<void(QString, promisePtr)>;
std::future<http::Response> _handleUriRequest( const http::Request& request,
                                               AsyncAction action,
                                               const QString& baseDir )
{
    const auto obj = json::toObject( request.body );
    if( obj.empty() || !obj["uri"].isString( ))
        return http::make_ready_future( http::Response{
                                            http::Code::BAD_REQUEST });

    auto uri = obj["uri"].toString();
    if( QDir::isRelativePath( uri ))
        uri = baseDir + "/" + uri;

    return std::async( std::launch::deferred, [action, uri]()
    {
        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();

        action( uri, std::move( promise ));

        if( !future.get( ))
            return http::Response{ http::Code::INTERNAL_SERVER_ERROR };
        return http::Response{ http::Code::OK };
    });
}
}

class RestInterface::Impl
{
public:
    Impl( const int port, OptionsPtr options,
          const MasterConfiguration& config_ )
        : httpServer{ port }
        , jsonOptions{ options }
        , jsonSize{ config_.getTotalSize() }
    {
        auto& server = httpServer.get();
        server.handleGET( "tide/version", tide::Version::getSchema(),
                          &tide::Version::toJSON );
        server.handlePUT( browseCmd );
        server.handlePUT( clearCmd );
        server.handlePUT( exitCmd );
        server.handlePUT( screenshotCmd );
        server.handlePUT( whiteboardCmd );

        server.handle( jsonOptions );

        server.handleGET( jsonSize );
    }

    RestServer httpServer;

    RestCommand browseCmd{ "tide/browse" };
    RestCommand clearCmd{ "tide/clear", false };
    RestCommand exitCmd{ "tide/exit", false };
    RestCommand screenshotCmd{ "tide/screenshot" };
    RestCommand whiteboardCmd{ "tide/whiteboard", false };

    JsonOptions jsonOptions;
    JsonSize jsonSize;

    std::unique_ptr<RestLogger> logContent; // Statistics (optional)
    std::unique_ptr<HtmlInterface> htmlInterface; // HTML interface (optional)
};

RestInterface::RestInterface( const int port, OptionsPtr options,
                              const MasterConfiguration& config )
    : _impl( new Impl( port, options, config ))
{
    // Note: using same formatting as TUIO instead of put_flog() here
    std::cout << "listening to REST messages on TCP port " <<
                 _impl->httpServer.getPort() << std::endl;

    connect( &_impl->browseCmd, &RestCommand::received,
             this, &RestInterface::browse );

    connect( &_impl->clearCmd, &RestCommand::received,
             this, &RestInterface::clear );

    connect( &_impl->exitCmd, &RestCommand::received,
             this, &RestInterface::exit );

    connect( &_impl->screenshotCmd, &RestCommand::received,
             this, &RestInterface::screenshot );

    connect( &_impl->whiteboardCmd, &RestCommand::received,
             this, &RestInterface::whiteboard );

    const auto contentDir = config.getContentDir();
    const auto sessionsDir = config.getSessionsDir();

    const auto openFunc = std::bind( &RestInterface::open, this, _1, QPointF(),
                                     _2 );
    const auto loadFunc = std::bind( &RestInterface::load, this, _1, _2 );
    const auto saveFunc = std::bind( &RestInterface::save, this, _1, _2 );

    auto& server = _impl->httpServer.get();

    server.handle( http::Method::PUT, "tide/open",
                   [openFunc, contentDir]( const http::Request& request )
    {
        return _handleUriRequest( request, openFunc, contentDir );
    });

    server.handle( http::Method::PUT, "tide/load",
                   [loadFunc, sessionsDir]( const http::Request& request )
    {
        return _handleUriRequest( request, loadFunc, sessionsDir );
    });

    server.handle( http::Method::PUT, "tide/save",
                   [saveFunc, sessionsDir]( const http::Request& request )
    {
        return _handleUriRequest( request, saveFunc, sessionsDir );
    });
}

RestInterface::~RestInterface() {}

void RestInterface::exposeStatistics( const LoggingUtility& logger ) const
{
    _impl->logContent.reset( new RestLogger( logger ));
    _impl->httpServer.get().handleGET( *_impl->logContent );
}

void RestInterface::setupHtmlInterface( DisplayGroup& group,
                                        const MasterConfiguration& config )
{
    auto& server = _impl->httpServer.get();
    _impl->htmlInterface.reset( new HtmlInterface( server, group, config ));

    connect( &_impl->htmlInterface->fileReceiver, &FileReceiver::open,
             this, &RestInterface::open );
}

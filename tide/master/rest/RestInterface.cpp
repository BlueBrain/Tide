/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include "RestInterface.h"

#include "FileSystemQuery.h"
#include "HtmlContent.h"
#include "JsonOptions.h"
#include "MasterConfiguration.h"
#include "FileReceiver.h"
#include "RestCommand.h"
#include "RestConfiguration.h"
#include "RestController.h"
#include "RestLogger.h"
#include "RestServer.h"
#include "RestWindows.h"
#include "StaticContent.h"

#include "scene/ContentFactory.h"

#include <tide/master/version.h>

#include <functional>
#include <QDir>

class RestInterface::Impl
{
public:
    Impl( const int port, OptionsPtr options_,
          const MasterConfiguration& config )
        : httpServer{ port }
        , options{ options_ }
        , sizeProperty{ config.getTotalSize() }
    {
        auto& server = httpServer.get();
        server.handleGET( "tide/version", tide::Version::getSchema(),
                          &tide::Version::toJSON );
        server.handlePUT( browseCmd );
        server.handlePUT( closeCmd );
        server.handlePUT( whiteboardCmd );
        server.handlePUT( screenshotCmd );
        server.handlePUT( exitCmd );
        server.handle( options );
        server.handleGET( sizeProperty );
    }

    RestServer httpServer;
    RestCommand browseCmd{ "tide/browse" };
    RestCommand closeCmd{ "tide/close" };
    RestCommand whiteboardCmd{ "tide/whiteboard", false };
    RestCommand screenshotCmd{ "tide/screenshot" };
    RestCommand exitCmd{ "tide/exit", false };

    JsonOptions options;
    JsonSize sizeProperty;

    std::unique_ptr<FileSystemQuery> contentDirQuery;
    std::unique_ptr<FileSystemQuery> sessionDirQuery;
    std::unique_ptr<FileReceiver> fileReceiver;
    std::unique_ptr<RestLogger> logContent;
    std::unique_ptr<HtmlContent> htmlContent;
    std::unique_ptr<RestController> sceneController;
    std::unique_ptr<RestWindows> windowsContent;
    std::unique_ptr<RestConfiguration> configurationContent;
};

RestInterface::RestInterface( const int port, OptionsPtr options,
                              const MasterConfiguration& config )
    : _impl( new Impl( port, options, config ))
    , _config( config )
{
    // Note: using same formatting as TUIO instead of put_flog() here
    std::cout << "listening to REST messages on TCP port " <<
                 _impl->httpServer.getPort() << std::endl;

    connect( &_impl->browseCmd, &RestCommand::received,
             this, &RestInterface::browse );

    connect( &_impl->whiteboardCmd, &RestCommand::received,
             this, &RestInterface::whiteboard );

    connect( &_impl->screenshotCmd, &RestCommand::received,
             this, &RestInterface::screenshot );

    connect( &_impl->exitCmd, &RestCommand::received,
             this, &RestInterface::exit );

    connect( &_impl->closeCmd, &RestCommand::received,
             this, &RestInterface::close );
}

RestInterface::~RestInterface() {}

void RestInterface::exposeStatistics( const LoggingUtility& logger ) const
{
    _impl->logContent.reset( new RestLogger( logger ));
    _impl->httpServer.get().handleGET( *_impl->logContent );
}

void RestInterface::setupHtmlInterface( DisplayGroup& displayGroup,
                                   const MasterConfiguration& config )
{
    _impl->htmlContent.reset( new HtmlContent( _impl->httpServer.get( )));

    _impl->windowsContent.reset( new RestWindows( displayGroup ));

    auto getWindowInfoFunc = std::bind( &RestWindows::getWindowInfo,
                                        _impl->windowsContent.get(),
                                        std::placeholders::_1 );

    _impl->httpServer.get().handle( zeroeq::http::Method::GET, "tide/windows/",
                                    getWindowInfoFunc );

    auto getWindowList = std::bind( &RestWindows::getWindowList,
                                    _impl->windowsContent.get(),
                                    std::placeholders::_1 );

    _impl->httpServer.get().handle( zeroeq::http::Method::GET, "tide/windows",
                                    getWindowList );

    _impl->configurationContent.reset( new RestConfiguration( config ));

    _impl->sceneController.reset( new RestController( _impl->httpServer.get(),
                                                      displayGroup,
                                                      _config ));

    _impl->fileReceiver.reset( new FileReceiver( ));

    auto prepareUpload = std::bind( &FileReceiver::prepareUpload,
                                    _impl->fileReceiver.get(),
                                    std::placeholders::_1 );

    _impl->httpServer.get().handle( zeroeq::http::Method::POST, "tide/upload",
                                    prepareUpload );

    auto handleUpload = std::bind( &FileReceiver::handleUpload,
                                   _impl->fileReceiver.get(),
                                   std::placeholders::_1 );

    _impl->httpServer.get().handle( zeroeq::http::Method::PUT, "tide/upload/",
                                    handleUpload );

    connect( _impl->fileReceiver.get(), &FileReceiver::open,
             this, &RestInterface::open );

    connect( _impl->sceneController.get(), &RestController::save,
             this, &RestInterface::save );

    connect( _impl->sceneController.get(), &RestController::load,
             this, &RestInterface::load );

    connect( _impl->sceneController.get(), &RestController::open,
             this, &RestInterface::open );

    const auto& fileFilters = ContentFactory::getSupportedFilesFilter( );

    _impl->contentDirQuery.reset( new FileSystemQuery( _config.getContentDir(),
                                                       fileFilters ));

    auto browseFilesFunc = std::bind( &FileSystemQuery::list,
                                      _impl->contentDirQuery.get(),
                                      std::placeholders::_1 );

    _impl->httpServer.get().handle( zeroeq::http::Method::GET, "tide/files/",
                                    browseFilesFunc );

    const QStringList sessionFilters{ "*.dcx" };
    _impl->sessionDirQuery.reset( new FileSystemQuery( _config.getSessionsDir(),
                                                       sessionFilters ));

    auto browseSessionsFunc = std::bind( &FileSystemQuery::list,
                                         _impl->sessionDirQuery.get(),
                                         std::placeholders::_1 );

    _impl->httpServer.get().handle( zeroeq::http::Method::GET,
                                    "tide/sessions/", browseSessionsFunc );

    _impl->httpServer.get().handleGET( *_impl->configurationContent );

}

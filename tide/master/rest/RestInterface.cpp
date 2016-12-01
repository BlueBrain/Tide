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

#include "JsonOptions.h"
#include "MasterConfiguration.h"
#include "RestCommand.h"
#include "RestLogger.h"
#include "RestServer.h"
#include "StaticContent.h"

#include <tide/master/version.h>

#include <QDateTime>
#include <QHostInfo>

namespace
{
const auto indexpage = QString(R"(
<!DOCTYPE html>
<html>
<head>
<meta charset='UTF-8'>
<title>Tide</title>
</head>
<body>
<h1>Tide %1</h1>
<p>Revision: <a href='https://github.com/BlueBrain/Tide/commit/%3'>%3</a></p>
<p>Running on: %2</p>
<p>Up since: %4</p>
</body>
</html>
)").arg( QString::fromStdString( tide::Version::getString( )),
         QHostInfo::localHostName(),
         QString::number( tide::Version::getRevision(), 16 ),
         QDateTime::currentDateTime().toString( )).toStdString();
}

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
        server.handleGET( indexPage );
        server.handlePUT( browseCmd );
        server.handlePUT( openCmd );
        server.handlePUT( loadCmd );
        server.handlePUT( saveCmd );
        server.handlePUT( whiteboardCmd );
        server.handlePUT( screenshotCmd );
        server.handlePUT( exitCmd );
        server.handle( options );
        server.handleGET( sizeProperty );
    }

    RestServer httpServer;
    StaticContent indexPage{ "tide", indexpage };
    RestCommand browseCmd{ "tide/browse" };
    RestCommand openCmd{ "tide/open" };
    RestCommand loadCmd{ "tide/load" };
    RestCommand saveCmd{ "tide/save" };
    RestCommand whiteboardCmd{ "tide/whiteboard", false };
    RestCommand screenshotCmd{ "tide/screenshot" };
    RestCommand exitCmd{ "tide/exit", false };
    JsonOptions options;
    JsonSize sizeProperty;
    std::unique_ptr<RestLogger> logContent;
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

    connect( &_impl->whiteboardCmd, &RestCommand::received,
             this, &RestInterface::whiteboard );

    connect( &_impl->openCmd, &RestCommand::received,
             this, &RestInterface::open );

    connect( &_impl->loadCmd, &RestCommand::received, [this](const QString uri)
    {
        if( uri.isEmpty( ))
            emit clear();
        else
            emit load( uri );
    });

    connect( &_impl->saveCmd, &RestCommand::received,
             this, &RestInterface::save );

    connect( &_impl->screenshotCmd, &RestCommand::received,
             this, &RestInterface::screenshot );

    connect( &_impl->exitCmd, &RestCommand::received,
             this, &RestInterface::exit );
}

RestInterface::~RestInterface() {}

void RestInterface::exposeStatistics( const LoggingUtility& logger ) const
{
    _impl->logContent.reset( new RestLogger( logger ));
    _impl->httpServer.get().handleGET( *(_impl->logContent.get( )));
}

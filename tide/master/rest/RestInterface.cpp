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

#include "RestCommand.h"

#include <zeroeq/http/server.h>
#include <zeroeq/uri.h>

#include <QSocketNotifier>

namespace
{
const uint32_t RECEIVE_TIMEOUT = 0; // non-blocking receive
}

class RestInterface::Impl
{
public:
    Impl( const int port )
        : httpServer{ zeroeq::URI { QString(":%1").arg( port ).toStdString( )}}
    {
        httpServer.subscribe( openCmd );
        httpServer.subscribe( loadCmd );
        httpServer.subscribe( saveCmd );
    }

    zeroeq::http::Server httpServer;
    QSocketNotifier socketNotifier{ httpServer.getSocketDescriptor(),
                                    QSocketNotifier::Read };
    RestCommand openCmd{ "tide::open" };
    RestCommand loadCmd{ "tide::load" };
    RestCommand saveCmd{ "tide::save" };
};

RestInterface::RestInterface( const int port )
    : _impl( new Impl( port ))
{
    connect( &_impl->socketNotifier, &QSocketNotifier::activated, [this]()
    {
        _impl->httpServer.receive( RECEIVE_TIMEOUT );
    });

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
}

RestInterface::~RestInterface() {}

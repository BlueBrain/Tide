/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#include "Application.h"

#include "localstreamer/PixelStreamer.h"
#include "localstreamer/PixelStreamerFactory.h"

#include "localstreamer/CommandLineOptions.h"

#include <QTimer>
#include <QNetworkProxy>

#include <iostream>

#define TIDE_STREAM_HOST_ADDRESS "localhost"
//#define COMPRESS_IMAGES

Application::Application( int &argc_, char** argv_ )
    : QApplication( argc_, argv_ )
    , _pixelStreamer( 0 )
    , _deflectStream( 0 )
{
    // Correctly setup the proxy from the 'http_proxy' environment variable
    const QUrl url( qgetenv( "http_proxy" ).constData( ));
    if( url.scheme() == "http" )
    {
        QNetworkProxy proxy( QNetworkProxy::HttpProxy, url.host(), url.port( ));
        QNetworkProxy::setApplicationProxy( proxy );
    }
}

Application::~Application()
{
    delete _deflectStream;
    delete _pixelStreamer;
}

bool Application::initialize( const CommandLineOptions& options )
{
    // Create the streamer
    _pixelStreamer = PixelStreamerFactory::create( options );
    if( !_pixelStreamer)
        return false;
    connect( _pixelStreamer, &PixelStreamer::imageUpdated,
             this, &Application::sendImage );
    connect( _pixelStreamer, &PixelStreamer::stateChanged,
             this, &Application::sendData );

    // Connect to Tide
    _deflectStream = new deflect::Stream( options.getStreamname().toStdString(),
                                          TIDE_STREAM_HOST_ADDRESS );
    if( !_deflectStream->isConnected( ))
    {
        std::cerr << "Could not connect to host!" << std::endl;
        delete _deflectStream;
        _deflectStream = 0;
        return false;
    }
    _deflectStream->registerForEvents();

    // Make sure to quit the application if the connection is closed.
    _deflectStream->disconnected.connect( QApplication::quit );

    // Use a timer to process Event received from the deflect::Stream
    QTimer* timer = new QTimer( this );
    connect( timer, SIGNAL( timeout( )), SLOT( processPendingEvents( )));
    timer->start( 1 );

    return true;
}

void Application::sendImage(QImage image)
{
#ifdef COMPRESS_IMAGES
    // QImage Format_RGB32 (0xffRRGGBB) corresponds in fact to
    // GL_BGRA == deflect::BGRA
    deflect::ImageWrapper deflectImage( (const void*)image.bits(),
                                        image.width(), image.height(),
                                        deflect::BGRA );
    deflectImage.compressionPolicy = deflect::COMPRESSION_ON;
#else
    // This conversion is suboptimal, but the only solution until we send the
    // PixelFormat with the PixelStreamSegment
    image = image.rgbSwapped();
    deflect::ImageWrapper deflectImage( (const void*)image.bits(),
                                        image.width(), image.height(),
                                        deflect::RGBA );
    deflectImage.compressionPolicy = deflect::COMPRESSION_OFF;
#endif
    const bool success = _deflectStream->send( deflectImage ) &&
                         _deflectStream->finishFrame();
    if( !success )
    {
        QApplication::quit();
        return;
    }
}

void Application::sendData( const QByteArray data )
{
    if( !_deflectStream->sendData( data.constData(), data.size( )))
        QApplication::quit();
}

void Application::processPendingEvents()
{
    while( _deflectStream->hasEvent( ))
        _pixelStreamer->processEvent( _deflectStream->getEvent( ));
}

/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#include "PixelStreamWindowManager.h"

#include "ContentFactory.h"
#include "ContentWindow.h"
#include "control/ContentController.h"
#include "control/ContentWindowController.h"
#include "control/DisplayGroupController.h"
#include "DisplayGroup.h"
#include "localstreamer/PixelStreamerLauncher.h"
#include "log.h"
#include "PixelStreamContent.h"

#include <deflect/EventReceiver.h>
#include <deflect/Frame.h>

namespace
{
const QSize EMPTY_STREAM_SIZE( 640, 480 );
}

PixelStreamWindowManager::PixelStreamWindowManager( DisplayGroup& displayGroup )
    : QObject()
    , _displayGroup( displayGroup )
    , _autoFocusNewWindows( true )
{
    connect( &displayGroup, SIGNAL( contentWindowRemoved( ContentWindowPtr )),
             this, SLOT( onContentWindowRemoved( ContentWindowPtr )));
}

ContentWindowPtr
PixelStreamWindowManager::getContentWindow( const QString& uri ) const
{
    ContentWindowMap::const_iterator it = _streamerWindows.find( uri );
    return it != _streamerWindows.end() ?
                     _displayGroup.getContentWindow( it->second ) :
                     ContentWindowPtr();
}

void PixelStreamWindowManager::hideWindow( const QString& uri )
{
    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        contentWindow->setState( ContentWindow::HIDDEN );
}

void PixelStreamWindowManager::showWindow( const QString& uri )
{
    ContentWindowPtr window = getContentWindow( uri );
    if( !window )
        return;

    window->setState( ContentWindow::NONE );
    _displayGroup.moveContentWindowToFront( window );
}

void PixelStreamWindowManager::openWindow( const QString& uri,
                                           const QPointF& pos,
                                           const QSize& size,
                                           const bool webbrowser )
{
    if( getContentWindow( uri ))
    {
        emit requestFirstFrame( uri );
        put_flog( LOG_INFO, "start sending frames for stream window: '%s'",
                  uri.toLocal8Bit().constData( ));
        return;
    }

    put_flog( LOG_INFO, "opening pixel stream window: '%s'",
              uri.toLocal8Bit().constData( ));

    const auto type = _isPanel( uri ) ? ContentWindow::PANEL :
                                        ContentWindow::DEFAULT;

    ContentPtr content = webbrowser ?
                ContentFactory::getWebbrowserContent( uri ) :
                ContentFactory::getPixelStreamContent( uri );

    if( size.isValid( ))
        content->setDimensions( size );
    ContentWindowPtr window( new ContentWindow( content, type ));

    ContentWindowController controller( *window, _displayGroup );
    controller.resize( size.isValid() ? size : EMPTY_STREAM_SIZE );
    controller.moveCenterTo( !pos.isNull() ? pos :
                                      _displayGroup.getCoordinates().center( ));

    _streamerWindows[ uri ] = window->getID();
    _displayGroup.addContentWindow( window );

    if( _autoFocusNewWindows && !webbrowser )
        DisplayGroupController{ _displayGroup }.focus( window->getID( ));
}

void PixelStreamWindowManager::openPixelStreamWindow( const QString uri )
{
    openWindow( uri, QPointF(), QSize( ));
}

void PixelStreamWindowManager::closePixelStreamWindow( const QString uri )
{
    put_flog( LOG_INFO, "closing pixel stream window: '%s'",
              uri.toLocal8Bit().constData( ));

    ContentWindowPtr window = getContentWindow( uri );
    if( window )
        DisplayGroupController{ _displayGroup }.remove( window->getID( ));
}

void PixelStreamWindowManager::registerEventReceiver( const QString uri,
                                                      const bool exclusive,
                                                      deflect::EventReceiver*
                                                      receiver )
{
    ContentWindowPtr window = getContentWindow( uri );
    if( !window )
    {
        put_flog( LOG_DEBUG, "No window found for stream: '%s', creating one.",
                  uri.toStdString().c_str( ));
        openPixelStreamWindow( uri );
        window = getContentWindow( uri );
    }

    // If a receiver is already registered, don't register this one if
    // "exclusive" was requested
    bool success = false;
    auto& content = dynamic_cast<PixelStreamContent&>( *window->getContent( ));
    if( !exclusive || !content.hasEventReceivers( ))
    {
        success = connect( &content, &PixelStreamContent::notify,
                           receiver, &deflect::EventReceiver::processEvent );
        if( success )
            content.incrementEventReceiverCount();
        else
            put_flog( LOG_ERROR, "QObject connection failed" );
    }

    emit eventRegistrationReply( uri, success );
}

void PixelStreamWindowManager::onContentWindowRemoved( ContentWindowPtr window )
{
    const auto type = window->getContent()->getType();
    if( type != CONTENT_TYPE_PIXEL_STREAM && type != CONTENT_TYPE_WEBBROWSER )
        return;

    const QString& uri = window->getContent()->getURI();
    _streamerWindows.erase( uri );
    emit pixelStreamWindowClosed( uri );
}

void PixelStreamWindowManager::updateStreamDimensions( deflect::FramePtr frame )
{
    const QSize size( frame->computeDimensions( ));

    auto window = getContentWindow( frame->uri );
    if( !window )
        return;

    // External streamers might not have reported an initial size yet
    if( window->getContent()->getDimensions().isEmpty( ))
    {
        const auto target = ContentWindowController::Coordinates::STANDARD;
        ContentWindowController controller{ *window, _displayGroup, target };
        controller.resize( size, CENTER );
    }
    window->getContent()->setDimensions( size );
    if( window->isFocused( ))
        DisplayGroupController{_displayGroup}.updateFocusedWindowsCoordinates();
}

void PixelStreamWindowManager::sendDataToWindow( const QString uri,
                                                 const QByteArray data )
{
    auto window = getContentWindow( uri );
    if( !window )
        return;

    auto& content = dynamic_cast<PixelStreamContent&>( *window->getContent( ));
    content.parseData( data );
}

bool PixelStreamWindowManager::getAutoFocusNewWindows() const
{
    return _autoFocusNewWindows;
}

void PixelStreamWindowManager::setAutoFocusNewWindows( const bool set )
{
    _autoFocusNewWindows = set;
}

void PixelStreamWindowManager::updateSizeHints( const QString uri,
                                                const deflect::SizeHints hints )
{
    auto window = getContentWindow( uri );
    if( !window )
        return;

    window->getContent()->setSizeHints( hints );

    const QSize size( hints.preferredWidth, hints.preferredHeight );
    if( size.isEmpty( ))
        return;

   // External streamers might not have reported an initial size yet
    if( window->getContent()->getDimensions().isEmpty( ))
        window->getContent()->setDimensions( size );

    ContentWindowController{ *window, _displayGroup }.adjustSize( SIZE_1TO1 );
}

bool PixelStreamWindowManager::_isPanel( const QString& uri ) const
{
    return uri == PixelStreamerLauncher::launcherUri;
}

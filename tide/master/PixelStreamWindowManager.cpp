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

#include "control/ContentController.h"
#include "control/ContentWindowController.h"
#include "control/DisplayGroupController.h"
#include "localstreamer/PixelStreamerLauncher.h"
#include "log.h"
#include "scene/ContentFactory.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "scene/PixelStreamContent.h"

#include <deflect/EventReceiver.h>
#include <deflect/Frame.h>

namespace
{
const QSize EMPTY_STREAM_SIZE( 640, 480 );
}

PixelStreamWindowManager::PixelStreamWindowManager( DisplayGroup& displayGroup )
    : _displayGroup( displayGroup )
{
    connect( &displayGroup, &DisplayGroup::contentWindowRemoved,
             this, &PixelStreamWindowManager::_onWindowRemoved );
    connect( &displayGroup, &DisplayGroup::contentWindowAdded,
             this, &PixelStreamWindowManager::_onWindowAdded );
}

ContentWindowPtr PixelStreamWindowManager::getWindow( const QString& uri ) const
{
    const auto it = _streamWindows.find( uri );
    return it != _streamWindows.end() ?
                     _displayGroup.getContentWindow( it->second ) :
                     ContentWindowPtr();
}

void PixelStreamWindowManager::hideWindow( const QString& uri )
{
    if( auto window = getWindow( uri ))
        window->setState( ContentWindow::HIDDEN );
}

void PixelStreamWindowManager::showWindow( const QString& uri )
{
    auto window = getWindow( uri );
    if( !window )
        return;

    window->setState( ContentWindow::NONE );
    _displayGroup.moveToFront( window );
}

bool _isPanel( const QString& uri )
{
    return uri == PixelStreamerLauncher::launcherUri;
}

ContentWindowPtr _makeStreamWindow( const QString& uri, const QSize& size,
                                    const bool webbrowser )
{
    auto content = webbrowser ? ContentFactory::getWebbrowserContent( uri ) :
                                ContentFactory::getPixelStreamContent( uri );
    if( size.isValid( ))
        content->setDimensions( size );

    const auto type = _isPanel( uri ) ? ContentWindow::PANEL :
                                        ContentWindow::DEFAULT;
    return boost::make_shared<ContentWindow>( content, type );
}

void PixelStreamWindowManager::openWindow( const QString& uri,
                                           const QPointF& pos,
                                           const QSize& size,
                                           const StreamType stream )
{
    if( _isWindowOpen( uri ))
        return;

    put_flog( LOG_INFO, "opening pixel stream window: '%s'",
              uri.toLocal8Bit().constData( ));

    const auto webbrowser = (stream == StreamType::WEBBROWSER);
    auto window = _makeStreamWindow( uri, size, webbrowser );

    ContentWindowController controller{ *window, _displayGroup };
    controller.resize( size.isValid() ? size : EMPTY_STREAM_SIZE );
    controller.moveCenterTo( !pos.isNull() ? pos : _displayGroup.center( ));

    _displayGroup.addContentWindow( window );

    if( _autoFocusNewWindows && stream == StreamType::EXTERNAL )
        DisplayGroupController{ _displayGroup }.focus( window->getID( ));
}

void PixelStreamWindowManager::handleStreamStart( const QString uri )
{
    // internal streams already have a window
    if( _isWindowOpen( uri ))
    {
        emit requestFirstFrame( uri );
        put_flog( LOG_INFO, "start sending frames for stream window: '%s'",
                  uri.toLocal8Bit().constData( ));
        return;
    }

    // external streams don't have a window yet, create one now
    openWindow( uri, QPointF(), QSize( ));
}

void PixelStreamWindowManager::handleStreamEnd( const QString uri )
{
    put_flog( LOG_INFO, "closing pixel stream window: '%s'",
              uri.toLocal8Bit().constData( ));

    if( auto window = getWindow( uri ))
        DisplayGroupController{ _displayGroup }.remove( window->getID( ));
}

void PixelStreamWindowManager::registerEventReceiver( const QString uri,
                                                      const bool exclusive,
                                                      deflect::EventReceiver*
                                                      receiver )
{
    auto window = getWindow( uri );
    if( !window )
    {
        put_flog( LOG_DEBUG, "No window found for stream: '%s', creating one.",
                  uri.toStdString().c_str( ));
        openWindow( uri, QPointF(), QSize( ));
        window = getWindow( uri );
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

void PixelStreamWindowManager::updateStreamDimensions( deflect::FramePtr frame )
{
    const auto size = frame->computeDimensions();

    auto window = getWindow( frame->uri );
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
    auto window = getWindow( uri );
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
    auto window = getWindow( uri );
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

bool PixelStreamWindowManager::_isWindowOpen( const QString& uri ) const
{
    return _streamWindows.find( uri ) != _streamWindows.end();
}

bool _isStreamType( const CONTENT_TYPE type )
{
    return type == CONTENT_TYPE_PIXEL_STREAM || type == CONTENT_TYPE_WEBBROWSER;
}

void PixelStreamWindowManager::_onWindowAdded( ContentWindowPtr window )
{
    // Do the mapping here, not in openWindow(), to include any streamer
    // restored from a session (for which openWindow is not called).
    if( _isStreamType( window->getContent()->getType( )))
        _streamWindows[ window->getContent()->getURI( )] = window->getID();
}

void PixelStreamWindowManager::_onWindowRemoved( ContentWindowPtr window )
{
    if( !_isStreamType( window->getContent()->getType( )))
        return;

    const auto& uri = window->getContent()->getURI();
    _streamWindows.erase( uri );
    emit streamWindowClosed( uri );
}

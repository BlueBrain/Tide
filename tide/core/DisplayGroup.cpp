/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "DisplayGroup.h"

#include "ContentWindow.h"
#include "ContentWindowController.h"
#include "LayoutEngine.h"
#include "log.h"

IMPLEMENT_SERIALIZE_FOR_XML( DisplayGroup )

DisplayGroup::DisplayGroup()
{
}

DisplayGroup::DisplayGroup( const QSizeF& size )
    : _showWindowTitles( true )
{
    coordinates_.setSize( size );
}

DisplayGroup::~DisplayGroup() {}

void DisplayGroup::addContentWindow( ContentWindowPtr contentWindow )
{
    for( ContentWindowPtr existingWindow : _contentWindows )
    {
        if( contentWindow->getID() == existingWindow->getID( ))
        {
            put_flog( LOG_DEBUG, "A window with the same id already exists!" );
            return;
        }
    }

    _contentWindows.push_back( contentWindow );
    _watchChanges( contentWindow );
    _assignController( *contentWindow );

    emit( contentWindowAdded( contentWindow ));
    _sendDisplayGroup();
}

void DisplayGroup::removeWindowLater( const QUuid windowId )
{
    auto window = getContentWindow( windowId );
    QMetaObject::invokeMethod( this, "removeContentWindow",
                               Qt::QueuedConnection,
                               Q_ARG( ContentWindowPtr, window ));
}

void DisplayGroup::removeContentWindow( ContentWindowPtr contentWindow )
{
    ContentWindowPtrs::iterator it = find( _contentWindows.begin(),
                                           _contentWindows.end(),
                                           contentWindow );
    if( it == _contentWindows.end( ))
        return;

    _removeFocusedWindow( *it );
    _contentWindows.erase( it );

    // disconnect any existing connections with the window
    disconnect( contentWindow.get(), 0, this, 0 );

    emit( contentWindowRemoved( contentWindow ));
    _sendDisplayGroup();
}

void DisplayGroup::moveContentWindowToFront( const QUuid id )
{
    moveContentWindowToFront( getContentWindow( id ));
}

void DisplayGroup::moveContentWindowToFront( ContentWindowPtr contentWindow )
{
    if( contentWindow == _contentWindows.back( ))
        return;

    ContentWindowPtrs::iterator it = find( _contentWindows.begin(),
                                           _contentWindows.end(),
                                           contentWindow );
    if( it == _contentWindows.end( ))
        return;

    // move it to end of the list (last item rendered is on top)
    _contentWindows.erase( it );
    _contentWindows.push_back( contentWindow );

    emit( contentWindowMovedToFront( contentWindow ));
    _sendDisplayGroup();
}

bool DisplayGroup::getShowWindowTitles() const
{
    return _showWindowTitles;
}

bool DisplayGroup::isEmpty() const
{
    return _contentWindows.empty();
}

ContentWindowPtr DisplayGroup::getActiveWindow() const
{
    if ( isEmpty( ))
        return ContentWindowPtr();

    return _contentWindows.back();
}

const ContentWindowPtrs& DisplayGroup::getContentWindows() const
{
    return _contentWindows;
}

ContentWindowPtr DisplayGroup::getContentWindow( const QUuid& id ) const
{
    for( ContentWindowPtr window : _contentWindows )
    {
        if( window->getID() == id )
            return window;
    }
    return ContentWindowPtr();
}

void DisplayGroup::setContentWindows( ContentWindowPtrs contentWindows )
{
    clear();

    for( ContentWindowPtr window : contentWindows )
    {
        addContentWindow( window );
        if( window->isFocused( ))
            _focusedWindows.insert( window );
    }
    _updateFocusedWindowsCoordinates();
    _sendDisplayGroup();
}

int DisplayGroup::getZindex( ContentWindowPtr window ) const
{
    const auto it = std::find( _contentWindows.begin(), _contentWindows.end(),
                               window );
    return it ==_contentWindows.end() ? - 1 : it - _contentWindows.begin();
}

bool DisplayGroup::hasFocusedWindows() const
{
    return !_focusedWindows.empty();
}

void DisplayGroup::focus( const QUuid& id )
{
    ContentWindowPtr window = getContentWindow( id );
    if( !window || window->isPanel() || _focusedWindows.count( window ))
        return;

    _focusedWindows.insert( window );
    _updateFocusedWindowsCoordinates();
    window->setFocused( true );

    if( _focusedWindows.size() == 1 )
        emit hasFocusedWindowsChanged();

    _sendDisplayGroup();
}

void DisplayGroup::unfocus( const QUuid& id )
{
    ContentWindowPtr window = getContentWindow( id );
    if( !window )
        return;

    window->setFocused( false );
    _removeFocusedWindow( window );
    // Make sure the window dimensions are re-adjusted to the new zoom level
    window->getController()->scale( window->getCoordinates().center(), 0.0 );

    _sendDisplayGroup();
}

const ContentWindowSet& DisplayGroup::getFocusedWindows() const
{
    return _focusedWindows;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
void DisplayGroup::moveToThread( QThread* thread )
{
    QObject::moveToThread( thread );
    for( auto window : _contentWindows )
    {
        window->moveToThread( thread );
        window->getContent()->moveToThread( thread );
    }
}
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

void DisplayGroup::clear()
{
    if( _contentWindows.empty( ))
        return;

    // Close regular windows but hide panels (instead of removing them)
    ContentWindowPtrs removeSet;
    for( auto window : _contentWindows )
    {
        if( window->isPanel( ))
            window->setState( ContentWindow::HIDDEN );
        else
            removeSet.push_back( window );
    }

    put_flog( LOG_INFO, "removing %i windows", removeSet.size( ));

    for( auto window : removeSet )
        removeContentWindow( window );
}

void DisplayGroup::setShowWindowTitles( const bool set )
{
    if( _showWindowTitles == set )
        return;

    _showWindowTitles = set;

    emit showWindowTitlesChanged( set );
    _sendDisplayGroup();
}

void DisplayGroup::_sendDisplayGroup()
{
    emit modified( shared_from_this( ));
}

void DisplayGroup::_watchChanges( ContentWindowPtr contentWindow )
{
    connect( contentWindow.get(), &ContentWindow::modified,
             this, &DisplayGroup::_sendDisplayGroup );

    connect( contentWindow.get(), &ContentWindow::contentModified,
             this, [this]()
    {
        _updateFocusedWindowsCoordinates();
        _sendDisplayGroup();
    } );
}

void DisplayGroup::_assignController( ContentWindow& window )
{
    window.setController( make_unique<ContentWindowController>( window, *this));
}

void DisplayGroup::_removeFocusedWindow( ContentWindowPtr window )
{
    if( _focusedWindows.erase( window ))
    {
        _updateFocusedWindowsCoordinates();

        if( _focusedWindows.empty( ))
            emit hasFocusedWindowsChanged();
    }
}

void DisplayGroup::_updateFocusedWindowsCoordinates()
{
    LayoutEngine engine( *this );
    for( auto window : _focusedWindows )
        window->setFocusedCoordinates( engine.getFocusedCoord( *window ));
}

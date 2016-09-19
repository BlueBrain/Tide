/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "DisplayGroupRenderer.h"

#include "DisplayGroup.h"
#include "ContentWindow.h"
#include "ContentWindowController.h"
#include "Options.h"
#include "Markers.h"
#include "VisibilityHelper.h"
#include "WallWindow.h"
#include "qmlUtils.h"

#include <deflect/Frame.h>

#include <boost/make_shared.hpp>

#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItem>

namespace
{
const QUrl QML_DISPLAYGROUP_URL( "qrc:/qml/wall/WallDisplayGroup.qml" );
}

DisplayGroupRenderer::DisplayGroupRenderer( WallWindow& parentWindow,
                                            DataProvider& provider,
                                            const QRect& screenRect )
    : _engine( *parentWindow.engine( ))
    , _provider( provider )
    , _displayGroup( new DisplayGroup( QSize( )))
    , _displayGroupItem( 0 )
    , _options( new Options )
    , _markers( new Markers )
    , _screenRect( screenRect )
{
    _engine.rootContext()->setContextProperty( "markers", _markers.get( ));
    _engine.rootContext()->setContextProperty( "options", _options.get( ));
    _createDisplayGroupQmlItem( *parentWindow.rootObject( ));
    _displayGroupItem->setPosition( -screenRect.topLeft( ));
    _setBackground( _options->getBackgroundContent( ));
}

void DisplayGroupRenderer::synchronize( WallToWallChannel& channel )
{
    for( auto& windowRenderer : _windowItems )
        windowRenderer->synchronize( channel );
}

bool DisplayGroupRenderer::needRedraw() const
{
    return _options->getShowStatistics() || _options->getShowClock();
}

void DisplayGroupRenderer::setRenderingOptions( OptionsPtr options )
{
    _engine.rootContext()->setContextProperty( "options", options.get( ));
    _setBackground( options->getBackgroundContent( ));
    _displayGroupItem->setVisible( !options->getShowTestPattern( ));
    _options = options; // Retain the new Options
}

void DisplayGroupRenderer::setMarkers( MarkersPtr markers )
{
    _engine.rootContext()->setContextProperty( "markers", markers.get( ));
    _markers = markers; // Retain the new Markers
}

void DisplayGroupRenderer::setDisplayGroup( DisplayGroupPtr displayGroup )
{
    // Update the scene with the new information
    _engine.rootContext()->setContextProperty( "displaygroup",
                                               displayGroup.get( ));

    ContentWindowPtrs contentWindows = displayGroup->getContentWindows();

    // Update windows, creating new ones if needed
    QSet<QUuid> updatedWindows;
    const QQuickItem* parentItem = nullptr;
    const VisibilityHelper helper( *displayGroup, _screenRect );
    for( ContentWindowPtr window : contentWindows )
    {
        const QUuid& id = window->getID();

        updatedWindows.insert( id );

        if( !_windowItems.contains( id ))
            _createWindowQmlItem( window );

        _windowItems[id]->update( window, helper.getVisibleArea( *window ));

        // Update stacking order
        QQuickItem* item = _windowItems[id]->getQuickItem();
        if( parentItem )
            item->stackAfter( parentItem );
        parentItem = item;
    }

    // Remove old windows
    QmlWindows::iterator it = _windowItems.begin();
    while( it != _windowItems.end( ))
    {
        if( updatedWindows.contains( it.key( )))
            ++it;
        else
        {
            emit windowRemoved( *it );
            it = _windowItems.erase( it );
        }
    }

    // Retain the new DisplayGroup
    _displayGroup = displayGroup;

    // Work around a bug in animation in Qt, where the opacity property
    // of the focus context may not always be restored to its original value.
    // See JIRA issue: DISCL-305
    if( !displayGroup->hasFocusedWindows() &&
            !displayGroup->hasFullscreenWindows( ))
    {
        for( QQuickItem* child : _displayGroupItem->childItems( ))
        {
            if( child->objectName() == "focuscontext" )
                child->setProperty( "opacity", 0.0 );
        }
    }
}

void DisplayGroupRenderer::updateRenderedFrames()
{
    const int frames = _displayGroupItem->property( "frames" ).toInt();
    _displayGroupItem->setProperty( "frames", frames + 1 );
}

void DisplayGroupRenderer::_createDisplayGroupQmlItem( QQuickItem& parentItem )
{
    _engine.rootContext()->setContextProperty( "displaygroup",
                                              _displayGroup.get( ));

    QQmlComponent component( &_engine, QML_DISPLAYGROUP_URL );
    qmlCheckOrThrow( component );
    _displayGroupItem = qobject_cast<QQuickItem*>( component.create( ));
    _displayGroupItem->setParentItem( &parentItem );
}

void DisplayGroupRenderer::_createWindowQmlItem( ContentWindowPtr window )
{
    const QUuid& id = window->getID();
    _windowItems[id].reset( new QmlWindowRenderer( _engine, _provider,
                                                   *_displayGroupItem,
                                                   window ));
    emit windowAdded( _windowItems[id] );
}

bool DisplayGroupRenderer::_hasBackgroundChanged( const QString& newUri ) const
{
    ContentPtr prevContent = _options->getBackgroundContent();
    const QString& prevUri = prevContent ? prevContent->getURI() : QString();
    return newUri != prevUri;
}

void DisplayGroupRenderer::_setBackground( ContentPtr content )
{
    if( !content )
    {
        _backgroundWindowItem.reset();
        return;
    }

    if( !_hasBackgroundChanged( content->getURI( )))
        return;

    ContentWindowPtr window = boost::make_shared<ContentWindow>( content );
    window->setController(
               make_unique<ContentWindowController>( *window, *_displayGroup ));
    window->getController()->adjustSize( SIZE_FULLSCREEN );
    _backgroundWindowItem.reset( new QmlWindowRenderer( _engine,
                                                        _provider,
                                                        *_displayGroupItem,
                                                        window,
                                                        true ));

    DisplayGroup emptyGroup( _screenRect.size( ));
    const VisibilityHelper helper( emptyGroup, _screenRect );
    _backgroundWindowItem->update( window, helper.getVisibleArea( *window ));
}

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

#include "DisplayGroupView.h"

#include "ContentWindow.h"
#include "DisplayGroup.h"
#include "Options.h"
#include "MasterConfiguration.h"
#include "qmlUtils.h"

#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlProperty>

namespace
{
const QUrl QML_CONTENTWINDOW_URL( "qrc:/qml/master/MasterContentWindow.qml" );
const QUrl QML_DISPLAYGROUP_URL( "qrc:/qml/master/MasterDisplayGroup.qml" );
const QUrl QML_BACKGROUND_URL( "qrc:/qml/master/DisplayGroupBackground.qml" );
const QString WALL_OBJECT_NAME( "Wall" );
}

DisplayGroupView::DisplayGroupView( OptionsPtr options,
                                    const MasterConfiguration& config )
    : _displayGroupItem( nullptr )
{
    setResizeMode( QQuickView::SizeRootObjectToView );

    rootContext()->setContextProperty( "options", options.get( ));
    rootContext()->setContextProperty( "view", this );

    setSource( QML_BACKGROUND_URL );

    _wallObject = rootObject()->findChild<QObject*>( WALL_OBJECT_NAME );
    _wallObject->setProperty( "numberOfTilesX", config.getTotalScreenCountX( ));
    _wallObject->setProperty( "numberOfTilesY", config.getTotalScreenCountY( ));
    _wallObject->setProperty( "mullionWidth", config.getMullionWidth( ));
    _wallObject->setProperty( "mullionHeight", config.getMullionHeight( ));
    _wallObject->setProperty( "screenWidth", config.getScreenWidth( ));
    _wallObject->setProperty( "screenHeight", config.getScreenHeight( ));
    _wallObject->setProperty( "wallWidth", config.getTotalWidth( ));
    _wallObject->setProperty( "wallHeight", config.getTotalHeight( ));
}

DisplayGroupView::~DisplayGroupView() {}

void DisplayGroupView::setDataModel( DisplayGroupPtr displayGroup )
{
    if( _displayGroup )
    {
        _displayGroup->disconnect( this );
        _clearScene();
    }

    _displayGroup = displayGroup;
    if( !_displayGroup )
        return;

    rootContext()->setContextProperty( "displaygroup", _displayGroup.get( ));

    QQmlComponent component( engine(), QML_DISPLAYGROUP_URL );
    _displayGroupItem = qobject_cast< QQuickItem* >( component.create( ));
    qmlCheckOrThrow( component );
    auto wallObject = rootObject()->findChild<QQuickItem*>( WALL_OBJECT_NAME );
    _displayGroupItem->setParentItem( wallObject );

    connect( _displayGroupItem, SIGNAL( launcherControlPressed( )),
             this, SIGNAL( launcherControlPressed( )));
    connect( _displayGroupItem, SIGNAL( settingsControlsPressed( )),
             this, SIGNAL( settingsControlsPressed( )));

    ContentWindowPtrs contentWindows = _displayGroup->getContentWindows();
    for( ContentWindowPtr contentWindow : contentWindows )
        _add( contentWindow );

    connect( _displayGroup.get(),
             SIGNAL( contentWindowAdded( ContentWindowPtr )),
             this, SLOT( _add( ContentWindowPtr )));
    connect( _displayGroup.get(),
             SIGNAL( contentWindowRemoved( ContentWindowPtr )),
             this, SLOT( _remove( ContentWindowPtr )));
    connect( _displayGroup.get(),
             SIGNAL( contentWindowMovedToFront( ContentWindowPtr )),
             this, SLOT( _moveToFront( ContentWindowPtr )));
}

QPointF DisplayGroupView::mapToWallPos( const QPointF& normalizedPos ) const
{
    const float scale = QQmlProperty::read( _wallObject, "scale" ).toFloat();
    const float offsetX = QQmlProperty::read( _wallObject, "offsetX" ).toFloat();
    const float offsetY = QQmlProperty::read( _wallObject, "offsetY" ).toFloat();

    const float screenPosX = normalizedPos.x() * _displayGroup->width() *
                             scale + offsetX;
    const float screenPosY = normalizedPos.y() * _displayGroup->height() *
                             scale + offsetY;

    return QPointF( screenPosX, screenPosY );
}

void DisplayGroupView::_clearScene()
{
    _uuidToWindowMap.clear();

    if( _displayGroupItem )
    {
        _displayGroupItem->setParentItem( 0 );
        delete _displayGroupItem;
        _displayGroupItem = 0;
    }
}

bool DisplayGroupView::event( QEvent *evt )
{
    switch( evt->type( ))
    {
    case QEvent::KeyPress:
    {
        QKeyEvent* k = static_cast< QKeyEvent* >( evt );

        // Override default behaviour to process TAB key events
        QQuickView::keyPressEvent( k );

        if( k->key() == Qt::Key_Backtab ||
            k->key() == Qt::Key_Tab ||
           ( k->key() == Qt::Key_Tab && ( k->modifiers() & Qt::ShiftModifier )))
        {
            evt->accept();
        }
        return true;
    }
    case QEvent::MouseButtonPress:
    {
        QMouseEvent* e = static_cast< QMouseEvent* >( evt );
        if( e->button() == Qt::LeftButton )
            emit mousePressed( _displayGroupItem->mapFromScene( e->localPos( )));
        break;
    }
    case QEvent::MouseMove:
    {
        QMouseEvent* e = static_cast< QMouseEvent* >( evt );
        if( e->buttons() & Qt::LeftButton )
            emit mouseMoved( _displayGroupItem->mapFromScene( e->localPos( )));
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent* e = static_cast< QMouseEvent* >( evt );
        if( e->button() == Qt::LeftButton )
            emit mouseReleased( _displayGroupItem->mapFromScene( e->localPos( )));
        break;
    }
    default:
        break;
    }
    return QQuickView::event( evt );
}

void DisplayGroupView::_add( ContentWindowPtr contentWindow )
{
    // New Context for the window, ownership retained by the windowItem
    QQmlContext* windowContext = new QQmlContext( rootContext( ));
    windowContext->setContextProperty( "contentwindow", contentWindow.get( ));

    QQmlComponent component( engine(), QML_CONTENTWINDOW_URL );
    QObject* windowItem = component.create( windowContext );
    windowContext->setParent( windowItem );

    // Store a reference to the window and add it to the scene
    const QUuid& id = contentWindow->getID();
    _uuidToWindowMap[ id ] = qobject_cast<QQuickItem*>( windowItem );
    _uuidToWindowMap[ id ]->setParentItem( _displayGroupItem );
}

void DisplayGroupView::_remove( ContentWindowPtr contentWindow )
{
    const QUuid& id = contentWindow->getID();
    if( !_uuidToWindowMap.contains( id ))
        return;

    QQuickItem* itemToRemove = _uuidToWindowMap[id];
    _uuidToWindowMap.remove( id );
    delete itemToRemove;
}

void DisplayGroupView::_moveToFront( ContentWindowPtr contentWindow )
{
    const QUuid& id = contentWindow->getID();
    if( !_uuidToWindowMap.contains( id ))
        return;

    QQuickItem* itemToRaise = _uuidToWindowMap[id];
    itemToRaise->stackAfter( _displayGroupItem->childItems().last( ));
}

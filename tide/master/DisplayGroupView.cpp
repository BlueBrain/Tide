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
    : displayGroupItem_( nullptr )
{
    setResizeMode( QQuickView::SizeRootObjectToView );

    rootContext()->setContextProperty( "options", options.get( ));
    rootContext()->setContextProperty( "view", this );

    setSource( QML_BACKGROUND_URL );

    wallObject_ = rootObject()->findChild<QObject*>( WALL_OBJECT_NAME );
    wallObject_->setProperty( "numberOfTilesX", config.getTotalScreenCountX( ));
    wallObject_->setProperty( "numberOfTilesY", config.getTotalScreenCountY( ));
    wallObject_->setProperty( "mullionWidth", config.getMullionWidth( ));
    wallObject_->setProperty( "mullionHeight", config.getMullionHeight( ));
    wallObject_->setProperty( "screenWidth", config.getScreenWidth( ));
    wallObject_->setProperty( "screenHeight", config.getScreenHeight( ));
    wallObject_->setProperty( "wallWidth", config.getTotalWidth( ));
    wallObject_->setProperty( "wallHeight", config.getTotalHeight( ));
}

DisplayGroupView::~DisplayGroupView() {}

void DisplayGroupView::setDataModel( DisplayGroupPtr displayGroup )
{
    if( displayGroup_ )
    {
        displayGroup_->disconnect( this );
        clearScene();
    }

    displayGroup_ = displayGroup;
    if( !displayGroup_ )
        return;

    rootContext()->setContextProperty( "displaygroup", displayGroup_.get( ));

    QQmlComponent component( engine(), QML_DISPLAYGROUP_URL );
    displayGroupItem_ = qobject_cast< QQuickItem* >( component.create( ));
    qmlCheckOrThrow( component );
    auto wallObject = rootObject()->findChild<QQuickItem*>( WALL_OBJECT_NAME );
    displayGroupItem_->setParentItem( wallObject );

    connect( displayGroupItem_, SIGNAL( launcherControlPressed( )),
             this, SIGNAL( launcherControlPressed( )));
    connect( displayGroupItem_, SIGNAL( settingsControlsPressed( )),
             this, SIGNAL( settingsControlsPressed( )));

    ContentWindowPtrs contentWindows = displayGroup_->getContentWindows();
    for( ContentWindowPtr contentWindow : contentWindows )
        add( contentWindow );

    connect( displayGroup_.get(),
             SIGNAL( contentWindowAdded( ContentWindowPtr )),
             this, SLOT( add( ContentWindowPtr )));
    connect( displayGroup_.get(),
             SIGNAL( contentWindowRemoved( ContentWindowPtr )),
             this, SLOT( remove( ContentWindowPtr )));
    connect( displayGroup_.get(),
             SIGNAL( contentWindowMovedToFront( ContentWindowPtr )),
             this, SLOT( moveToFront( ContentWindowPtr )));
}

QPointF DisplayGroupView::mapToWallPos( const QPointF& normalizedPos ) const
{
    const float scale = QQmlProperty::read( wallObject_, "scale" ).toFloat();
    const float offsetX = QQmlProperty::read( wallObject_, "offsetX" ).toFloat();
    const float offsetY = QQmlProperty::read( wallObject_, "offsetY" ).toFloat();

    const float screenPosX = normalizedPos.x() * displayGroup_->width() *
                             scale + offsetX;
    const float screenPosY = normalizedPos.y() * displayGroup_->height() *
                             scale + offsetY;

    return QPointF( screenPosX, screenPosY );
}

void DisplayGroupView::clearScene()
{
    uuidToWindowMap_.clear();

    if( displayGroupItem_ )
    {
        displayGroupItem_->setParentItem( 0 );
        delete displayGroupItem_;
        displayGroupItem_ = 0;
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
    default:
        return QQuickView::event( evt );
    }
}

void DisplayGroupView::add( ContentWindowPtr contentWindow )
{
    // New Context for the window, ownership retained by the windowItem
    QQmlContext* windowContext = new QQmlContext( rootContext( ));
    windowContext->setContextProperty( "contentwindow", contentWindow.get( ));

    QQmlComponent component( engine(), QML_CONTENTWINDOW_URL );
    QObject* windowItem = component.create( windowContext );
    windowContext->setParent( windowItem );

    // Store a reference to the window and add it to the scene
    const QUuid& id = contentWindow->getID();
    uuidToWindowMap_[ id ] = qobject_cast<QQuickItem*>( windowItem );
    uuidToWindowMap_[ id ]->setParentItem( displayGroupItem_ );
}

void DisplayGroupView::remove( ContentWindowPtr contentWindow )
{
    const QUuid& id = contentWindow->getID();
    if( !uuidToWindowMap_.contains( id ))
        return;

    QQuickItem* itemToRemove = uuidToWindowMap_[id];
    uuidToWindowMap_.remove( id );
    delete itemToRemove;
}

void DisplayGroupView::moveToFront( ContentWindowPtr contentWindow )
{
    const QUuid& id = contentWindow->getID();
    if( !uuidToWindowMap_.contains( id ))
        return;

    QQuickItem* itemToRaise = uuidToWindowMap_[id];
    itemToRaise->stackAfter( displayGroupItem_->childItems().last( ));
}

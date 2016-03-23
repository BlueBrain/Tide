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
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "Tile.h"

#include "QuadLineNode.h"
#include "TextureNode.h"
#include "log.h"

namespace
{
const qreal borderWidth = 10.0;
const QColor borderColor( "lightgreen" );
}

// false-positive on qt signals for Q_PROPERTY notifiers
// cppcheck-suppress uninitMemberVar
Tile::Tile( const uint id, const QRect& rect )
    : _tileId( id )
    , _policy( AdjustToTexture )
    , _swapRequested( false )
    , _updateTextureRequested( true )
    , _nextCoord( rect )
    , _backGlTexture( 0 )
    , _showBorder( false )
    , _border( nullptr )
{
    setFlag( ItemHasContents, true );
    setVisible( false );

    connect( this, &QQuickItem::parentChanged, this, &Tile::_onParentChanged );
}

uint Tile::getId() const
{
    return _tileId;
}

bool Tile::getShowBorder() const
{
    return _showBorder;
}

void Tile::setShowBorder( const bool set )
{
    if( _showBorder == set )
        return;

    _showBorder = set;
    emit showBorderChanged();
    QQuickItem::update();
}

void Tile::update( const QRect& rect )
{
    _updateTextureRequested = true;
    _nextCoord = rect;
    QQuickItem::update();
}

uint Tile::getBackGlTexture() const
{
    return _backGlTexture;
}

QSize Tile::getBackGlTextureSize() const
{
    return _nextCoord.size();
}

void Tile::setSizePolicy( const Tile::SizePolicy policy )
{
    _policy = policy;
}

void Tile::swapImage()
{
    _swapRequested = true;

    if( !isVisible( ))
        setVisible( true );

    if( _policy == AdjustToTexture )
    {
        setPosition( _nextCoord.topLeft( ));
        setSize( _nextCoord.size( ));
    }

    QQuickItem::update();
}

QSGNode* Tile::updatePaintNode( QSGNode* oldNode,
                                QQuickItem::UpdatePaintNodeData* )
{
    TextureNode* node = static_cast<TextureNode*>( oldNode );
    if( !node )
        node = new TextureNode( _nextCoord.size(), window( ));

    if( _swapRequested )
    {
        node->swap();
        _backGlTexture = node->getBackGlTexture();
        _swapRequested = false;
    }

    if( _updateTextureRequested )
    {
        node->setBackTextureSize( _nextCoord.size( ));
        _backGlTexture = node->getBackGlTexture();
        _updateTextureRequested = false;
        emit textureReady( shared_from_this( ));
    }

    node->setRect( boundingRect( ));

    _updateBorderNode( node );

    return node;
}

void Tile::_updateBorderNode( TextureNode* parentNode )
{
    if( _showBorder )
    {
        if( !_border )
        {
            _border = new QuadLineNode( parentNode->rect(), borderWidth );
            _border->setColor( borderColor );
            parentNode->appendChildNode( _border );
        }
        else
            _border->setRect( parentNode->rect( ));
    }
    else if( _border )
    {
        parentNode->removeChildNode( _border );
        delete _border;
        _border = nullptr;
    }
}

void Tile::_onParentChanged( QQuickItem* newParent )
{
    if( !newParent )
    {
        disconnect( _widthConn );
        disconnect( _heightConn );
        return;
    }

    if( _policy != FillParent )
        return;

    setWidth( newParent->width( ));
    setHeight( newParent->height( ));

    _widthConn = connect( newParent, &QQuickItem::widthChanged, [this]() {
        setWidth( parentItem()->width( ));
    } );
    _heightConn = connect( newParent, &QQuickItem::heightChanged, [this]() {
        setHeight( parentItem()->height( ));
    } );
}

/*********************************************************************/
/* Copyright (c) 2014-2016, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#include "ContentWindowController.h"

#include "ContentWindow.h"
#include "DisplayGroup.h"
#include "geometry.h"
#include "ZoomController.h"

namespace
{
const qreal MIN_SIZE = 0.05;
const qreal MIN_VISIBLE_AREA_PX = 300.0;
const qreal LARGE_SIZE_SCALE = 0.75;
const qreal FITTING_SIZE_SCALE = 0.9;
const qreal ONE_PERCENT = 0.01;
}

ContentWindowController::ContentWindowController( ContentWindow& contentWindow,
                                                  const DisplayGroup& group,
                                                  Coordinates target )
    : _contentWindow( contentWindow )
    , _displayGroup( group )
    , _target( target )
{}

void ContentWindowController::resize( const QSizeF size,
                                      const WindowPoint fixedPoint )
{
    QSizeF newSize( _contentWindow.getContent()->getDimensions( ));
    if( newSize.isEmpty( ))
        newSize = size;
    else
        newSize.scale( size, Qt::KeepAspectRatio );

    switch( fixedPoint )
    {
    case CENTER:
        _resize( _getCoordinates().center(), newSize );
        break;
    case TOP_LEFT:
    default:
        _resize( _getCoordinates().topLeft(), newSize );
    }
}

void ContentWindowController::resizeRelative( const QPointF& delta )
{
    const QRectF& coord = _getCoordinates();

    QPointF fixedPoint;
    QSizeF newSize = coord.size();

    switch( _contentWindow.getActiveHandle( ))
    {
    case ContentWindow::TOP:
        fixedPoint = QPointF( coord.left() + coord.width() * 0.5,
                              coord.bottom( ));
        newSize += QSizeF( 0, -delta.y( ));
        break;
    case ContentWindow::RIGHT:
        fixedPoint = QPointF( coord.left(),
                              coord.top() + coord.height() * 0.5 );
        newSize += QSizeF( delta.x(), 0 );
        break;
    case ContentWindow::BOTTOM:
        fixedPoint = QPointF( coord.left() + coord.width() * 0.5,
                              coord.top( ));
        newSize += QSizeF( 0, delta.y( ));
        break;
    case ContentWindow::LEFT:
        fixedPoint = QPointF( coord.right(),
                              coord.top() + coord.height() * 0.5 );
        newSize += QSizeF( -delta.x(), 0 );
        break;
    case ContentWindow::TOP_LEFT:
        fixedPoint = coord.bottomRight();
        newSize += QSizeF( -delta.x(), -delta.y( ));
        break;
    case ContentWindow::BOTTOM_LEFT:
        fixedPoint = coord.topRight();
        newSize += QSizeF( -delta.x(), delta.y( ));
        break;
    case ContentWindow::TOP_RIGHT:
        fixedPoint = coord.bottomLeft();
        newSize += QSizeF( delta.x(), -delta.y( ));
        break;
    case ContentWindow::BOTTOM_RIGHT:
        fixedPoint = coord.topLeft();
        newSize += QSizeF( delta.x(), delta.y( ));
        break;
    case ContentWindow::NOHANDLE:
    default:
        return;
    }

    // Resizing from one of the corners modifies the aspect ratio.
    // Resizing from one of the sides borders tend to let the window snap back
    // to its content's aspect ratio.
    if( _contentWindow.getResizePolicy() == ContentWindow::KEEP_ASPECT_RATIO )
    {
        if( _contentWindow.getContent()->getZoomRect() == UNIT_RECTF )
            _constrainAspectRatio( newSize );
        if( _isCloseToContentAspectRatio( newSize ))
            _snapToContentAspectRatio( newSize );
    }

    _resize( fixedPoint, newSize );
}

void ContentWindowController::scale( const QPointF& center,
                                     const double pixelDelta )
{
    QSizeF newSize = _getCoordinates().size();
    newSize.scale( newSize.width() + pixelDelta,
                   newSize.height() + pixelDelta,
                   pixelDelta < 0 ? Qt::KeepAspectRatio
                                  : Qt::KeepAspectRatioByExpanding );
    _resize( center, newSize );
}

void ContentWindowController::adjustSize( const SizeState state )
{
    switch( state )
    {
    case SIZE_1TO1:
        resize( _contentWindow.getContent()->getPreferredDimensions(), CENTER);
        break;

    case SIZE_1TO1_FITTING:
    {
        const QSizeF max = _displayGroup.size() * FITTING_SIZE_SCALE;
        const QSize oneToOne =
                _contentWindow.getContent()->getPreferredDimensions();
        if( oneToOne.width() > max.width() || oneToOne.height() > max.height( ))
            resize( max, CENTER );
        else
            resize( oneToOne, CENTER );
    } break;

    case SIZE_LARGE:
        resize( LARGE_SIZE_SCALE * _displayGroup.size(), CENTER );
        break;

    case SIZE_FULLSCREEN:
    {
        auto content = _contentWindow.getContent();
        content->resetZoom();

        auto size = geometry::getAdjustedSize( *content, _displayGroup );
        constrainSize( size );
        _apply( _getCenteredCoordinates( size ));
    } break;

    case SIZE_FULLSCREEN_MAX:
    {
        auto content = _contentWindow.getContent();
        content->resetZoom();

        auto size = geometry::getExpandedSize( *content, _displayGroup );
        constrainSize( size );
        _apply( _getCenteredCoordinates( size ));
    } break;

    default:
        return;
    }
}

void ContentWindowController::toogleFullscreenMaxSize()
{
    if( !_targetIsFullscreen( ))
        return;

    const auto windowSize = _getCoordinates().size();
    if( windowSize < _displayGroup.size( ))
        adjustSize( SizeState::SIZE_FULLSCREEN_MAX );
    else
        adjustSize( SizeState::SIZE_FULLSCREEN );
}

void ContentWindowController::moveTo( const QPointF& position,
                                      const WindowPoint handle )
{
    auto coordinates = _getCoordinates();
    switch( handle )
    {
    case TOP_LEFT:
        coordinates.moveTopLeft( position );
        break;
    case CENTER:
        coordinates.moveCenter( position );
        break;
    default:
        return;
    }
    _constrainPosition( coordinates );

    _apply( coordinates );
}

void ContentWindowController::moveBy( const QPointF& delta )
{
    moveTo( _getCoordinates().topLeft() + delta );
}

QSizeF ContentWindowController::getMinSize() const
{
    const QSizeF wallSize = _displayGroup.size();
    if( _targetIsFullscreen( ))
    {
        const QSizeF contentSize = _contentWindow.getContent()->getDimensions();
        return contentSize.scaled( wallSize, Qt::KeepAspectRatio );
    }

    const QSizeF& minContentSize =
            _contentWindow.getContent()->getMinDimensions();
    const QSizeF minSize(
                std::max( MIN_SIZE * wallSize.width(), MIN_VISIBLE_AREA_PX ),
                std::max( MIN_SIZE * wallSize.height(), MIN_VISIBLE_AREA_PX ));
    return std::max( minContentSize, minSize );
}

QSizeF ContentWindowController::getMaxSize() const
{
    const QRectF& zoomRect = _contentWindow.getContent()->getZoomRect();
    QSizeF maxSize = _contentWindow.getContent()->getMaxDimensions();
    maxSize.rwidth() *= zoomRect.size().width();
    maxSize.rheight() *= zoomRect.size().height();
    return maxSize;
}

void ContentWindowController::constrainSize( QSizeF& windowSize ) const
{
    windowSize = geometry::constrain( windowSize, getMinSize(), getMaxSize( ));
}

void ContentWindowController::_resize( const QPointF& center, QSizeF size )
{
    constrainSize( size );

    auto coordinates = _getCoordinates();
    coordinates = geometry::resizeAroundPosition( coordinates, center, size );
    _constrainPosition( coordinates );

    _apply( coordinates );

    auto controller = ContentController::create( _contentWindow );
    auto zoomController = dynamic_cast<ZoomController*>( controller.get());
    if( zoomController )
        zoomController->adjustZoomToContentAspectRatio();
}

void ContentWindowController::_constrainAspectRatio( QSizeF& windowSize ) const
{
    const QSizeF currentSize = _getCoordinates().size();
    const auto mode = windowSize < currentSize ?
                           Qt::KeepAspectRatio : Qt::KeepAspectRatioByExpanding;
    windowSize = currentSize.scaled( windowSize, mode );
}

bool ContentWindowController::_isCloseToContentAspectRatio( const QSizeF&
                                                            windowSize ) const
{
    const qreal windowAR = windowSize.width() / windowSize.height();
    const qreal contentAR = _contentWindow.getContent()->getAspectRatio();

    return std::fabs( windowAR - contentAR ) < ONE_PERCENT;
}

void ContentWindowController::_snapToContentAspectRatio( QSizeF&
                                                         windowSize ) const
{
    const QSizeF contentSize( _contentWindow.getContent()->getDimensions( ));
    const auto mode = windowSize < contentSize ?
                       Qt::KeepAspectRatio : Qt::KeepAspectRatioByExpanding;
    windowSize = contentSize.scaled( windowSize, mode );
}

void ContentWindowController::_constrainPosition( QRectF& window ) const
{
    const QRectF& group = _displayGroup.getCoordinates();

    if( _targetIsFullscreen( ))
    {
        const qreal overlapX = group.width() - window.width();
        const qreal overlapY = group.height() - window.height();

        const qreal minX = overlapX < 0.0 ? overlapX : 0.5 * overlapX;
        const qreal minY = overlapY < 0.0 ? overlapY : 0.5 * overlapY;

        const qreal maxX = 0.0;
        const qreal maxY = 0.0;

        window.moveTopLeft( { std::max( minX, std::min( window.x(), maxX )),
                              std::max( minY, std::min( window.y(), maxY ))} );
        return;
    }

    const qreal minX = MIN_VISIBLE_AREA_PX - window.width();
    const qreal minY = MIN_VISIBLE_AREA_PX - window.height();

    const qreal maxX = group.width() - MIN_VISIBLE_AREA_PX;
    const qreal maxY = group.height() - MIN_VISIBLE_AREA_PX;

    window.moveTopLeft( { std::max( minX, std::min( window.x(), maxX )),
                          std::max( minY, std::min( window.y(), maxY ))} );
}

QRectF
ContentWindowController::_getCenteredCoordinates( const QSizeF& size ) const
{
    const QRectF& group = _displayGroup.getCoordinates();

    // centered coordinates on the display group
    QRectF coord( QPointF(), size );
    coord.moveCenter( group.center( ));
    return coord;
}

bool ContentWindowController::_targetIsFullscreen() const
{
    return _target == Coordinates::FULLSCREEN ||
            ( _target == Coordinates::AUTO && _contentWindow.isFullscreen( ));
}

const QRectF& ContentWindowController::_getCoordinates() const
{
    switch( _target )
    {
    case ContentWindowController::Coordinates::STANDARD:
        return _contentWindow.getCoordinates();
    case ContentWindowController::Coordinates::FOCUSED:
        return _contentWindow.getFocusedCoordinates();
    case ContentWindowController::Coordinates::FULLSCREEN:
        return _contentWindow.getFullscreenCoordinates();
    case ContentWindowController::Coordinates::AUTO:
    default:
        return _contentWindow.getDisplayCoordinates();
    }
}

void ContentWindowController::_apply( const QRectF& coordinates )
{
    switch( _target )
    {
    case ContentWindowController::Coordinates::STANDARD:
        _contentWindow.setCoordinates( coordinates );
        break;
    case ContentWindowController::Coordinates::FOCUSED:
        _contentWindow.setFocusedCoordinates( coordinates );
        break;
    case ContentWindowController::Coordinates::FULLSCREEN:
        _contentWindow.setFullscreenCoordinates( coordinates );
        break;
    case ContentWindowController::Coordinates::AUTO:
        _contentWindow.setDisplayCoordinates( coordinates );
        break;
    }
}

/*********************************************************************/
/* Copyright (c) 2014-2015, EPFL/Blue Brain Project                  */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#include "ContentWindowController.h"

#include "ContentWindow.h"
#include "DisplayGroup.h"
#include "ZoomInteractionDelegate.h"

#include <QTransform>

namespace
{
const qreal MIN_SIZE = 0.05;
const qreal MIN_VISIBLE_AREA_PX = 300.0;
const qreal LARGE_SIZE_SCALE = 0.75;
const qreal FITTING_SIZE_SCALE = 0.9;
const qreal ONE_PERCENT = 0.01;
}

ContentWindowController::ContentWindowController()
    : _contentWindow( 0 )
    , _displayGroup( 0 )
{}

ContentWindowController::ContentWindowController( ContentWindow& contentWindow,
                                                  const DisplayGroup& displayGroup )
    : _contentWindow( &contentWindow )
    , _displayGroup( &displayGroup )
{}

void ContentWindowController::resize( const QSizeF size,
                                      const WindowPoint fixedPoint )
{
    QSizeF newSize( _contentWindow->getContent()->getDimensions( ));
    if( newSize.isEmpty( ))
        newSize = size;
    else
        newSize.scale( size, Qt::KeepAspectRatio );

    switch( fixedPoint )
    {
    case CENTER:
        _resize( _contentWindow->getCoordinates().center(), newSize );
        break;
    case TOP_LEFT:
    default:
        _resize( _contentWindow->getCoordinates().topLeft(), newSize );
    }
}

void ContentWindowController::resizeRelative( const QPointF& delta )
{
    const QRectF& coord = _contentWindow->getCoordinates();

    QPointF fixedPoint;
    QSizeF newSize = coord.size();

    switch( _contentWindow->getActiveHandle( ))
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
    if( _contentWindow->getResizePolicy() == ContentWindow::KEEP_ASPECT_RATIO )
    {
        if( _contentWindow->getContent()->getZoomRect() == UNIT_RECTF )
            _constrainAspectRatio( newSize );
        if( _isCloseToContentAspectRatio( newSize ))
            _snapToContentAspectRatio( newSize );
    }

    _resize( fixedPoint, newSize );
}

void ContentWindowController::scale( const QPointF& center,
                                     const double pixelDelta )
{
    QSizeF newSize = _contentWindow->getCoordinates().size();
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
        resize( _contentWindow->getContent()->getPreferredDimensions(), CENTER);
        break;

    case SIZE_1TO1_FITTING:
    {
        const QSizeF max = _displayGroup->getCoordinates().size() *
                           FITTING_SIZE_SCALE;
        const QSize oneToOne =
                _contentWindow->getContent()->getPreferredDimensions();
        if( oneToOne.width() > max.width() || oneToOne.height() > max.height( ))
            resize( max, CENTER );
        else
            resize( oneToOne, CENTER );
    } break;

    case SIZE_LARGE:
    {
        const QSizeF wallSize = _displayGroup->getCoordinates().size();
        resize( LARGE_SIZE_SCALE * wallSize, CENTER );
    } break;

    case SIZE_FULLSCREEN:
    {
        _contentWindow->getContent()->resetZoom();
        QSizeF size = _contentWindow->getContent()->getDimensions();
        size.scale( _displayGroup->getCoordinates().size(),
                    Qt::KeepAspectRatio );
        constrainSize( size );
        const auto fullscreenCoordinates = _getCenteredCoordinates( size );
        _contentWindow->setFullscreenCoordinates( fullscreenCoordinates );
    } break;

    default:
        return;
    }
}

void ContentWindowController::moveTo( const QPointF& position,
                                      const WindowPoint handle )
{
    QRectF coordinates( _contentWindow->getCoordinates( ));
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

    _contentWindow->setCoordinates( coordinates );
}

QSizeF ContentWindowController::getMinSize() const
{
    const QSizeF& minContentSize =
            _contentWindow->getContent()->getMinDimensions();
    const QSizeF& wallSize = _displayGroup->getCoordinates().size();
    const QSizeF minSize(
                std::max( MIN_SIZE * wallSize.width(), MIN_VISIBLE_AREA_PX ),
                std::max( MIN_SIZE * wallSize.height(), MIN_VISIBLE_AREA_PX ));
    return std::max( minContentSize, minSize );
}

QSizeF ContentWindowController::getMaxSize() const
{
    const QRectF& zoomRect = _contentWindow->getContent()->getZoomRect();
    QSizeF maxSize = getMaxContentSize();
    maxSize.rwidth() *= zoomRect.size().width();
    maxSize.rheight() *= zoomRect.size().height();
    return maxSize;
}

QSizeF ContentWindowController::getMaxContentSize() const
{
    QSizeF maxContentSize = _contentWindow->getContent()->getMaxDimensions();
    if( maxContentSize.isValid( ))
        return maxContentSize;

    maxContentSize = _contentWindow->getContent()->getDimensions();
    return maxContentSize.scaled( _displayGroup->getCoordinates().size(),
                                  Qt::KeepAspectRatioByExpanding );
}

void ContentWindowController::constrainSize( QSizeF& windowSize ) const
{
    const QSizeF& maxSize = getMaxSize();
    if( windowSize > maxSize )
    {
        windowSize.scale( maxSize, Qt::KeepAspectRatio );
        return;
    }

    const QSizeF& minSize = getMinSize();
    if( windowSize < minSize )
        windowSize.scale( minSize, Qt::KeepAspectRatioByExpanding );
}

QRectF
ContentWindowController::scaleRectAroundPosition( const QRectF& rect,
                                                  const QPointF& position,
                                                  const QSizeF& size )
{
    QTransform transform;
    transform.translate( position.x(), position.y( ));
    transform.scale( size.width() / rect.width(),
                     size.height() / rect.height( ));
    transform.translate( -position.x(), -position.y( ));

    return transform.mapRect( rect );
}

void ContentWindowController::_resize( const QPointF& center, QSizeF size )
{
    constrainSize( size );

    QRectF coordinates( _contentWindow->getCoordinates( ));
    coordinates = scaleRectAroundPosition( coordinates, center, size );
    _constrainPosition( coordinates );

    _contentWindow->setCoordinates( coordinates );

    auto zoomDelegate = dynamic_cast<ZoomInteractionDelegate*>(
                            _contentWindow->getInteractionDelegate( ));
    if( zoomDelegate )
        zoomDelegate->adjustZoomToContentAspectRatio();
}

void ContentWindowController::_constrainAspectRatio( QSizeF& windowSize ) const
{
    const QSizeF currentSize = _contentWindow->getCoordinates().size();
    const auto mode = windowSize < currentSize ?
                           Qt::KeepAspectRatio : Qt::KeepAspectRatioByExpanding;
    windowSize = currentSize.scaled( windowSize, mode );
}

bool ContentWindowController::_isCloseToContentAspectRatio( const QSizeF&
                                                            windowSize ) const
{
    const qreal windowAR = windowSize.width() / windowSize.height();
    const qreal contentAR = _contentWindow->getContent()->getAspectRatio();

    return std::fabs( windowAR - contentAR ) < ONE_PERCENT;
}

void ContentWindowController::_snapToContentAspectRatio( QSizeF&
                                                         windowSize ) const
{
    const QSizeF contentSize( _contentWindow->getContent()->getDimensions( ));
    const auto mode = windowSize < contentSize ?
                       Qt::KeepAspectRatio : Qt::KeepAspectRatioByExpanding;
    windowSize = contentSize.scaled( windowSize, mode );
}

void ContentWindowController::_constrainPosition( QRectF& window ) const
{
    const QRectF& group = _displayGroup->getCoordinates();

    const qreal minX = MIN_VISIBLE_AREA_PX - window.width();
    const qreal minY = MIN_VISIBLE_AREA_PX - window.height();

    const qreal maxX = group.width() - MIN_VISIBLE_AREA_PX;
    const qreal maxY = group.height() - MIN_VISIBLE_AREA_PX;

    const QPointF position( std::max( minX, std::min( window.x(), maxX )),
                            std::max( minY, std::min( window.y(), maxY )));

    window.moveTopLeft( position );
}

QRectF
ContentWindowController::_getCenteredCoordinates( const QSizeF& size ) const
{
    const QRectF& group = _displayGroup->getCoordinates();

    // centered coordinates on the display group
    QRectF coord( QPointF(), size );
    coord.moveCenter( group.center( ));
    return coord;
}

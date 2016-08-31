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

#include "MultitouchArea.h"

#include <QQuickWindow>
#include <cmath>

namespace
{
const qreal wheelFactor = 0.3;
const qreal defaultPanThresholdPx = 20;
const qreal pinchThresholdPx = 10;
const qreal swipeMaxFingersIntervalPx = 120;
const qreal swipeThresholdPx = 80;
const qreal doubleTapThresholdPx = 40;
const uint tapAndHoldTimeout = 1000;
const uint doubleTapTimeout = 750;
}

MultitouchArea::MultitouchArea( QQuickItem* parent_ )
    : QQuickItem( parent_ )
    , _panThreshold( defaultPanThresholdPx )
{
    setAcceptedMouseButtons( Qt::LeftButton );

    _tapAndHoldTimer.setInterval( tapAndHoldTimeout );
    connect( &_tapAndHoldTimer, &QTimer::timeout, [this]() {
        emit tapAndHold( _getTouchCenterStartPos(), _getPointsCount( ));
    });
}

QQuickItem* MultitouchArea::getReferenceItem() const
{
    return _referenceItem;
}

qreal MultitouchArea::getPanThreshold() const
{
    return _panThreshold;
}

void MultitouchArea::setReferenceItem( QQuickItem* arg )
{
    if( _referenceItem == arg )
        return;

    _referenceItem = arg;
    emit referenceItemChanged( arg );
}

void MultitouchArea::setPanThreshold( const qreal arg )
{
    if( _panThreshold == arg )
        return;

    _panThreshold = arg;
    emit panThresholdChanged( arg );
}

QPointF MultitouchArea::_getScenePos( const QMouseEvent& mouse )
{
    return mapToItem( _referenceItem, mouse.localPos( ));
}

QPointF MultitouchArea::_getScenePos( const QTouchEvent::TouchPoint& point )
{
    return mapToItem( _referenceItem, point.pos( ));
}

QPointF MultitouchArea::_getScenePos( const QWheelEvent& wheel )
{
    return mapToItem( _referenceItem, wheel.posF( ));
}

uint _getButtonsCount( const QMouseEvent& mouse )
{
    uint count = 0;
    if( mouse.buttons() & Qt::LeftButton )
        ++count;
    if( mouse.buttons() & Qt::RightButton )
        ++count;
    if( mouse.buttons() & Qt::MiddleButton )
        ++count;
    return count;
}

void MultitouchArea::mousePressEvent( QMouseEvent* mouse )
{
    if( _getButtonsCount( *mouse ) == 1 )
        emit touchStarted( _getScenePos( *mouse ));

     _handleMultipointGestures( _getPositions( *mouse ));
}

void MultitouchArea::mouseMoveEvent( QMouseEvent* mouse )
{
    _handleMultipointGestures( _getPositions( *mouse ));
}

void MultitouchArea::mouseReleaseEvent( QMouseEvent* mouse )
{
    const auto count = _getButtonsCount( *mouse );
    const auto pos = _getScenePos( *mouse );

    if( count == 0 && !_panning )
        emit tap( pos );

    _handleMultipointGestures( _getPositions( *mouse ));

    if( count == 0 )
        emit touchEnded( pos );
}

void MultitouchArea::mouseDoubleClickEvent( QMouseEvent* mouse )
{
    emit doubleTap( _getScenePos( *mouse ));
}

void MultitouchArea::wheelEvent( QWheelEvent* wheel )
{
    emit pinchStarted();
    const auto delta = wheel->angleDelta();
    emit pinch( _getScenePos( *wheel ), QPointF{ delta } * wheelFactor );
    emit pinchEnded();
}

void MultitouchArea::touchEvent( QTouchEvent* touch )
{
    const auto& points = touch->touchPoints();

    if( touch->type() == QEvent::TouchBegin )
        emit touchStarted( _getScenePos( points.at( 0 )));

    switch( points.size( ))
    {
    case 1:
        _handleSinglePoint( points.at( 0 ));
        break;
    case 2:
        _handleTwoPoints( points.at( 0 ), points.at( 1 ));
        break;
    default:
        break;
    }
    _handleMultipointGestures( _getPositions( points ));

    if( touch->type() == QEvent::TouchEnd )
        emit touchEnded( _getScenePos( points.at( 0 )));
}

MultitouchArea::Positions
MultitouchArea::_getPositions( const QMouseEvent& mouse )
{
    return Positions{ _getButtonsCount( mouse ), _getScenePos( mouse ) };
}

MultitouchArea::Positions
MultitouchArea::_getPositions( const TouchPoints& points )
{
    Positions positions;
    for( auto point : points )
    {
        if( point.state() != Qt::TouchPointReleased )
            positions.push_back( _getScenePos( point ));
    }
    return positions;
}

void MultitouchArea::_handleSinglePoint( const QTouchEvent::TouchPoint& point )
{
    const auto pos = _getScenePos( point );

    switch( point.state( ))
    {
    case Qt::TouchPointPressed:
        _updateDoubleTapGesture( pos );
        break;
    case Qt::TouchPointStationary:
        break;
    case Qt::TouchPointMoved:
        break;
    case Qt::TouchPointReleased:
        if( !_panning )
             emit tap( pos );
        break;
    }
}

void MultitouchArea::_startDoubleTapGesture( const QPointF& pos )
{
    _tapStartPos = pos;
    _doubleTapTimer.start();
}

void MultitouchArea::_updateDoubleTapGesture( const QPointF& pos )
{
    if( _doubleTapTimer.isActive( ))
    {
        if( (pos - _tapStartPos).manhattanLength() < doubleTapThresholdPx )
            emit doubleTap( pos );
        _cancelDoubleTapGesture();
    }
    else
        _startDoubleTapGesture( pos );
}

void MultitouchArea::_cancelDoubleTapGesture()
{
    _doubleTapTimer.stop();
}

qreal _getDist( const QPointF& p0, const QPointF& p1 )
{
    const QPointF dist = p1 - p0;
    return std::sqrt( QPointF::dotProduct( dist, dist ));
}

QPointF _getCenter( const QPointF& pos0, const QPointF& pos1 )
{
    return ( pos0 + pos1 ) / 2;
}

QRectF _makeBoundingRect( const QPointF& pos0, const QPointF& pos1 )
{
    QRectF rect;
    rect.setLeft( std::min( pos0.x(), pos1.x( )));
    rect.setRight( std::max( pos0.x(), pos1.x( )));
    rect.setTop( std::min( pos0.y(), pos1.y( )));
    rect.setBottom( std::max( pos0.y(), pos1.y( )));
    return rect;
}

void MultitouchArea::_handleTwoPoints( const QTouchEvent::TouchPoint& p0,
                                       const QTouchEvent::TouchPoint& p1 )
{
    if( p0.state() == Qt::TouchPointReleased ||
        p1.state() == Qt::TouchPointReleased )
    {
        _cancelPinchGesture();
        _cancelSwipeGesture();
        return;
    }

    const auto pos0 = _getScenePos( p0 );
    const auto pos1 = _getScenePos( p1 );

    if( p0.state() == Qt::TouchPointPressed ||
        p1.state() == Qt::TouchPointPressed )
    {
        _initPinchGesture( pos0, pos1 );
        _initSwipeGesture( pos0, pos1 );
    }

    _updatePinchGesture( pos0, pos1 );
    _updateSwipeGesture( pos0, pos1 );
}

void MultitouchArea::_initPinchGesture( const QPointF& pos0,
                                        const QPointF& pos1 )
{
    const auto twoFingersStartRect = _makeBoundingRect( pos0, pos1 );
    const auto w = twoFingersStartRect.width();
    const auto h = twoFingersStartRect.height();
    _initialPinchDist = std::sqrt( w * w + h * h );
}

void MultitouchArea::_startPinchGesture( const QPointF& pos0,
                                         const QPointF& pos1 )
{
    if( _pinching )
        return;

    _pinching = true;
    _lastPinchRect = _makeBoundingRect( pos0, pos1 );
    emit pinchStarted();
}

void MultitouchArea::_updatePinchGesture( const QPointF& pos0,
                                          const QPointF& pos1 )
{
    if( !_pinching )
    {
        const auto pinchDist = _getDist( pos0, pos1 );
        const auto pinchDelta = std::abs( pinchDist - _initialPinchDist );
        if( pinchDelta > pinchThresholdPx )
            _startPinchGesture( pos0, pos1 );
        else
            return;
    }

    const auto pinchRect = _makeBoundingRect( pos0, pos1 );
    const auto pinchDelta = pinchRect.size() - _lastPinchRect.size();
    _lastPinchRect = pinchRect;
    emit pinch( pinchRect.center(), QPointF{ pinchDelta.width(),
                                             pinchDelta.height() });
}

void MultitouchArea::_cancelPinchGesture()
{
    if( !_pinching )
        return;

    _pinching = false;
    emit pinchEnded();
}

bool _checkFingersDistanceForSwipe( const QPointF& pos0, const QPointF& pos1 )
{
    const qreal fingersInterval = _getDist( pos0, pos1 );
    return fingersInterval < swipeMaxFingersIntervalPx;
}

void MultitouchArea::_initSwipeGesture( const QPointF& pos0,
                                        const QPointF& pos1 )
{
    _canBeSwipe = _checkFingersDistanceForSwipe( pos0, pos1 );
    _swipeStartPos = _getCenter( pos0, pos1 );
}

void MultitouchArea::_updateSwipeGesture( const QPointF& pos0,
                                          const QPointF& pos1 )
{
    if( !_canBeSwipe )
        return;

    if( !_checkFingersDistanceForSwipe( pos0, pos1 ))
    {
        _cancelSwipeGesture();
        return;
    }

    const auto twoFingersPos = _getCenter( pos0, pos1 );
    const auto twoFingersStartPos = _swipeStartPos;
    const auto dist = _getDist( twoFingersStartPos, twoFingersPos );
    if( dist > swipeThresholdPx )
    {
        const qreal dx = twoFingersPos.x() - twoFingersStartPos.x();
        const qreal dy = twoFingersPos.y() - twoFingersStartPos.y();

        if( std::abs( dx ) > std::abs( dy ))
        {
            // Horizontal swipe
            if( dx > 0.0 )
                emit swipeRight();
            else
                emit swipeLeft();
        }
        else
        {
            // Vertical swipe
            if( dy > 0.0 )
                emit swipeDown();
            else
                emit swipeUp();
        }
        _cancelSwipeGesture(); // Only allow one swipe
    }
}

void MultitouchArea::_cancelSwipeGesture()
{
    _canBeSwipe = false;
}

void MultitouchArea::_handleMultipointGestures( const Positions& positions )
{
    if( positions.size() != _touchStartPos.size( ))
    {
        _cancelTapAndHoldGesture();
        _cancelPanGesture();

        _touchStartPos = positions;

        if( !positions.empty( ))
            _startTapAndHoldGesture();
        return;
    }

    _updateTapAndHoldGesture( positions );
    _updatePanGesture( positions );
}

void MultitouchArea::_startTapAndHoldGesture()
{
    _tapAndHoldTimer.start();
}

void MultitouchArea::_updateTapAndHoldGesture( const Positions& positions )
{
    if( _tapAndHoldTimer.isActive( ))
        _cancelTapAndHoldIfMoved( positions );
}

void MultitouchArea::_cancelTapAndHoldIfMoved( const Positions& positions )
{
    size_t i = 0;
    for( const auto& pos : positions )
    {
        if( (pos - _touchStartPos[i++]).manhattanLength() > _panThreshold )
        {
            _cancelTapAndHoldGesture();
            return;
        }
    }
}

void MultitouchArea::_cancelTapAndHoldGesture()
{
    _tapAndHoldTimer.stop();
}

template<class Container>
QPointF _computeCenter( const Container& positions )
{
    QPointF center;
    for( const auto& pos : positions )
        center += pos;
    return center / positions.size();
}

QPointF MultitouchArea::_getTouchCenterStartPos() const
{
    return _computeCenter( _touchStartPos );
}

uint MultitouchArea::_getPointsCount() const
{
    return _touchStartPos.size();
}

void MultitouchArea::_startPanGesture( const QPointF& pos )
{
    if( _panning )
        return;

    _panning = true;
    emit panStarted( pos, _getPointsCount( ));
}

void MultitouchArea::_updatePanGesture( const Positions& positions )
{
    const auto pos = _computeCenter( positions );
    const auto start = _getTouchCenterStartPos();

    if( !_panning && (pos - start).manhattanLength() > _panThreshold )
    {
        _startPanGesture( pos );
        _lastPanPos = pos;
    }
    if( _panning )
    {
        emit pan( pos, pos - _lastPanPos, positions.size( ));
        _lastPanPos = pos;
    }
}

void MultitouchArea::_cancelPanGesture()
{
    if( !_panning )
        return;

    _panning = false;
    emit panEnded();
}

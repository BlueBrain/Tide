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

#ifndef MULTITOUCHAREA_H
#define MULTITOUCHAREA_H

#include <QQuickItem>

#include <QTimer>

/**
 * A multipoint touch area to detect touch gestures on Qml objects.
 */
class MultitouchArea : public QQuickItem
{
    Q_OBJECT

    /** Set a target item to enable the touch area to move it. */
    Q_PROPERTY( QQuickItem* referenceItem READ getReferenceItem
                WRITE setReferenceItem NOTIFY referenceItemChanged )

    /** The minimum displacement until a gesture is considered as a pan. */
    Q_PROPERTY( qreal panThreshold READ getPanThreshold WRITE setPanThreshold
                NOTIFY panThresholdChanged )

public:
    /** Constructor. */
    MultitouchArea( QQuickItem* parent = 0 );

    /** @name Q_PROPERTY getters. */
    //@{
    QQuickItem* getReferenceItem() const;
    qreal getPanThreshold() const;
    //@}

public slots:
    /** @name Q_PROPERTY setters. */
    //@{
    void setReferenceItem( QQuickItem* arg );
    void setPanThreshold( qreal arg );
    //@}

signals:
    /** @name Q_PROPERTY notifiers. */
    //@{
    void referenceItemChanged( QQuickItem* arg );
    void panThresholdChanged( qreal arg );
    //@}

    /** @name Basic touch events. */
    //@{
    /** Always emitted for the first finger that touches the area. */
    void touchStarted( QPointF pos );

    /** Always emitted for the last finger that is removed from the area. */
    void touchEnded( QPointF pos );
    //@}

    /** @name Basic one-finger tap events. */
    //@{
    /** Emitted for a one-finger touch and release in-place (i.e. not a pan). */
    void tap( QPointF pos );

    /** Emitted when two taps occur in a fast sequence. */
    void doubleTap( QPointF pos );
    //@}

    /** @name Two-fingers gestures. */
    //@{
    /** Emitted when a pinch starts (i.e. two fingers start moving apart). */
    void pinchStarted();

    /** Emitted for each step of a two-fingers pinch gesture. */
    void pinch( QPointF pos, QPointF pixelDelta );

    /** Emitted when a pinch ends (i.e. one of the two fingers is released). */
    void pinchEnded();

    /** Two-fingers swipe to the left. */
    void swipeLeft();
    /** Two-fingers swipe to the right. */
    void swipeRight();
    /** Two-fingers swipe up. */
    void swipeUp();
    /** Two-fingers swipe down. */
    void swipeDown();
    //@}

    /** @name Multi-finger gestures. */
    //@{
    /** Emitted after a prolonged non-moving touch with one or more fingers. */
    void tapAndHold( QPointF pos, uint numPoints );

    /** Emitted when a pan starts (i.e. one or more finger(s) start moving). */
    void panStarted( QPointF pos, uint numPoints );

    /** Emitted for each finger movement between panStarted-panEnded. */
    void pan( QPointF pos, QPointF delta, uint numPoints );

    /** Emitted when a pan ends (finger released or new finger detected). */
    void panEnded();
    //@}

private:
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseDoubleClickEvent( QMouseEvent* event ) override;
    void wheelEvent( QWheelEvent* event ) override;
    void touchEvent( QTouchEvent* event ) override;

    using Positions = std::vector<QPointF>;
    using TouchPoints = QList<QTouchEvent::TouchPoint>;

    Positions _getPositions( const QMouseEvent& mouse );
    Positions _getPositions( const TouchPoints& points );

    QPointF _getScenePos( const QMouseEvent& mouse );
    QPointF _getScenePos( const QTouchEvent::TouchPoint& point );
    QPointF _getScenePos( const QWheelEvent& wheel );

    /** @name Single-point gestures */
    //@{
    void _handleSinglePoint( const QTouchEvent::TouchPoint& point );

    void _startDoubleTapGesture( const QPointF& pos );
    void _updateDoubleTapGesture( const QPointF& pos );
    void _cancelDoubleTapGesture();
    //@}

    /** @name Two-point gestures */
    //@{
    void _handleTwoPoints( const QTouchEvent::TouchPoint& p0,
                           const QTouchEvent::TouchPoint& p1 );

    void _initPinchGesture( const QPointF& pos0, const QPointF& pos1 );
    void _startPinchGesture( const QPointF& pos0, const QPointF& pos1 );
    void _updatePinchGesture( const QPointF& pos0, const QPointF& pos1 );
    void _cancelPinchGesture();

    void _initSwipeGesture( const QPointF& pos0, const QPointF& pos1 );
    void _updateSwipeGesture( const QPointF& pos0, const QPointF& pos1 );
    void _cancelSwipeGesture();
    //@}

    /** @name Mulit-point gestures */
    //@{
    void _handleMultipointGestures( const Positions& positions );

    void _startTapAndHoldGesture();
    void _updateTapAndHoldGesture( const Positions& positions );
    void _cancelTapAndHoldIfMoved( const Positions& positions );
    void _cancelTapAndHoldGesture();

    QPointF _getTouchCenterStartPos() const;
    uint _getPointsCount() const;

    void _startPanGesture( const QPointF& pos );
    void _updatePanGesture( const Positions& positions );
    void _cancelPanGesture();
    //@}

    QQuickItem* _referenceItem = nullptr;

    /** @name Single-point gestures */
    //@{
    QPointF _tapStartPos;
    QTimer _doubleTapTimer;
    //@}

    /** @name Two-point gestures */
    //@{
    bool _pinching = false;
    qreal _initialPinchDist = 0.0;
    QRectF _lastPinchRect;

    bool _canBeSwipe = false;
    QPointF _swipeStartPos;
    //@}

    /** @name Mulit-point gestures */
    //@{
    Positions _touchStartPos;

    QTimer _tapAndHoldTimer;

    qreal _panThreshold;
    bool _panning = false;
    QPointF _lastPanPos;
    //@}
};

#endif

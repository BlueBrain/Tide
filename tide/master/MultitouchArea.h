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
    MultitouchArea( QQuickItem* parent = 0 );

    QQuickItem* getReferenceItem() const;
    qreal getPanThreshold() const;

public slots:
    void setReferenceItem( QQuickItem* arg );
    void setPanThreshold( qreal arg );

signals:
    void referenceItemChanged( QQuickItem* arg );
    void panThresholdChanged( qreal arg );

    void tapStarted( QPointF pos );
    void tapEnded( QPointF pos );

    void doubleTap( QPointF pos );

    void tapAndHold( QPointF pos );

    void panStarted( QPointF pos );
    void pan( QPointF pos, QPointF delta );
    void panEnded();

    void pinch( QPointF pos, qreal pixelDelta );

    void swipeLeft();
    void swipeRight();
    void swipeUp();
    void swipeDown();

private:
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseDoubleClickEvent( QMouseEvent* event ) override;
    void wheelEvent( QWheelEvent* event ) override;
    void touchEvent( QTouchEvent* event ) override;

    QPointF _getScenePos( QMouseEvent* mouse );
    QPointF _getScenePos( const QTouchEvent::TouchPoint& point );
    QPointF _getScenePos( const QPointF& itemPos );

    qreal _getPinchDistance( const QTouchEvent::TouchPoint& p0,
                             const QTouchEvent::TouchPoint& p1 );
    QPointF _getCenter( const QTouchEvent::TouchPoint& p0,
                        const QTouchEvent::TouchPoint& p1 );

    void _handleSinglePoint( const QTouchEvent::TouchPoint& point );

    void _startPanGesture( const QPointF& pos );
    void _cancelPanGesture();

    void _startTapAndHoldGesture();
    void _cancelTapAndHoldGesture();

    void _startDoubleTapGesture();
    void _cancelDoubleTapGesture();

    void _handleTwoPoints( const QTouchEvent::TouchPoint& p0,
                           const QTouchEvent::TouchPoint& p1 );

    QQuickItem* _referenceItem;
    qreal _panThreshold;

    QPointF _mousePrevPos;

    bool _panning;
    QPointF _lastPanPos;

    bool _twoFingersDetectionStarted;
    bool _canBeSwipe;
    bool _pinching;
    qreal _lastPinchDist;
    QPointF _twoFingersStartPos;

    QPointF _tapStartPos;
    uint _tapCounter;
    qreal _initialPinchDist;

    QTimer _tapAndHoldTimer;
    QTimer _doubleTapTimer;
};

#endif

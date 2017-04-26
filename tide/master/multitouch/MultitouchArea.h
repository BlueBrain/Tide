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

#include "multitouch/DoubleTapDetector.h"
#include "multitouch/PanDetector.h"
#include "multitouch/PinchDetector.h"
#include "multitouch/SwipeDetector.h"
#include "multitouch/TapAndHoldDetector.h"
#include "multitouch/TapDetector.h"

/**
 * A multipoint touch area to detect touch gestures on Qml objects.
 */
class MultitouchArea : public QQuickItem
{
    Q_OBJECT

    /** Set a target item to enable the touch area to move it. */
    Q_PROPERTY(QQuickItem* referenceItem READ getReferenceItem WRITE
                   setReferenceItem NOTIFY referenceItemChanged)

    /** The minimum displacement until a gesture is considered as a pan. */
    Q_PROPERTY(qreal panThreshold READ getPanThreshold WRITE setPanThreshold
                   NOTIFY panThresholdChanged)

public:
    /** Constructor. */
    MultitouchArea(QQuickItem* parent = 0);

    /** @name Q_PROPERTY getters. */
    //@{
    QQuickItem* getReferenceItem() const;
    qreal getPanThreshold() const;
    //@}

public slots:
    /** @name Q_PROPERTY setters. */
    //@{
    void setReferenceItem(QQuickItem* arg);
    void setPanThreshold(qreal arg);
    //@}

signals:
    /** @name Q_PROPERTY notifiers. */
    //@{
    void referenceItemChanged(QQuickItem* arg);
    void panThresholdChanged(qreal arg);
    //@}

    /** @name Basic touch events. */
    //@{
    /** Always emitted for the first finger that touches the area. */
    void touchStarted(QPointF pos);

    /** Always emitted for the last finger that is removed from the area. */
    void touchEnded(QPointF pos);

    /** Emitted when a new touch point is added. */
    void touchPointAdded(int id, QPointF pos);

    /** Emitted when an existing touch point is updated. */
    void touchPointUpdated(int id, QPointF pos);

    /** Emitted when an existing touch point is removed. */
    void touchPointRemoved(int id, QPointF pos);
    //@}

    /** @name Two-finger gestures. */
    //@{
    /** @copydoc PinchDetector::pinchStarted */
    void pinchStarted();

    /** @copydoc PinchDetector::pinch */
    void pinch(QPointF pos, QPointF pixelDelta);

    /** @copydoc PinchDetector::pinchEnded */
    void pinchEnded();

    /** @copydoc SwipeDetector::swipeLeft */
    void swipeLeft();
    /** @copydoc SwipeDetector::swipeRight */
    void swipeRight();
    /** @copydoc SwipeDetector::swipeUp */
    void swipeUp();
    /** @copydoc SwipeDetector::swipeDown */
    void swipeDown();
    //@}

    /** @name Multi-finger gestures. */
    //@{
    /** @copydoc TapDetector::tap */
    void tap(QPointF pos, uint numPoints);

    /** @copydoc DoubleTapDetector::doubleTap */
    void doubleTap(QPointF pos, uint numPoints);

    /** @copydoc TapAndHoldDetector::tapAndHold */
    void tapAndHold(QPointF pos, uint numPoints);

    /** @copydoc PanDetector::panStarted */
    void panStarted(QPointF pos, uint numPoints);

    /** @copydoc PanDetector::pan */
    void pan(QPointF pos, QPointF delta, uint numPoints);

    /** @copydoc PanDetector::panEnded */
    void panEnded();
    //@}

private:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void touchEvent(QTouchEvent* event) override;

    using TouchPoints = QList<QTouchEvent::TouchPoint>;

    Positions _getPositions(const QMouseEvent& mouse);
    Positions _getPositions(const TouchPoints& points);

    QPointF _getScenePos(const QMouseEvent& mouse);
    QPointF _getScenePos(const QTouchEvent::TouchPoint& point);
    QPointF _getScenePos(const QWheelEvent& wheel);

    void _handleGestures(const Positions& positions);
    void _handleTouch(const TouchPoints& points);

    QQuickItem* _referenceItem = nullptr;

    DoubleTapDetector _doubleTapDetector;
    PanDetector _panDetector;
    PinchDetector _pinchDetector;
    SwipeDetector _swipeDetector;
    TapAndHoldDetector _tapAndHoldDetector;
    TapDetector _tapDetector;

    Positions _touchStartPos;
};

#endif

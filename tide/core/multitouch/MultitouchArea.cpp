/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "MultitouchArea.h"

#include "multitouch/MathUtils.h"

namespace
{
const qreal wheelFactor = 0.3;
const qreal defaultPanThresholdPx = 20;
const qreal pinchThresholdPx = 10;
const qreal swipeMaxFingersIntervalPx = 120;
const qreal swipeThresholdPx = 80;
const qreal doubleTapThresholdPx = 40;
const uint doubleTapTimeoutMs = 750;
const uint tapAndHoldTimeoutMs = 1000;
}

MultitouchArea::MultitouchArea(QQuickItem* parent_)
    : QQuickItem(parent_)
    , _doubleTapDetector(doubleTapThresholdPx, doubleTapTimeoutMs)
    , _panDetector(defaultPanThresholdPx)
    , _pinchDetector(pinchThresholdPx)
    , _swipeDetector(swipeMaxFingersIntervalPx, swipeThresholdPx)
    , _tapAndHoldDetector(tapAndHoldTimeoutMs, defaultPanThresholdPx)
    , _tapDetector(defaultPanThresholdPx)
{
    setAcceptedMouseButtons(Qt::LeftButton);

    connect(&_doubleTapDetector, &DoubleTapDetector::doubleTap, this,
            &MultitouchArea::doubleTap);

    connect(&_panDetector, &PanDetector::panStarted, this,
            &MultitouchArea::panStarted);
    connect(&_panDetector, &PanDetector::pan, this, &MultitouchArea::pan);
    connect(&_panDetector, &PanDetector::panEnded, this,
            &MultitouchArea::panEnded);

    connect(&_pinchDetector, &PinchDetector::pinchStarted, this,
            &MultitouchArea::pinchStarted);
    connect(&_pinchDetector, &PinchDetector::pinch, this,
            &MultitouchArea::pinch);
    connect(&_pinchDetector, &PinchDetector::pinchEnded, this,
            &MultitouchArea::pinchEnded);

    connect(&_swipeDetector, &SwipeDetector::swipeDown, this,
            &MultitouchArea::swipeDown);
    connect(&_swipeDetector, &SwipeDetector::swipeLeft, this,
            &MultitouchArea::swipeLeft);
    connect(&_swipeDetector, &SwipeDetector::swipeRight, this,
            &MultitouchArea::swipeRight);
    connect(&_swipeDetector, &SwipeDetector::swipeUp, this,
            &MultitouchArea::swipeUp);

    connect(&_tapAndHoldDetector, &TapAndHoldDetector::tapAndHold, this,
            &MultitouchArea::tapAndHold);

    connect(&_tapDetector, &TapDetector::tap,
            [this](const QPointF& pos, const uint numPoints) {
                if (!_blockTap)
                    emit tap(pos, numPoints);
            });

    connect(this, &MultitouchArea::touchStarted, [this] { _blockTap = false; });
    connect(this, &MultitouchArea::tapAndHold, [this] { _blockTap = true; });
}

QQuickItem* MultitouchArea::getReferenceItem() const
{
    return _referenceItem;
}

qreal MultitouchArea::getPanThreshold() const
{
    return _panDetector.getPanThreshold();
}

void MultitouchArea::setReferenceItem(QQuickItem* arg)
{
    if (_referenceItem == arg)
        return;

    _referenceItem = arg;
    emit referenceItemChanged(arg);
}

void MultitouchArea::setPanThreshold(const qreal arg)
{
    if (getPanThreshold() == arg)
        return;

    _panDetector.setPanThreshold(arg);
    emit panThresholdChanged(arg);
}

uint _getButtonsCount(const QMouseEvent& mouse)
{
    uint count = 0;
    if (mouse.buttons() & Qt::LeftButton)
        ++count;
    if (mouse.buttons() & Qt::RightButton)
        ++count;
    if (mouse.buttons() & Qt::MiddleButton)
        ++count;
    return count;
}

void MultitouchArea::mousePressEvent(QMouseEvent* mouse)
{
    const auto buttonsCount = int(_getButtonsCount(*mouse));
    const auto pos = _getScenePos(*mouse);
    if (buttonsCount == 1)
        emit touchStarted(pos);

    emit touchPointAdded(buttonsCount - 1, pos);

    _handleGestures(_getPositions(*mouse));
}

void MultitouchArea::mouseMoveEvent(QMouseEvent* mouse)
{
    const auto buttonsCount = int(_getButtonsCount(*mouse));
    const auto pos = _getScenePos(*mouse);
    for (int i = 0; i < buttonsCount; ++i)
        emit touchPointUpdated(i, pos);

    _handleGestures(_getPositions(*mouse));
}

void MultitouchArea::mouseReleaseEvent(QMouseEvent* mouse)
{
    const auto buttonsCount = int(_getButtonsCount(*mouse));
    const auto pos = _getScenePos(*mouse);

    emit touchPointRemoved(buttonsCount, pos);

    _handleGestures(_getPositions(*mouse));

    if (buttonsCount == 0)
        emit touchEnded(pos);
}

void MultitouchArea::wheelEvent(QWheelEvent* wheel)
{
    emit pinchStarted();
    const auto delta = wheel->angleDelta();
    emit pinch(_getScenePos(*wheel), QPointF{delta} * wheelFactor);
    emit pinchEnded();
}

void MultitouchArea::touchEvent(QTouchEvent* touch)
{
    const auto& points = touch->touchPoints();

    if (touch->type() == QEvent::TouchBegin)
        emit touchStarted(_getScenePos(points.at(0)));

    _handleGestures(_getPositions(points));
    _handleTouch(points);

    if (touch->type() == QEvent::TouchEnd)
        emit touchEnded(_getScenePos(points.at(0)));
}

Positions MultitouchArea::_getPositions(const QMouseEvent& mouse)
{
    return Positions{_getButtonsCount(mouse), _getScenePos(mouse)};
}

Positions MultitouchArea::_getPositions(const TouchPoints& points)
{
    Positions positions;
    for (auto point : points)
    {
        if (point.state() != Qt::TouchPointReleased)
            positions.push_back(_getScenePos(point));
    }
    return positions;
}

QPointF MultitouchArea::_getScenePos(const QMouseEvent& mouse)
{
    return mapToItem(_referenceItem, mouse.localPos());
}

QPointF MultitouchArea::_getScenePos(const QTouchEvent::TouchPoint& point)
{
    return mapToItem(_referenceItem, point.pos());
}

QPointF MultitouchArea::_getScenePos(const QWheelEvent& wheel)
{
    return mapToItem(_referenceItem, wheel.posF());
}

void MultitouchArea::_handleGestures(const Positions& positions)
{
    if (positions.size() != _touchStartPos.size())
    {
        if (positions.size() == 2)
        {
            _pinchDetector.initGesture(positions.at(0), positions.at(1));
            _swipeDetector.initGesture(positions.at(0), positions.at(1));
        }
        else
        {
            _pinchDetector.cancelGesture();
            _swipeDetector.cancelGesture();
        }
        _doubleTapDetector.initGesture(positions);
        _tapAndHoldDetector.initGesture(positions);
        _tapDetector.initGesture(positions);
        _panDetector.initGesture(positions);

        _touchStartPos = positions;
        return;
    }

    if (positions.size() == 2)
    {
        _pinchDetector.updateGesture(positions.at(0), positions.at(1));
        _swipeDetector.updateGesture(positions.at(0), positions.at(1));
    }
    _tapAndHoldDetector.updateGesture(positions);
    _tapDetector.updateGesture(positions);
    _panDetector.updateGesture(positions);
}

void MultitouchArea::_handleTouch(const TouchPoints& points)
{
    for (const auto& point : points)
    {
        switch (point.state())
        {
        case Qt::TouchPointPressed:
            emit touchPointAdded(point.id(), _getScenePos(point));
            break;
        case Qt::TouchPointMoved:
            emit touchPointUpdated(point.id(), _getScenePos(point));
            break;
        case Qt::TouchPointReleased:
            emit touchPointRemoved(point.id(), _getScenePos(point));
            break;
        case Qt::TouchPointStationary:
            break;
        }
    }
}

/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#ifndef MOCKTOUCHEVENTS_H
#define MOCKTOUCHEVENTS_H

#include <QTouchEvent>

const QSize wallSize(1600, 800);
const QRect widget(QPoint(510, 353), QSize(435, 212));

QMap<int, QTouchEvent::TouchPoint> touchPointMap;

QTouchDevice touchDevice;

QEvent* createTouchEvent(
    const int id, const QEvent::Type eventType,
    const Qt::TouchPointState touchPointStates, const QPointF& normalizedPos,
    const QTouchDevice::DeviceType deviceType = QTouchDevice::TouchScreen)
{
    const QPoint scenePos(wallSize.width() * normalizedPos.x(),
                          wallSize.height() * normalizedPos.y());

    const QPoint screenPos(widget.x() + widget.width() * normalizedPos.x(),
                           widget.y() + widget.height() * normalizedPos.y());

    QTouchEvent::TouchPoint touchPoint(id);
    touchPoint.setPressure(1.0);
    touchPoint.setNormalizedPos(normalizedPos);
    touchPoint.setPos(scenePos);
    touchPoint.setScenePos(scenePos);
    touchPoint.setScreenPos(screenPos);

    switch (eventType)
    {
    case QEvent::TouchBegin:
        touchPoint.setStartNormalizedPos(touchPoint.normalizedPos());
        touchPoint.setStartPos(touchPoint.pos());
        touchPoint.setStartScreenPos(touchPoint.screenPos());
        touchPoint.setStartScenePos(touchPoint.scenePos());

        touchPoint.setLastNormalizedPos(touchPoint.normalizedPos());
        touchPoint.setLastPos(touchPoint.pos());
        touchPoint.setLastScreenPos(touchPoint.screenPos());
        touchPoint.setLastScenePos(touchPoint.scenePos());
        break;

    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        const QTouchEvent::TouchPoint& prevPoint = touchPointMap.value(id);
        touchPoint.setStartNormalizedPos(prevPoint.startNormalizedPos());
        touchPoint.setStartPos(prevPoint.startPos());
        touchPoint.setStartScreenPos(prevPoint.startScreenPos());
        touchPoint.setStartScenePos(prevPoint.startScenePos());

        touchPoint.setLastNormalizedPos(prevPoint.normalizedPos());
        touchPoint.setLastPos(prevPoint.pos());
        touchPoint.setLastScreenPos(prevPoint.screenPos());
        touchPoint.setLastScenePos(prevPoint.scenePos());
        break;
    }
    default:;
    }

    touchPointMap.insert(id, touchPoint);

    touchDevice.setType(deviceType);

    QEvent* event = new QTouchEvent(eventType, &touchDevice, Qt::NoModifier,
                                    touchPointStates, touchPointMap.values());

    if (eventType == QEvent::TouchEnd)
        touchPointMap.remove(id);

    return event;
}

#endif

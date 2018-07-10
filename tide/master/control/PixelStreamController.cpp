/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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

#include "PixelStreamController.h"

#include "scene/PixelStreamContent.h"
#include "scene/Window.h"

PixelStreamController::PixelStreamController(Window& window)
    : ContentController(window)
{
    connect(&window, &Window::coordinatesChanged, this,
            &PixelStreamController::_sendSizeChangedEvent);
    connect(&window, &Window::modeChanged, this,
            &PixelStreamController::_sendSizeChangedEvent);

    auto& content = dynamic_cast<PixelStreamContent&>(window.getContent());
    connect(this, &PixelStreamController::notify, &content,
            &PixelStreamContent::notify);
}

void PixelStreamController::_addTouchPoint(const int id,
                                           const QPointF& position)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_TOUCH_ADD;
    deflectEvent.key = id;

    emit notify(deflectEvent);
}

void PixelStreamController::_updateTouchPoint(const int id,
                                              const QPointF& position)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_TOUCH_UPDATE;
    deflectEvent.key = id;

    emit notify(deflectEvent);
}

void PixelStreamController::_removeTouchPoint(const int id,
                                              const QPointF& position)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_TOUCH_REMOVE;
    deflectEvent.key = id;

    emit notify(deflectEvent);
}

void PixelStreamController::_touchBegin(const QPointF& position)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_PRESS;

    emit notify(deflectEvent);
}

void PixelStreamController::_touchEnd(const QPointF& position)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_RELEASE;

    emit notify(deflectEvent);
}

void PixelStreamController::_tap(const QPointF& position, const uint numPoints)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_CLICK;
    deflectEvent.key = numPoints;

    emit notify(deflectEvent);
}

void PixelStreamController::_doubleTap(const QPointF& position,
                                       const uint numPoints)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_DOUBLECLICK;
    deflectEvent.key = numPoints;

    emit notify(deflectEvent);
}

void PixelStreamController::_tapAndHold(const QPointF& position,
                                        const uint numPoints)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_TAP_AND_HOLD;
    deflectEvent.key = numPoints;

    emit notify(deflectEvent);
}

void PixelStreamController::_pan(const QPointF& position, const QPointF& delta,
                                 const uint numPoints)
{
    auto deflectEvent = _getNormEvent(position);
    if (numPoints == 1)
        deflectEvent.type = deflect::Event::EVT_MOVE;
    else
    {
        deflectEvent.type = deflect::Event::EVT_PAN;
        deflectEvent.key = numPoints;
    }

    const auto normDelta = _normalize(delta);
    deflectEvent.dx = normDelta.x();
    deflectEvent.dy = normDelta.y();

    emit notify(deflectEvent);
}

void PixelStreamController::_pinch(const QPointF& position,
                                   const QPointF& delta)
{
    auto deflectEvent = _getNormEvent(position);
    deflectEvent.type = deflect::Event::EVT_PINCH;
    deflectEvent.mouseLeft = false;

    const auto normDelta = _normalize(delta);
    deflectEvent.dx = normDelta.x();
    deflectEvent.dy = normDelta.y();

    emit notify(deflectEvent);
}

deflect::Event _makeSwipeEvent(const deflect::Event::EventType type)
{
    deflect::Event event;
    event.type = type;
    return event;
}

void PixelStreamController::_swipeLeft()
{
    emit notify(_makeSwipeEvent(deflect::Event::EVT_SWIPE_LEFT));
}

void PixelStreamController::_swipeRight()
{
    emit notify(_makeSwipeEvent(deflect::Event::EVT_SWIPE_RIGHT));
}

void PixelStreamController::_swipeUp()
{
    emit notify(_makeSwipeEvent(deflect::Event::EVT_SWIPE_UP));
}

void PixelStreamController::_swipeDown()
{
    emit notify(_makeSwipeEvent(deflect::Event::EVT_SWIPE_DOWN));
}

void PixelStreamController::_prevPage()
{
    swipeLeft();
}

void PixelStreamController::_nextPage()
{
    swipeRight();
}

void PixelStreamController::_keyPress(const int key, const int modifiers,
                                      const QString& text)
{
    deflect::Event deflectEvent;
    deflectEvent.type = deflect::Event::EVT_KEY_PRESS;
    deflectEvent.key = key;
    deflectEvent.modifiers = modifiers;
    strncpy(deflectEvent.text, text.toStdString().c_str(),
            sizeof(deflectEvent.text));

    emit notify(deflectEvent);
}

void PixelStreamController::_keyRelease(const int key, const int modifiers,
                                        const QString& text)
{
    deflect::Event deflectEvent;
    deflectEvent.type = deflect::Event::EVT_KEY_RELEASE;
    deflectEvent.key = key;
    deflectEvent.modifiers = modifiers;
    strncpy(deflectEvent.text, text.toStdString().c_str(),
            sizeof(deflectEvent.text));

    emit notify(deflectEvent);
}

void PixelStreamController::_sendSizeChangedEvent()
{
    const auto win = getWindowSize();

    deflect::Event deflectEvent;
    deflectEvent.type = deflect::Event::EVT_VIEW_SIZE_CHANGED;
    deflectEvent.dx = win.width();
    deflectEvent.dy = win.height();

    emit notify(deflectEvent);
}

QPointF PixelStreamController::_normalize(const QPointF& point) const
{
    const auto window = getWindowSize();
    return QPointF{point.x() / window.width(), point.y() / window.height()};
}

deflect::Event PixelStreamController::_getNormEvent(
    const QPointF& position) const
{
    const auto normalizedPos = _normalize(position);

    deflect::Event deflectEvent;
    deflectEvent.mouseLeft = true;
    deflectEvent.mouseX = normalizedPos.x();
    deflectEvent.mouseY = normalizedPos.y();
    return deflectEvent;
}

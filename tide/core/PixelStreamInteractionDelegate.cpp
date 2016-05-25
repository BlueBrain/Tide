/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#include "PixelStreamInteractionDelegate.h"

#include "ContentWindow.h"

PixelStreamInteractionDelegate::PixelStreamInteractionDelegate( ContentWindow&
                                                                contentWindow )
    : ContentInteractionDelegate( contentWindow )
{
    connect( &contentWindow, &ContentWindow::coordinatesChanged,
             this, &PixelStreamInteractionDelegate::_sendSizeChangedEvent );
    connect( &contentWindow, &ContentWindow::modeChanged,
             this, &PixelStreamInteractionDelegate::_sendSizeChangedEvent );
}

void PixelStreamInteractionDelegate::touchBegin( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_PRESS;

    emit notify( deflectEvent );
}

void PixelStreamInteractionDelegate::touchEnd( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_RELEASE;

    emit notify( deflectEvent );
}

void PixelStreamInteractionDelegate::tap( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_CLICK;

    emit notify( deflectEvent );
}

void PixelStreamInteractionDelegate::doubleTap( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_DOUBLECLICK;

    emit notify( deflectEvent );
}

void PixelStreamInteractionDelegate::tapAndHold( const QPointF position )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_TAP_AND_HOLD;

    emit notify( deflectEvent );
}

void PixelStreamInteractionDelegate::pan( const QPointF position,
                                          const QPointF delta )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_MOVE;

    const QPointF normDelta = getNormalizedPoint( delta );
    deflectEvent.dx = normDelta.x();
    deflectEvent.dy = normDelta.y();

    emit notify( deflectEvent );
}

void PixelStreamInteractionDelegate::pinch( const QPointF position,
                                            const qreal pixelDelta )
{
    deflect::Event deflectEvent = _getNormEvent( position );
    deflectEvent.type = deflect::Event::EVT_WHEEL;
    deflectEvent.mouseLeft = false;
    deflectEvent.dy = pixelDelta;

    emit notify( deflectEvent );
}

deflect::Event swipeEvent( deflect::Event::EventType type )
{
    deflect::Event event;
    event.type = type;
    return event;
}

void PixelStreamInteractionDelegate::swipeLeft()
{
    emit notify( swipeEvent( deflect::Event::EVT_SWIPE_LEFT ));
}

void PixelStreamInteractionDelegate::swipeRight()
{
    emit notify( swipeEvent( deflect::Event::EVT_SWIPE_RIGHT ));
}

void PixelStreamInteractionDelegate::swipeUp()
{
    emit notify( swipeEvent( deflect::Event::EVT_SWIPE_UP ));
}

void PixelStreamInteractionDelegate::swipeDown()
{
    emit notify( swipeEvent( deflect::Event::EVT_SWIPE_DOWN ));
}

void PixelStreamInteractionDelegate::prevPage()
{
    swipeLeft();
}

void PixelStreamInteractionDelegate::nextPage()
{
    swipeRight();
}

void PixelStreamInteractionDelegate::keyPress( const int key,
                                               const int modifiers,
                                               const QString text )
{
    deflect::Event deflectEvent;
    deflectEvent.type = deflect::Event::EVT_KEY_PRESS;
    deflectEvent.key = key;
    deflectEvent.modifiers = modifiers;
    strncpy( deflectEvent.text, text.toStdString().c_str(),
             sizeof( deflectEvent.text ));

    emit notify( deflectEvent );
}

void PixelStreamInteractionDelegate::keyRelease( const int key,
                                                 const int modifiers,
                                                 const QString text )
{
    deflect::Event deflectEvent;
    deflectEvent.type = deflect::Event::EVT_KEY_RELEASE;
    deflectEvent.key = key;
    deflectEvent.modifiers = modifiers;
    strncpy( deflectEvent.text, text.toStdString().c_str(),
             sizeof( deflectEvent.text ));

    emit notify( deflectEvent );
}

void PixelStreamInteractionDelegate::_sendSizeChangedEvent()
{
    const QRectF& win = _contentWindow.getDisplayCoordinates();

    deflect::Event deflectEvent;
    deflectEvent.type = deflect::Event::EVT_VIEW_SIZE_CHANGED;
    deflectEvent.dx = win.width();
    deflectEvent.dy = win.height();

    emit notify( deflectEvent );
}

deflect::Event PixelStreamInteractionDelegate::_getNormEvent( const QPointF&
                                                              position ) const
{
    const QRectF& win = _contentWindow.getDisplayCoordinates();

    deflect::Event deflectEvent;
    deflectEvent.mouseLeft = true;
    deflectEvent.mouseX = ( position.x() - win.x( )) / win.width();
    deflectEvent.mouseY = ( position.y() - win.y( )) / win.height();
    return deflectEvent;
}

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

#ifndef PIXELSTREAMINTERACTIONDELEGATE_H
#define PIXELSTREAMINTERACTIONDELEGATE_H

#include "ContentInteractionDelegate.h"

#include <deflect/Event.h>

/**
 * Forward user actions to a deflect::Stream using Deflect events.
 */
class PixelStreamInteractionDelegate : public ContentInteractionDelegate
{
    Q_OBJECT

public:
    /** Constructor */
    PixelStreamInteractionDelegate( ContentWindow& contentWindow );

    /** @name Touch gesture handlers. */
    //@{
    void touchBegin( QPointF position ) override;
    void touchEnd( QPointF position ) override;
    void tap( QPointF position ) override;
    void doubleTap( QPointF position ) override;
    void tapAndHold( QPointF position ) override;
    void pan( QPointF position, QPointF delta ) override;
    void pinch( QPointF position, qreal pixelDelta ) override;
    void swipeLeft() override;
    void swipeRight() override;
    void swipeUp() override;
    void swipeDown() override;
    //@}

    /** @name Mouse gesture handlers. */
    //@{
    void keyPress( int key, int modifiers, QString text ) override;
    void keyRelease( int key, int modifiers, QString text ) override;
    //@}

    /** @name UI event handlers. */
    //@{
    void prevPage() override;
    void nextPage() override;
    //@}

    /** Register to receive events on this content. */
    bool registerEventReceiver( deflect::EventReceiver* receiver );

    /** Does this delegate already have registered EventReceiver(s) */
    bool hasEventReceivers() const;

signals:
    /** @internal Notify registered EventReceivers that an Event occured. */
    void notify( deflect::Event event );

private slots:
    void _sendSizeChangedEvent();

private:
    deflect::Event _getNormEvent( const QPointF& position ) const;

    unsigned int _eventReceiversCount;
};

#endif // PIXELSTREAMINTERACTIONDELEGATE_H

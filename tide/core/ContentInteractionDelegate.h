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

#ifndef CONTENTINTERACTIONDELEGATE_H
#define CONTENTINTERACTIONDELEGATE_H

#include "types.h"

#include <QObject>

/**
 * Handle user interaction with the Content of a ContentWindow.
 *
 * This class is abstract and should be reimplemented for the
 * different Content type.
 */
class ContentInteractionDelegate : public QObject
{
    Q_OBJECT

public:
    /** Construct a default interaction delegate that does nothing. */
    explicit ContentInteractionDelegate( ContentWindow& window );

    /** Virtual destructor. */
    virtual ~ContentInteractionDelegate();

    /** @name Touch gesture handlers. */
    //@{
    Q_INVOKABLE virtual void touchBegin( QPointF position )
    { Q_UNUSED( position ) }
    Q_INVOKABLE virtual void touchEnd( QPointF position )
    { Q_UNUSED( position ) }
    Q_INVOKABLE virtual void tap( QPointF position )
    { Q_UNUSED( position ) }
    Q_INVOKABLE virtual void doubleTap( QPointF position )
    { Q_UNUSED( position ) }
    Q_INVOKABLE virtual void tapAndHold( QPointF position, uint numPoints )
    { Q_UNUSED( position ) Q_UNUSED( numPoints ) }
    Q_INVOKABLE virtual void pan( QPointF position, QPointF delta,
                                  uint numPoints )
    { Q_UNUSED( position ) Q_UNUSED( delta ) Q_UNUSED( numPoints ) }
    Q_INVOKABLE virtual void pinch( QPointF position, QPointF pixelDelta )
    { Q_UNUSED( position ) Q_UNUSED( pixelDelta ) }
    Q_INVOKABLE virtual void swipeLeft() {}
    Q_INVOKABLE virtual void swipeRight() {}
    Q_INVOKABLE virtual void swipeUp() {}
    Q_INVOKABLE virtual void swipeDown() {}
    //@}

    /** @name Keyboard event handlers. */
    //@{
    Q_INVOKABLE virtual void keyPress( int key, int modifiers, QString text )
    { Q_UNUSED( key ) Q_UNUSED( modifiers ) Q_UNUSED( text ) }
    Q_INVOKABLE virtual void keyRelease( int key, int modifiers, QString text )
    { Q_UNUSED( key ) Q_UNUSED( modifiers ) Q_UNUSED( text ) }
    //@}

    /** @name UI event handlers. */
    //@{
    Q_INVOKABLE virtual void prevPage() {}
    Q_INVOKABLE virtual void nextPage() {}
    //@}

protected:
    ContentWindow& _contentWindow;

    QPointF getNormalizedPoint( const QPointF& point ) const;

private:
    Q_DISABLE_COPY( ContentInteractionDelegate )
};

#endif

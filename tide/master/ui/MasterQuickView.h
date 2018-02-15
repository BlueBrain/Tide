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

#ifndef MASTERQUICKVIEW_H
#define MASTERQUICKVIEW_H

#include "types.h"

class MasterDisplayGroupRenderer;

#include <QGesture>
#include <QGestureEvent>
#include <QQuickView>
#include <QUuid>

/**
 * A view of the display wall inside the master window.
 */
class MasterQuickView : public QQuickView
{
    Q_OBJECT

public:
    /** Constructor. */
    MasterQuickView(OptionsPtr options, ScreenLockPtr lock,
                    const Configuration& config);

    /** Destructor */
    ~MasterQuickView();

    /** Get the wall qml item. */
    QQuickItem* wallItem();

    /** Map a normalized touch event on the wall to this view's coordinates. */
    QPointF mapToWallPos(const QPointF& normalizedPos) const;

signals:
    /** @name Emitted when a user interactacts with the mouse. */
    //@{
    void mousePressed(QPointF pos);
    void mouseMoved(QPointF pos);
    void mouseReleased(QPointF pos);
    //@}

private:
    /** Re-implement QWindow event to capture tab key. */
    bool event(QEvent* event) final;
    void _mapTouchEvent(QTouchEvent* event);

    QQuickItem* _wallItem = nullptr;
};

#endif

/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#ifndef WINDOWTOUCHCONTROLLER_H
#define WINDOWTOUCHCONTROLLER_H

#include "types.h"

#include "control/WindowController.h"

#include <QObject>

/**
 * Controller for interacting with the window on a touch surface.
 */
class WindowTouchController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowTouchController)

public:
    /**
     * Create a touch controller for a window.
     * @param window the target window.
     * @param group the group to which the window belongs.
     */
    WindowTouchController(Window& window, DisplayGroup& group);

    Q_INVOKABLE void onTouchStarted();
    Q_INVOKABLE void onTap();
    Q_INVOKABLE void onTapAndHold();
    Q_INVOKABLE void onDoubleTap(uint numPoints);

    Q_INVOKABLE void onPanStarted();
    Q_INVOKABLE void onPan(const QPointF& pos, const QPointF& delta,
                           uint numPoints);
    Q_INVOKABLE void onPanEnded();

    Q_INVOKABLE void onPinchStarted();
    Q_INVOKABLE void onPinch(const QPointF& pos, const QPointF& delta);
    Q_INVOKABLE void onPinchEnded();

private:
    Window& _window;
    DisplayGroup& _group;
    WindowController _controller;

    void _toggleFocusMode();
    bool _isWindowActive() const;
};

#endif

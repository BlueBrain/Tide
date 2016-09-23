/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#ifndef QUICKRENDERER_H
#define QUICKRENDERER_H

#include "types.h"

#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class QOpenGLContext;
class QQuickRenderControl;
class QSurface;

/**
 * Renders the QML scene from the given window using QQuickRenderControl onto
 * the surface of the window and synchronizes the swapBuffers() with all other
 * wall processes. Note that this object needs to be moved to a seperate
 * (render)thread to function properly.
 *
 * Inspired by http://doc.qt.io/qt-5/qtquick-rendercontrol-window-multithreaded-cpp.html
 */
class QuickRenderer : public QObject
{
    Q_OBJECT

public:
    QuickRenderer( WallWindow& window );

    /**
     * To be called from GUI/main thread to trigger rendering and swapBuffers().
     * This call is blocking until sync() is done in render thread and must be
     * executed on all wall processes for non-deadlocking swap barrier.
     */
    void render();

signals:
    /** Emitted after swapBuffers(). Originates from render thread */
    void frameSwapped();

    /**
     * To be called from GUI/main thread to initialize this object on render
     * thread. Blocks until operation on render thread is done.
     */
    void init();

    /**
     * To be called from GUI/main thread to stop using this object on the render
     * thread. Blocks until operation on render thread is done.
     */
    void stop();

private:
    bool event( QEvent* qtEvent ) final;
    void _onInit();
    void _onRender();
    void _onStop();

    QOpenGLContext& _glContext;
    QQuickRenderControl& _renderControl;
    QSurface& _surface;

    WallToWallChannel& _wallChannel;

    bool _initialized;

    QMutex _mutex;
    QWaitCondition _cond;
};

#endif

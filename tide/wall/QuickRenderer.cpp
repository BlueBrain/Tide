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

#include "QuickRenderer.h"

#include "network/WallToWallChannel.h"
#include "WallWindow.h"

#include <QCoreApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickRenderControl>

QuickRenderer::QuickRenderer( WallWindow& window )
    : _glContext( window.getOpenGLContext( ))
    , _renderControl( window.getRenderControl( ))
    , _surface( window )
    , _wallChannel( window.getWallChannel( ))
    , _initialized( false )
{
    connect( this, &QuickRenderer::init, this,
             &QuickRenderer::_onInit, Qt::BlockingQueuedConnection );
    connect( this, &QuickRenderer::stop, this,
             &QuickRenderer::_onStop, Qt::BlockingQueuedConnection );
}

void QuickRenderer::render()
{
    QMutexLocker lock( &_mutex );
    QCoreApplication::postEvent( this, new QEvent( QEvent::User ));

    // the main thread has to be blocked for sync()
    _cond.wait( &_mutex );
}

bool QuickRenderer::event( QEvent* e )
{
    if( e->type() == QEvent::User )
    {
        _onRender();
        return true;
    }
    return QObject::event( e );
}

void QuickRenderer::_onInit()
{
    _glContext.makeCurrent( &_surface );
    _renderControl.initialize( &_glContext );
    _initialized = true;
}

void QuickRenderer::_onStop()
{
    _glContext.makeCurrent( &_surface );

    _renderControl.invalidate();

    _glContext.doneCurrent();
    _glContext.moveToThread( QCoreApplication::instance()->thread( ));
}

void QuickRenderer::_onRender()
{
    if( !_initialized )
        return;

    {
        QMutexLocker lock( &_mutex );

        _glContext.makeCurrent( &_surface );

        _renderControl.sync();

        // unblock main thread after sync in render thread is done
        _cond.wakeOne();
    }

    _renderControl.render();
    _glContext.functions()->glFinish();

    _wallChannel.globalBarrier();

    _glContext.swapBuffers( &_surface );
    _glContext.functions()->glFlush();

    emit frameSwapped();
}

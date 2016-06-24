/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#ifndef WALLWINDOW_H
#define WALLWINDOW_H

#include "types.h"
#include "WallToWallChannel.h"

#include <QQuickWindow>

class QuickRenderer;
class QQuickRenderControl;
class QQmlEngine;
class QQmlComponent;
class QQuickItem;

class WallWindow : public QQuickWindow
{
    Q_OBJECT

public:
    /**
     * Create a wall window.
     * @param config the wall configuration to setup this window wrt position,
     *               size, etc.
     * @param renderControl the Qt render control for QML scene rendering
     * @param wallChannel to synchronize clocks and swapBuffers()
     */
    WallWindow( const WallConfiguration& config,
                QQuickRenderControl* renderControl,
                WallToWallChannel& wallChannel );

    ~WallWindow();

    /** @return the display group renderer. */
    DisplayGroupRenderer& getDisplayGroupRenderer();

    /** Update and synchronize scene objects and trigger frame rendering. */
    bool syncAndRender();

    /** Set new render options. */
    void setRenderOptions( OptionsPtr options );

    /** Set new display group. */
    void setDisplayGroup( DisplayGroupPtr displayGroup );

    /** Set new touchpoint's markers. */
    void setMarkers( MarkersPtr markers );

    /** @return the data provider. */
    DataProvider& getDataProvider();

    /** @return the QML engine. */
    QQmlEngine* engine() const;

    /** @return the root object of the QML scene. */
    QQuickItem*	rootObject() const;

    /** @return the communication channel to synchronize with other windows. */
    WallToWallChannel& getWallChannel();

    /** @return the OpenGL context. */
    QOpenGLContext& getOpenGLContext();

    /** @return the Qt quick render control. */
    QQuickRenderControl& getRenderControl();

    /** @return the texture uploader. */
    TextureUploader& getUploader();

private:
    void exposeEvent( QExposeEvent* exposeEvent ) final;

    void _startQuick( const WallConfiguration& config );

    DisplayGroupRenderer* _displayGroupRenderer;
    TestPattern* _testPattern;
    WallToWallChannel& _wallChannel;

    QOpenGLContext* _glContext;
    QQuickRenderControl* _renderControl;
    QuickRenderer* _quickRenderer;
    QThread* _quickRendererThread;
    QQmlEngine* _qmlEngine;
    QQmlComponent* _qmlComponent;
    QQuickItem* _rootItem;
    bool _rendererInitialized;

    QThread* _uploadThread;
    TextureUploader* _uploader;
    DataProvider* _provider;
};

#endif

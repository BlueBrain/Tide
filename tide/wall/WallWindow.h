/*********************************************************************/
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project                  */
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

#ifndef WALLWINDOW_H
#define WALLWINDOW_H

#include "WallSynchronizer.h"
#include "types.h"

#include <QQuickWindow>

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
     * @param windowIndex the index of the window for this process
     * @param provider the provider of data for the windows
     * @param renderControl the Qt render control for QML scene rendering
     * @param synchronizer to synchronize swapBuffers()
     */
    WallWindow(const WallConfiguration& config, uint windowIndex,
               DataProvider& provider,
               std::unique_ptr<QQuickRenderControl> renderControl,
               WallSynchronizer& synchronizer);

    ~WallWindow();

    bool isInitialized() const;
    bool needRedraw() const;

    /**
     * Synchronize scene objects with render thread and trigger frame rendering.
     *
     * @param grab indicate that the frame should be grabbed after rendering.
     */
    void render(bool grab = false);

    /** Set new display group. */
    void setDisplayGroup(DisplayGroupPtr displayGroup);

    /** Set new touchpoint's markers. */
    void setMarkers(MarkersPtr markers);

    /** Set new render options. */
    void setRenderOptions(OptionsPtr options);

    /** @return the QML engine. */
    QQmlEngine* engine() const;

    /** @return the root object of the QML scene. */
    QQuickItem* rootObject() const;

    /** @return the texture uploader. */
    TextureUploader& getUploader();

signals:
    /** Emitted after syncAndRender() has been called with grab set to true. */
    void imageGrabbed(QImage image, QPoint index);

private:
    void exposeEvent(QExposeEvent* exposeEvent) final;

    void _startQuick(const WallConfiguration& config, const uint windowIndex);

    DataProvider& _provider;
    std::unique_ptr<QQuickRenderControl> _renderControl;
    WallSynchronizer& _synchronizer;
    bool _rendererInitialized = false;
    bool _grabImage = false;

    std::unique_ptr<deflect::qt::QuickRenderer> _quickRenderer;
    std::unique_ptr<QThread> _quickRendererThread;

    std::unique_ptr<TextureUploader> _uploader;
    std::unique_ptr<QThread> _uploadThread;

    std::unique_ptr<QQmlEngine> _qmlEngine;
    QQuickItem* _rootItem = nullptr; // child qobject of contentItem()
    std::unique_ptr<DisplayGroupRenderer> _displayGroupRenderer;
    std::unique_ptr<TestPattern> _testPattern;
};

#endif

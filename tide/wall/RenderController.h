/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
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

#ifndef RENDERCONTROLLER_H
#define RENDERCONTROLLER_H

#include "types.h"

#include "SwapSyncObject.h"

#include <QObject>

/**
 * Setup the scene and control the rendering options during runtime.
 */
class RenderController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RenderController)

public:
    RenderController(const WallConfiguration& config, DataProvider& provider,
                     WallToWallChannel& wallChannel, SwapSync type);
    ~RenderController();

public slots:
    void updateScene(ScenePtr scene);
    void updateMarkers(MarkersPtr markers);
    void updateOptions(OptionsPtr options);
    void updateLock(ScreenLockPtr lock);
    void updateCountdownStatus(CountdownStatusPtr status);
    void updateRequestScreenshot();
    void updateQuit();

signals:
    void screenshotRendered(QImage image, QPoint index);

private:
    std::vector<WallWindowPtr> _windows;
    DataProvider& _provider;
    WallToWallChannel& _wallChannel;
    std::unique_ptr<SwapSynchronizer> _swapSynchronizer;

    SwapSyncObject<ScenePtr> _syncScene;
    SwapSyncObject<MarkersPtr> _syncMarkers;
    SwapSyncObject<OptionsPtr> _syncOptions;
    SwapSyncObject<ScreenLockPtr> _syncLock;
    SwapSyncObject<CountdownStatusPtr> _syncCountdownStatus;
    SwapSyncObject<bool> _syncScreenshot{false};
    SwapSyncObject<bool> _syncQuit{false};

    int _renderTimer = 0;
    int _stopRenderingDelayTimer = 0;
    int _idleRedrawTimer = 0;
    bool _redrawNeeded = false;

    void timerEvent(QTimerEvent* qtEvent) final;

    /** Initialization. */
    void _connectSwapSyncObjects();
    void _connectRedrawSignal();
    void _connectScreenshotSignals();
    void _setupSwapSynchronization(SwapSync type);

    /** Synchronization and rendering. */
    void _requestRender();
    void _syncAndRender();
    void _renderAllWindows();
    void _scheduleRedraw();
    void _scheduleStopRendering();
    void _stopRendering();
    void _synchronizeSceneUpdates();
    void _synchronizeDataSourceUpdates();

    /** Shutdown. */
    void _terminateRendering();
};

#endif

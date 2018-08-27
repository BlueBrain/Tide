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

#include "RenderController.h"

#include "DataProvider.h"
#include "WallConfiguration.h"
#include "network/WallToWallChannel.h"
#include "qml/WallWindow.h"
#include "scene/CountdownStatus.h"
#include "scene/Options.h"
#include "scene/Scene.h"
#include "scene/ScreenLock.h"
#include "swapsync/SwapSynchronizer.h"

RenderController::RenderController(const WallConfiguration& config,
                                   DataProvider& provider,
                                   WallToWallChannel& wallChannel,
                                   NetworkBarrier& swapSyncBarrier,
                                   const SwapSync type)
    : _windows{WallWindow::createWindows(config, provider)}
    , _provider{provider}
    , _wallChannel{wallChannel}
{
    _connectSwapSyncObjects();
    _connectRedrawSignal();
    _connectScreenshotSignals();
    _setupSwapSynchronization(swapSyncBarrier, type);
    updateScene(Scene::create(config.surfaces));
}

RenderController::~RenderController() = default;

void RenderController::updateScene(ScenePtr scene)
{
    _syncScene.update(scene);
    _requestRender();
}

void RenderController::updateMarkers(MarkersPtr markers)
{
    _syncMarkers.update(markers);
    _requestRender();
}

void RenderController::updateOptions(OptionsPtr options)
{
    _syncOptions.update(options);
    _requestRender();
}

void RenderController::updateLock(ScreenLockPtr lock)
{
    _syncLock.update(lock);
    _requestRender();
}

void RenderController::updateCountdownStatus(CountdownStatusPtr status)
{
    _syncCountdownStatus.update(status);
    _requestRender();
}

void RenderController::updateRequestScreenshot()
{
    _syncScreenshot.update(true);
    _requestRender();
}

void RenderController::updateQuit()
{
    _syncQuit.update(true);
    _requestRender();
}

void RenderController::timerEvent(QTimerEvent* qtEvent)
{
    if (qtEvent->timerId() == _renderTimer)
        _syncAndRender();
    else if (qtEvent->timerId() == _idleRedrawTimer)
        _requestRender();
    else if (qtEvent->timerId() == _stopRenderingDelayTimer)
        _stopRendering();
}

void RenderController::_connectSwapSyncObjects()
{
    _syncScene.setCallback([this](ScenePtr scene) {
        _provider.updateDataSources(*scene);
        for (auto&& window : _windows)
            window->setSurface(scene->getSurfacePtr(window->getSurfaceIndex()));
    });
    _syncMarkers.setCallback([this](MarkersPtr markers) {
        for (auto&& window : _windows)
            window->setMarkers(markers);
    });
    _syncOptions.setCallback([this](OptionsPtr options) {
        for (auto&& window : _windows)
            window->setRenderOptions(options);
    });
    _syncLock.setCallback([this](ScreenLockPtr lock) {
        for (auto&& window : _windows)
            window->setScreenLock(lock);
    });
    _syncCountdownStatus.setCallback([this](CountdownStatusPtr status) {
        for (auto&& window : _windows)
            window->setCountdownStatus(status);
    });
}

void RenderController::_connectRedrawSignal()
{
    connect(&_provider, &DataProvider::imageLoaded, this,
            [this] { _redrawNeeded = true; }, Qt::QueuedConnection);
}

void RenderController::_connectScreenshotSignals()
{
    for (auto&& window : _windows)
    {
        connect(window.get(), &WallWindow::imageGrabbed, this,
                &RenderController::screenshotRendered);
    }
}

void RenderController::_setupSwapSynchronization(
    NetworkBarrier& swapSyncBarrier, const SwapSync type)
{
    if (type == SwapSync::hardware)
        print_log(LOG_INFO, LOG_GENERAL,
                  "Launching with hardware swap synchronization...");

    _swapSynchronizer =
        SwapSynchronizerFactory::get(type)->create(swapSyncBarrier,
                                                   _windows.size());
    for (auto&& window : _windows)
        window->setSwapSynchronizer(_swapSynchronizer.get());
}

void RenderController::_requestRender()
{
    killTimer(_stopRenderingDelayTimer);
    killTimer(_idleRedrawTimer);
    _stopRenderingDelayTimer = 0;
    _idleRedrawTimer = 0;

    if (_renderTimer == 0)
        _renderTimer = startTimer(5, Qt::PreciseTimer);
}

void RenderController::_syncAndRender()
{
    _synchronizeSceneUpdates();
    if (_syncQuit.get())
    {
        _terminateRendering();
        return;
    }

    _synchronizeDataSourceUpdates();
    _renderAllWindows();
    _scheduleRedraw();
}

void RenderController::_renderAllWindows()
{
    const auto grab = _syncScreenshot.get();
    if (grab)
        _syncScreenshot = SwapSyncObject<bool>{false};

    for (auto&& window : _windows)
    {
        if (!window->isInitialized())
            return;
        window->render(grab);
    }
}

void RenderController::_scheduleRedraw()
{
    for (const auto& window : _windows)
        _redrawNeeded = _redrawNeeded || window->needRedraw();

    if (_wallChannel.allReady(!_redrawNeeded))
        _scheduleStopRendering();
    else
        _requestRender();

    _redrawNeeded = false;
}

void RenderController::_scheduleStopRendering()
{
    if (_stopRenderingDelayTimer == 0)
        _stopRenderingDelayTimer = startTimer(5000 /*ms*/);
}

void RenderController::_stopRendering()
{
    killTimer(_renderTimer);
    killTimer(_stopRenderingDelayTimer);
    _renderTimer = 0;
    _stopRenderingDelayTimer = 0;

    // Redraw screen every minute so that the on-screen clock is up to date
    if (_idleRedrawTimer == 0)
        _idleRedrawTimer = startTimer(60000 /*ms*/);
}

void RenderController::_synchronizeSceneUpdates()
{
    auto versionCheckFunc = std::bind(&WallToWallChannel::checkVersion,
                                      &_wallChannel, std::placeholders::_1);

    _syncScene.sync(versionCheckFunc);
    _syncMarkers.sync(versionCheckFunc);
    _syncOptions.sync(versionCheckFunc);
    _syncLock.sync(versionCheckFunc);
    _syncCountdownStatus.sync(versionCheckFunc);
    _syncScreenshot.sync(versionCheckFunc);
    _syncQuit.sync(versionCheckFunc);
}

void RenderController::_synchronizeDataSourceUpdates()
{
    _wallChannel.synchronizeClock();
    _provider.synchronizeTilesSwap(_wallChannel);
    _provider.synchronizeTilesUpdate(_wallChannel);
}

void RenderController::_terminateRendering()
{
    killTimer(_renderTimer);
    killTimer(_stopRenderingDelayTimer);
    killTimer(_idleRedrawTimer);

    for (auto&& window : _windows)
        window.release()->deleteLater();
    _windows.clear();
}

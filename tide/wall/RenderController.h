/*********************************************************************/
/* Copyright (c) 2014-2017, EPFL/Blue Brain Project                  */
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
#include "SwapSynchronizer.h"

#include <QObject>

/**
 * Setup the scene and control the rendering options during runtime.
 */
class RenderController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RenderController)

public:
    /** Constructor */
    RenderController(std::vector<WallWindow*> windows, DataProvider& provider,
                     WallToWallChannel& wallChannel, SwapSync type);

public slots:
    void requestRender();

    void updateBackground(BackgroundPtr background);
    void updateCountdownStatus(CountdownStatusPtr status);
    void updateDisplayGroup(DisplayGroupPtr displayGroup);
    void updateLock(ScreenLockPtr lock);
    void updateMarkers(MarkersPtr markers);
    void updateOptions(OptionsPtr options);
    void updateRequestScreenshot();
    void updateQuit();

signals:
    void screenshotRendered(QImage image, QPoint index);

private:
    std::vector<WallWindow*> _windows; // deleteLater from syncQuit
    DataProvider& _provider;
    WallToWallChannel& _wallChannel;
    std::unique_ptr<SwapSynchronizer> _swapSynchronizer;

    SwapSyncObject<BackgroundPtr> _syncBackground;
    SwapSyncObject<CountdownStatusPtr> _syncCountdownStatus;
    SwapSyncObject<DisplayGroupPtr> _syncDisplayGroup;
    SwapSyncObject<ScreenLockPtr> _syncLock;
    SwapSyncObject<MarkersPtr> _syncMarkers;
    SwapSyncObject<OptionsPtr> _syncOptions;
    SwapSyncObject<bool> _syncScreenshot{false};
    SwapSyncObject<bool> _syncQuit{false};

    int _renderTimer = 0;
    int _stopRenderingDelayTimer = 0;
    int _idleRedrawTimer = 0;
    bool _needRedraw = false;

    void _setupSwapSynchronization(SwapSync type);

    void timerEvent(QTimerEvent* qtEvent) final;

    /** Update and synchronize scene objects before rendering a frame. */
    void _syncAndRender();
    bool _syncAndRenderWindows(bool grab);
    void _synchronizeObjects(const SyncFunction& versionCheckFunc);
};

#endif

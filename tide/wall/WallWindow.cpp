/*********************************************************************/
/* Copyright (c) 2015-2018, EPFL/Blue Brain Project                  */
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

#include "WallWindow.h"

#include "SwapSynchronizer.h"
#include "TestPattern.h"
#include "WallConfiguration.h"
#include "WallRenderContext.h"
#include "WallSceneRenderer.h"
#include "log.h"
#include "qmlUtils.h"
#include "scene/Background.h"
#include "scene/Options.h"
#include "screens.h"

#include <deflect/qt/QuickRenderer.h>

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQmlEngine>
#include <QQuickRenderControl>
#include <QThread>

WallWindow::WallWindow(const WallConfiguration& config, const uint windowIndex,
                       DataProvider& provider,
                       std::unique_ptr<QQuickRenderControl> renderControl)
    : QQuickWindow(renderControl.get())
    , _provider(provider)
    , _renderControl(std::move(renderControl))
    , _quickRenderer(new deflect::qt::QuickRenderer(*this, *_renderControl))
    , _quickRendererThread(new QThread)
    , _qmlEngine(new QQmlEngine)
{
    const auto windowNumber = QString::number(windowIndex);
    _quickRendererThread->setObjectName("Render #" + windowNumber);

    const auto& currentScreen = config.process.screens.at(windowIndex);

    if (auto qscreen = screens::find(currentScreen.display))
        setScreen(qscreen);
    else if (!currentScreen.display.isEmpty())
        print_log(LOG_FATAL, LOG_GENERAL, "Could not find display: '%s'",
                  currentScreen.display.toLocal8Bit().constData());

    setFlags(Qt::FramelessWindowHint);
    setPosition(currentScreen.position);
    const auto& surface = config.surfaces[currentScreen.surfaceIndex];
    resize(surface.getScreenRect(currentScreen.globalIndex).size());

    if (currentScreen.fullscreen)
    {
        setCursor(Qt::BlankCursor);
        showFullScreen();
    }
    else
        show();

    _startQuick(config, windowIndex);
}

WallWindow::~WallWindow()
{
    _quickRenderer->stop();
    _quickRendererThread->quit();
    _quickRendererThread->wait();
}

void WallWindow::setSwapSynchronizer(SwapSynchronizer* synchronizer)
{
    _synchronizer = synchronizer;
}

bool WallWindow::isInitialized() const
{
    return _rendererInitialized;
}

bool WallWindow::needRedraw() const
{
    return _sceneRenderer->needRedraw();
}

void WallWindow::exposeEvent(QExposeEvent*)
{
    if (!_rendererInitialized)
    {
// Initialize the renderer once the window is shown for correct GL
// context realisiation

#if QT_VERSION >= 0x050500
        // Call required to make QtGraphicalEffects work in the initial scene.
        _renderControl->prepareThread(_quickRendererThread.get());
#else
        print_log(LOG_DEBUG, LOG_GENERAL,
                  "missing QQuickRenderControl::prepareThread() on "
                  "Qt < 5.5. Expect some qWarnings and failing "
                  "QtGraphicalEffects.");
#endif

        _quickRenderer->moveToThread(_quickRendererThread.get());
        _quickRendererThread->start();
        _quickRenderer->init();

        _rendererInitialized = true;
    }
}

void WallWindow::_startQuick(const WallConfiguration& config,
                             const uint windowIndex)
{
    const auto& currentScreen = config.process.screens.at(windowIndex);
    const auto globalIndex = currentScreen.globalIndex;

    connect(_quickRenderer.get(), &deflect::qt::QuickRenderer::afterRender,
            [this, globalIndex] {
                if (_synchronizer)
                    _synchronizer->globalBarrier(*this);

                _quickRenderer->context()->swapBuffers(this);
                _quickRenderer->context()->functions()->glFlush();
                QMetaObject::invokeMethod(_sceneRenderer.get(),
                                          "updateRenderedFrames",
                                          Qt::QueuedConnection);
                if (_grabImage)
                {
                    emit imageGrabbed(_renderControl->grab(), globalIndex);
                    _grabImage = false;
                }
            });

    connect(_quickRenderer.get(), &deflect::qt::QuickRenderer::stopping,
            [this] {
                if (_synchronizer)
                    _synchronizer->exitBarrier(*this);
            });

    const auto& surface = config.surfaces[currentScreen.surfaceIndex];

    const auto screenRect = surface.getScreenRect(currentScreen.globalIndex);
    const auto wallSize = surface.getTotalSize();
    const auto view = currentScreen.stereoMode;

    WallRenderContext context{*_qmlEngine, _provider, wallSize, screenRect,
                              view};
    _sceneRenderer.reset(new WallSceneRenderer(context, *contentItem()));

    _testPattern.reset(
        new TestPattern(config, surface, currentScreen, *contentItem()));
    _testPattern->setPosition(-screenRect.topLeft());
}

void WallWindow::render(const bool grab)
{
    _grabImage = grab;

    _renderControl->polishItems();
    _quickRenderer->render();
}

void WallWindow::setBackground(BackgroundPtr background)
{
    setColor(background->getColor());
    _sceneRenderer->setBackground(background);
}

void WallWindow::setDisplayGroup(DisplayGroupPtr displayGroup)
{
    _sceneRenderer->setDisplayGroup(displayGroup);
}

void WallWindow::setScreenLock(ScreenLockPtr lock)
{
    _sceneRenderer->setScreenLock(lock);
}

void WallWindow::setCountdownStatus(CountdownStatusPtr status)
{
    _sceneRenderer->setCountdownStatus(status);
}

void WallWindow::setMarkers(MarkersPtr markers)
{
    _sceneRenderer->setMarkers(markers);
}

void WallWindow::setRenderOptions(OptionsPtr options)
{
    _testPattern->setVisible(options->getShowTestPattern());
    _sceneRenderer->setRenderingOptions(options);
}

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

#include "WallWindow.h"

#include "WallConfiguration.h"
#include "WallRenderContext.h"
#include "qml/TestPattern.h"
#include "qml/WallSurfaceRenderer.h"
#include "qml/qscreens.h"
#include "scene/Background.h"
#include "scene/Options.h"
#include "scene/Surface.h"
#include "swapsync/SwapSynchronizer.h"
#include "utils/log.h"
#include "utils/qml.h"

#include <deflect/qt/QuickRenderer.h>

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQmlEngine>
#include <QQuickRenderControl>
#include <QThread>

WallWindowPtr WallWindow::create(const WallConfiguration& config,
                                 const uint windowIndex, DataProvider& provider)
{
    return WallWindowPtr{
        new WallWindow{config, windowIndex, provider,
                       std::make_unique<QQuickRenderControl>()}};
}

std::vector<WallWindowPtr> WallWindow::createWindows(
    const WallConfiguration& config, DataProvider& provider)
{
    std::vector<WallWindowPtr> windows;
    for (auto screen = 0u; screen < config.screens.size(); ++screen)
        windows.emplace_back(WallWindow::create(config, screen, provider));
    return windows;
}

WallWindow::WallWindow(const WallConfiguration& config, const uint windowIndex,
                       DataProvider& provider,
                       std::unique_ptr<QQuickRenderControl> renderControl)
    : QQuickWindow(renderControl.get())
    , _provider(provider)
    , _renderControl(std::move(renderControl))
    , _quickRendererThread(new QThread)
    , _qmlEngine(new QQmlEngine)
{
    _quickRendererThread->setObjectName(QString("Render #%1").arg(windowIndex));

    const auto& screenConfig = config.screens.at(windowIndex);
    _surfaceIndex = screenConfig.surfaceIndex;
    _globalIndex = screenConfig.globalIndex;

    if (auto qscreen = qscreens::find(screenConfig.display))
        setScreen(qscreen);
    else if (!screenConfig.display.isEmpty())
        print_log(LOG_FATAL, LOG_GENERAL, "Could not find display: '%s'",
                  screenConfig.display.toLocal8Bit().constData());

    setFlags(Qt::FramelessWindowHint);
    setPosition(screenConfig.position);

    const auto& surfaceConfig = config.surfaces[screenConfig.surfaceIndex];
    resize(surfaceConfig.getScreenRect(screenConfig.globalIndex).size());

    if (screenConfig.fullscreen)
    {
        setCursor(Qt::BlankCursor);
        showFullScreen();
    }
    else
        show();

    _setupScene(config, windowIndex);
}

WallWindow::~WallWindow()
{
    _quickRenderer->stop();
    _quickRendererThread->quit();
    _quickRendererThread->wait();
}

size_t WallWindow::getSurfaceIndex() const
{
    return _surfaceIndex;
}

void WallWindow::setSwapSynchronizer(SwapSynchronizer* synchronizer)
{
    _synchronizer = synchronizer;
}

bool WallWindow::isInitialized() const
{
    return !!_quickRenderer;
}

bool WallWindow::needRedraw() const
{
    return _surfaceRenderer->needRedraw();
}

void WallWindow::render(const bool grab)
{
    _grabImage = grab;

    _renderControl->polishItems();
    _quickRenderer->render();
}

void WallWindow::setSurface(SurfacePtr surface)
{
    setColor(surface->getBackground().getColor());
    _surfaceRenderer->setSurface(surface);
}

void WallWindow::setScreenLock(ScreenLockPtr lock)
{
    _surfaceRenderer->setScreenLock(lock);
}

void WallWindow::setCountdownStatus(CountdownStatusPtr status)
{
    _surfaceRenderer->setCountdownStatus(status);
}

void WallWindow::setMarkers(MarkersPtr markers)
{
    _surfaceRenderer->setMarkers(markers);
}

void WallWindow::setRenderOptions(OptionsPtr options)
{
    _testPattern->setVisible(options->getShowTestPattern());
    _surfaceRenderer->setRenderingOptions(options);
}

void WallWindow::exposeEvent(QExposeEvent*)
{
    // Initialize the renderer once the window is shown for correct GL
    // context realisiation
    if (!_quickRenderer)
        _startQuickRenderer();
}

void WallWindow::_startQuickRenderer()
{
#if QT_VERSION >= 0x050500
    // Call required to make QtGraphicalEffects work in the initial scene.
    _renderControl->prepareThread(_quickRendererThread.get());
#else
    print_log(LOG_DEBUG, LOG_GENERAL,
              "missing QQuickRenderControl::prepareThread() on "
              "Qt < 5.5. Expect some qWarnings and failing "
              "QtGraphicalEffects.");
#endif

    _quickRenderer =
        std::make_unique<deflect::qt::QuickRenderer>(*this, *_renderControl);
    _quickRenderer->moveToThread(_quickRendererThread.get());
    _quickRendererThread->start();
    _quickRenderer->init();

    connect(_quickRenderer.get(), &deflect::qt::QuickRenderer::afterRender,
            [this] {
                if (_synchronizer)
                    _synchronizer->globalBarrier(*this);

                _quickRenderer->context()->swapBuffers(this);
                _quickRenderer->context()->functions()->glFlush();
                QMetaObject::invokeMethod(_surfaceRenderer.get(),
                                          "updateRenderedFrames",
                                          Qt::QueuedConnection);
                if (_grabImage)
                {
                    emit imageGrabbed(_renderControl->grab(), _globalIndex);
                    _grabImage = false;
                }
            });

    connect(_quickRenderer.get(), &deflect::qt::QuickRenderer::stopping,
            [this] {
                if (_synchronizer)
                    _synchronizer->exitBarrier(*this);
            });
}

void WallWindow::_setupScene(const WallConfiguration& config,
                             const uint windowIndex)
{
    const auto& screenConfig = config.screens.at(windowIndex);
    const auto& surface = config.surfaces[screenConfig.surfaceIndex];

    const auto screenRect = surface.getScreenRect(screenConfig.globalIndex);
    const auto wallSize = surface.getTotalSize();
    const auto stereoView = screenConfig.stereoMode;
    const auto surfaceIndex = screenConfig.surfaceIndex;

    WallRenderContext context{*_qmlEngine, _provider,  wallSize,
                              screenRect,  stereoView, surfaceIndex};
    _surfaceRenderer.reset(new WallSurfaceRenderer(context, *contentItem()));

    _testPattern.reset(
        new TestPattern(config, surface, screenConfig, *contentItem()));
    _testPattern->setPosition(-screenRect.topLeft());
}

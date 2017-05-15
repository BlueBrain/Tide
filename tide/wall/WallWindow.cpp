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

#include "WallWindow.h"

#include "DataProvider.h"
#include "DisplayGroupRenderer.h"
#include "TestPattern.h"
#include "TextureUploader.h"
#include "WallConfiguration.h"
#include "log.h"
#include "qmlUtils.h"
#include "scene/Options.h"
#include "screens.h"

#include <deflect/qt/QuickRenderer.h>

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQmlEngine>
#include <QQuickRenderControl>
#include <QThread>

namespace
{
const QUrl QML_ROOT_COMPONENT("qrc:/qml/wall/Background.qml");
}

WallWindow::WallWindow(const WallConfiguration& config, const uint windowIndex,
                       DataProvider& provider,
                       std::unique_ptr<QQuickRenderControl> renderControl,
                       WallSynchronizer& synchronizer)
    : QQuickWindow(renderControl.get())
    , _provider(provider)
    , _renderControl(std::move(renderControl))
    , _synchronizer(synchronizer)
    , _quickRenderer(new deflect::qt::QuickRenderer(*this, *_renderControl))
    , _quickRendererThread(new QThread)
    , _uploader(new TextureUploader)
    , _uploadThread(new QThread)
    , _qmlEngine(new QQmlEngine)
{
    const auto windowNumber = QString::number(windowIndex);
    _quickRendererThread->setObjectName("Render #" + windowNumber);
    _uploadThread->setObjectName("Upload #" + windowNumber);

    const auto& screen = config.getScreens().at(windowIndex);
    const auto screenSize = config.getScreenRect(screen.globalIndex).size();

    if (auto qscreen = screens::find(screen.display))
        setScreen(qscreen);
    else if (!screen.display.isEmpty())
        put_flog(LOG_FATAL, "Could not find display: '%s'",
                 screen.display.toLocal8Bit().constData());

    setFlags(Qt::FramelessWindowHint);
    setPosition(screen.position);
    resize(screenSize);

    if (config.getFullscreen())
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
    _uploader->stop();
    _uploadThread->quit();
    _uploadThread->wait();

    _quickRenderer->stop();
    _quickRendererThread->quit();
    _quickRendererThread->wait();
}

bool WallWindow::isInitialized() const
{
    return _rendererInitialized;
}

bool WallWindow::needRedraw() const
{
    return _displayGroupRenderer->needRedraw();
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
        put_flog(LOG_DEBUG,
                 "missing QQuickRenderControl::prepareThread() on "
                 "Qt < 5.5. Expect some qWarnings and failing "
                 "QtGraphicalEffects.");
#endif

        _quickRenderer->moveToThread(_quickRendererThread.get());
        _quickRendererThread->start();
        _quickRenderer->init();

        _uploader->moveToThread(_uploadThread.get());
        _uploadThread->start();
        _uploader->init(_quickRenderer->context(), screen());

        _rendererInitialized = true;
    }
}

void WallWindow::_startQuick(const WallConfiguration& config,
                             const uint windowIndex)
{
    _rootItem = qml::makeItem(*_qmlEngine, QML_ROOT_COMPONENT);
    _rootItem->setParentItem(contentItem());

    // behave like SizeRootObjectToView
    _rootItem->setWidth(width());
    _rootItem->setHeight(height());

    const auto& screen = config.getScreens().at(windowIndex);
    const auto screenRect = config.getScreenRect(screen.globalIndex);
    const auto view = config.getScreens().at(windowIndex).stereoMode;

    // DisplayGroupRenderer needs _engine and _rootItem from *this*
    _displayGroupRenderer.reset(
        new DisplayGroupRenderer(*this, _provider, screenRect, view));

    connect(_displayGroupRenderer.get(), &DisplayGroupRenderer::imageLoaded,
            _uploader.get(), &TextureUploader::uploadTexture);

    const auto globalIndex = screen.globalIndex;
    connect(_quickRenderer.get(), &deflect::qt::QuickRenderer::afterRender,
            [this, globalIndex] {
                _synchronizer.globalBarrier();
                _quickRenderer->context()->swapBuffers(this);
                _quickRenderer->context()->functions()->glFlush();
                QMetaObject::invokeMethod(_displayGroupRenderer.get(),
                                          "updateRenderedFrames",
                                          Qt::QueuedConnection);
                if (_grabImage)
                {
                    emit imageGrabbed(_renderControl->grab(), globalIndex);
                    _grabImage = false;
                }
            });

    _testPattern.reset(new TestPattern(config, _rootItem));
    _testPattern->setPosition(-screenRect.topLeft());
}

void WallWindow::render(const bool grab)
{
    _grabImage = grab;

    _renderControl->polishItems();
    _quickRenderer->render();
}

void WallWindow::setDisplayGroup(DisplayGroupPtr displayGroup)
{
    _displayGroupRenderer->setDisplayGroup(displayGroup);
}

void WallWindow::setMarkers(MarkersPtr markers)
{
    _displayGroupRenderer->setMarkers(markers);
}

void WallWindow::setRenderOptions(OptionsPtr options)
{
    setColor(options->getShowTestPattern() ? Qt::black
                                           : options->getBackgroundColor());
    _testPattern->setVisible(options->getShowTestPattern());

    _displayGroupRenderer->setRenderingOptions(options);
}

QQmlEngine* WallWindow::engine() const
{
    return _qmlEngine.get();
}

QQuickItem* WallWindow::rootObject() const
{
    return _rootItem;
}

TextureUploader& WallWindow::getUploader()
{
    return *_uploader;
}

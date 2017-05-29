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

#include "WallWindow.h"

#include "DataProvider.h"
#include "DisplayGroupRenderer.h"
#include "PowerTimer.h"
#include "TestPattern.h"
#include "TextureUploader.h"
#include "WallConfiguration.h"
#include "log.h"
#include "qmlUtils.h"
#include "scene/Options.h"

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

WallWindow::WallWindow(const WallConfiguration& config,
                       QQuickRenderControl* renderControl,
                       WallToWallChannel& wallChannel)
    : QQuickWindow(renderControl)
    , _displayGroupRenderer(nullptr)
    , _testPattern(nullptr)
    , _wallChannel(wallChannel)
    , _renderControl(renderControl)
    , _quickRenderer(new deflect::qt::QuickRenderer(*this, *_renderControl))
    , _quickRendererThread(new QThread)
    , _rendererInitialized(false)
    , _qmlEngine(new QQmlEngine)
    , _qmlComponent(nullptr)
    , _rootItem(nullptr)
    , _uploadThread(new QThread)
    , _uploader(new TextureUploader)
    , _provider(new DataProvider{config.getStereoMode()})
{
    connect(_provider, &DataProvider::imageLoaded, _uploader,
            &TextureUploader::uploadTexture);

    const QPoint& screenIndex = config.getGlobalScreenIndex();
    const QRect& screenRect = config.getScreenRect(screenIndex);
    const QPoint& windowPos = config.getWindowPos();

    setFlags(Qt::FramelessWindowHint);
    setPosition(windowPos);
    resize(screenRect.size());

    if (config.getFullscreen())
    {
        setCursor(Qt::BlankCursor);
        showFullScreen();
    }
    else
        show();

    _startQuick(config);
}

WallWindow::~WallWindow()
{
    delete _provider;

    _uploader->stop();
    _uploadThread->quit();
    _uploadThread->wait();
    delete _uploadThread;

    _quickRenderer->stop();
    _quickRendererThread->quit();
    _quickRendererThread->wait();
    delete _quickRendererThread;

    delete _displayGroupRenderer;
    delete _qmlComponent;
    delete _qmlEngine;
    delete _quickRenderer;
    delete _uploader;
}

void WallWindow::exposeEvent(QExposeEvent*)
{
    if (!_rendererInitialized)
    {
// Initialize the renderer once the window is shown for correct GL
// context realisiation

#if QT_VERSION >= 0x050500
        // Call required to make QtGraphicalEffects work in the initial scene.
        _renderControl->prepareThread(_quickRendererThread);
#else
        put_flog(LOG_DEBUG,
                 "missing QQuickRenderControl::prepareThread() on "
                 "Qt < 5.5. Expect some qWarnings and failing "
                 "QtGraphicalEffects.");
#endif

        _quickRenderer->moveToThread(_quickRendererThread);

        _quickRendererThread->setObjectName("Render");
        _quickRendererThread->start();

        _quickRenderer->init();

        _uploader->moveToThread(_uploadThread);
        _uploadThread->setObjectName("Upload");
        _uploadThread->start();

        _uploader->init(_quickRenderer->context());

        _wallChannel.globalBarrier();
        _rendererInitialized = true;
    }
}

void WallWindow::_startQuick(const WallConfiguration& config)
{
    _qmlComponent = new QQmlComponent(_qmlEngine, QML_ROOT_COMPONENT);
    qmlCheckOrThrow(*_qmlComponent);
    QObject* rootObject_ = _qmlComponent->create();
    _rootItem = qobject_cast<QQuickItem*>(rootObject_);
    _rootItem->setParentItem(contentItem());

    // behave like SizeRootObjectToView
    _rootItem->setWidth(width());
    _rootItem->setHeight(height());

    const QPoint& screenIndex = config.getGlobalScreenIndex();
    const QRect& screenRect = config.getScreenRect(screenIndex);
    _displayGroupRenderer =
        new DisplayGroupRenderer(*this, *_provider, screenRect);
    connect(_quickRenderer, &deflect::qt::QuickRenderer::afterRender, [&] {
        _wallChannel.globalBarrier();
        _quickRenderer->context()->swapBuffers(this);
        _quickRenderer->context()->functions()->glFlush();
        QMetaObject::invokeMethod(_displayGroupRenderer, "updateRenderedFrames",
                                  Qt::QueuedConnection);
        if (_grabImage)
        {
            emit imageGrabbed(_renderControl->grab());
            _grabImage = false;
        }
    });

    _testPattern = new TestPattern(config, _rootItem);
    _testPattern->setPosition(-screenRect.topLeft());
}

bool WallWindow::syncAndRender(const bool grab)
{
    if (!_rendererInitialized)
        return true;

    _wallChannel.synchronizeClock();
    _displayGroupRenderer->synchronize(_wallChannel);

    _grabImage = grab;

    _renderControl->polishItems();
    _quickRenderer->render();

    const bool needRedraw = _displayGroupRenderer->needRedraw();
    return !_wallChannel.allReady(!needRedraw);
}

void WallWindow::setRenderOptions(OptionsPtr options)
{
    setColor(options->getShowTestPattern() ? Qt::black
                                           : options->getBackgroundColor());
    _testPattern->setVisible(options->getShowTestPattern());

    _displayGroupRenderer->setRenderingOptions(options);
}

void WallWindow::setDisplayGroup(DisplayGroupPtr displayGroup)
{
    _displayGroupRenderer->setDisplayGroup(displayGroup);
}

void WallWindow::setMarkers(MarkersPtr markers)
{
    _displayGroupRenderer->setMarkers(markers);
}

void WallWindow::setTimer(PowerTimerPtr timer)
{
    _displayGroupRenderer->setTimer(timer);
}

DataProvider& WallWindow::getDataProvider()
{
    return *_provider;
}

QQmlEngine* WallWindow::engine() const
{
    return _qmlEngine;
}

QQuickItem* WallWindow::rootObject() const
{
    return _rootItem;
}

WallToWallChannel& WallWindow::getWallChannel()
{
    return _wallChannel;
}

TextureUploader& WallWindow::getUploader()
{
    return *_uploader;
}

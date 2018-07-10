/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#include "MasterApplication.h"

#include "InactivityTimer.h"
#include "MasterSurfaceRenderer.h"
#include "PixelStreamWindowManager.h"
#include "QmlTypeRegistration.h"
#include "ScreenshotAssembler.h"
#include "configuration/Configuration.h"
#include "configuration/SurfaceConfigValidator.h"
#include "control/SceneController.h"
#include "localstreamer/PixelStreamerLauncher.h"
#include "log.h"
#include "network/MasterFromWallChannel.h"
#include "network/MasterToForkerChannel.h"
#include "network/MasterToWallChannel.h"
#include "scene/Background.h"
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
#include "scene/Markers.h"
#include "scene/Options.h"
#include "scene/Scene.h"
#include "scene/ScreenLock.h"
#include "scene/VectorialContent.h"
#include "ui/MasterQuickView.h"
#include "ui/MasterWindow.h"

#if TIDE_ENABLE_REST_INTERFACE
#include "LoggingUtility.h"
#include "rest/RestInterface.h"
#endif

#if TIDE_ENABLE_PLANAR_CONTROLLER
#include "ScreenControllerFactory.h"
#endif

#include <deflect/qt/QuickRenderer.h>
#include <deflect/server/EventReceiver.h>
#include <deflect/server/Server.h>

#include <QQuickRenderControl>
#include <stdexcept>

namespace
{
const int MOUSE_MARKER_ID = INT_MAX; // TUIO touch point IDs start at 0
const QUrl QML_OFFSCREEN_ROOT_COMPONENT("qrc:/qml/master/OffscreenRoot.qml");
}

MasterApplication::MasterApplication(int& argc_, char** argv_,
                                     const QString& config,
                                     MPIChannelPtr worldChannel,
                                     MPIChannelPtr forkChannel)
    : QApplication(argc_, argv_)
    , _config(new Configuration(config))
    , _masterToForkerChannel(new MasterToForkerChannel(forkChannel))
    , _masterToWallChannel(new MasterToWallChannel(worldChannel))
    , _masterFromWallChannel(new MasterFromWallChannel(worldChannel))
    , _lock(ScreenLock::create())
    , _markers(Markers::create(0))
    , _options(Options::create())
{
    master::registerQmlTypes();
    Content::setMaxScale(_config->settings.contentMaxScale);
    VectorialContent::setMaxScale(_config->settings.contentMaxScaleVectorial);

    // don't create touch points for mouse events and vice versa
    setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
    setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);

    _validateConfig();
    _init();

    _masterToWallChannel->send(*_config);
}

MasterApplication::~MasterApplication()
{
    _deflectServer.reset();
    _pixelStreamerLauncher.reset();

    // Make sure the send quit happens after any pending send operation;
    // If a send operation is not matched by a receive, the MPI connection
    // will block indefintely when trying to disconnect.
    QMetaObject::invokeMethod(_masterToForkerChannel.get(), "sendQuit",
                              Qt::BlockingQueuedConnection);
    QMetaObject::invokeMethod(_masterToWallChannel.get(), "sendQuit",
                              Qt::BlockingQueuedConnection);
    _mpiSendThread.quit();
    _mpiSendThread.wait();

    // This will wait for the quit reply from the first wall process
    _mpiReceiveThread.quit();
    _mpiReceiveThread.wait();
}

void MasterApplication::load(const QString& sessionFile)
{
    _sceneController->load(sessionFile, BoolCallback());
}

void MasterApplication::_validateConfig()
{
    for (const auto& surface : _config->surfaces)
    {
        try
        {
            SurfaceConfigValidator{surface}.validateDimensions();
        }
        catch (const dimensions_mismatch& e)
        {
            put_log(LOG_WARN, LOG_GENERAL, "Check your configuration: %s",
                    e.what());
        }
    }
}

void MasterApplication::_init()
{
    _scene = Scene::create(_config->surfaces);
    _sceneController =
        std::make_unique<SceneController>(*_scene, _config->folders);

    connect(_sceneController.get(), &SceneController::startWebbrowser,
            [this](const auto& webbrowser) {
                _pixelStreamerLauncher->launch(webbrowser);
            });

    _pixelStreamWindowManager.reset(new PixelStreamWindowManager(*_scene));
    _pixelStreamWindowManager->setAutoFocusNewWindows(
        _options->getAutoFocusPixelStreams());
    connect(_options.get(), &Options::autoFocusPixelStreamsChanged,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::setAutoFocusNewWindows);

    _pixelStreamerLauncher.reset(
        new PixelStreamerLauncher(*_pixelStreamWindowManager, *_config));

    _initView();

#if TIDE_ENABLE_REST_INTERFACE
    _initRestInterface();
#endif
#if TIDE_ENABLE_PLANAR_CONTROLLER
    if (!_config->master.planarSerialPort.isEmpty())
        _initScreenController();
#endif
    _startDeflectServer();
    _setupMPIConnections();
}

void MasterApplication::_initView()
{
    if (_config->master.headless)
        _initOffscreenView();
    else
        _initMasterWindow();
}

void MasterApplication::_initMasterWindow()
{
    _masterWindow.reset(new MasterWindow(_scene, _options, *_config));

    connect(_masterWindow.get(), &MasterWindow::openWebBrowser,
            _pixelStreamerLauncher.get(),
            &PixelStreamerLauncher::openWebbrowser);

    connect(_masterWindow.get(), &MasterWindow::openWhiteboard,
            _pixelStreamerLauncher.get(),
            &PixelStreamerLauncher::openWhiteboard);

    connect(_masterWindow.get(), &MasterWindow::sessionLoaded,
            _sceneController.get(), &SceneController::apply);

    auto view = _masterWindow->getQuickView();
    _createSurfaceRenderer(*view->engine(), view->getSurfaceItem());

    connect(view, &MasterQuickView::mousePressed, [this](const QPointF pos) {
        _markers->addMarker(MOUSE_MARKER_ID, pos);
    });
    connect(view, &MasterQuickView::mouseMoved, [this](const QPointF pos) {
        _markers->updateMarker(MOUSE_MARKER_ID, pos);
    });
    connect(view, &MasterQuickView::mouseReleased,
            [this](const QPointF) { _markers->removeMarker(MOUSE_MARKER_ID); });
}

void MasterApplication::_initOffscreenView()
{
    _offscreenQuickView.reset(new deflect::qt::OffscreenQuickView{
        std::make_unique<QQuickRenderControl>(),
        deflect::qt::RenderMode::DISABLED});
    _offscreenQuickView->load(QML_OFFSCREEN_ROOT_COMPONENT).wait();
    _offscreenQuickView->resize(_config->surfaces[0].getTotalSize());

    auto engine = _offscreenQuickView->getEngine();
    auto item = _offscreenQuickView->getRootItem();

    _createSurfaceRenderer(*engine, *item);
}

void MasterApplication::_createSurfaceRenderer(QQmlEngine& engine,
                                               QQuickItem& parentItem)
{
    _setContextProperties(*engine.rootContext());

    _surfaceRenderer.reset(
        new MasterSurfaceRenderer{_scene->getSurface(0), engine, parentItem});

    connect(_surfaceRenderer.get(), &MasterSurfaceRenderer::openLauncher,
            _pixelStreamerLauncher.get(), &PixelStreamerLauncher::openLauncher);
    connect(_surfaceRenderer.get(), &MasterSurfaceRenderer::open,
            _sceneController.get(), &SceneController::openAll);
}

void MasterApplication::_setContextProperties(QQmlContext& context)
{
    context.setContextProperty("options", _options.get());
    context.setContextProperty("lock", _lock.get());
}

void MasterApplication::_startDeflectServer()
{
    if (_deflectServer)
        return;
    try
    {
        _deflectServer.reset(new deflect::server::Server);
    }
    catch (const std::runtime_error& e)
    {
        print_log(LOG_FATAL, LOG_STREAM, "Could not start Deflect server: '%s'",
                  e.what());
        return;
    }

    connect(_deflectServer.get(), &deflect::server::Server::pixelStreamOpened,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::handleStreamStart);

    connect(_deflectServer.get(),
            &deflect::server::Server::pixelStreamException,
            [this](const QString uri, const QString what) {
                print_log(LOG_WARN, LOG_STREAM,
                          "Stream '%s' encountered an exception: '%s'",
                          uri.toLocal8Bit().constData(),
                          what.toLocal8Bit().constData());
            });

    connect(_deflectServer.get(), &deflect::server::Server::pixelStreamClosed,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::handleStreamEnd);

    connect(_pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::streamWindowClosed, _deflectServer.get(),
            &deflect::server::Server::closePixelStream);

    connect(_deflectServer.get(), &deflect::server::Server::receivedFrame,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::updateStreamWindows);

    connect(_pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::requestFirstFrame, _deflectServer.get(),
            &deflect::server::Server::requestFrame);

    connect(_deflectServer.get(), &deflect::server::Server::registerToEvents,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::registerEventReceiver);

    connect(_deflectServer.get(), &deflect::server::Server::receivedSizeHints,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::updateSizeHints);

    connect(_deflectServer.get(), &deflect::server::Server::receivedData,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::sendDataToWindow);
}

void MasterApplication::_setupMPIConnections()
{
    _masterToForkerChannel->moveToThread(&_mpiSendThread);
    _masterToWallChannel->moveToThread(&_mpiSendThread);
    _masterFromWallChannel->moveToThread(&_mpiReceiveThread);

    connect(_pixelStreamerLauncher.get(), &PixelStreamerLauncher::start,
            _masterToForkerChannel.get(), &MasterToForkerChannel::sendStart);

    connect(_scene.get(), &Scene::modified, _masterToWallChannel.get(),
            [this](ScenePtr scene) { _masterToWallChannel->sendAsync(scene); },
            Qt::DirectConnection);

    connect(_options.get(), &Options::updated, _masterToWallChannel.get(),
            [this](OptionsPtr options) {
                _masterToWallChannel->sendAsync(options);
            },
            Qt::DirectConnection);

    if (_inactivityTimer)
    {
        connect(_inactivityTimer.get(), &InactivityTimer::countdownUpdated,
                _masterToWallChannel.get(),
                [this](CountdownStatusPtr status) {
                    _masterToWallChannel->sendAsync(status);
                },
                Qt::DirectConnection);
    }

    connect(_lock.get(), &ScreenLock::modified, [this](ScreenLockPtr lock) {
        _masterToWallChannel->sendAsync(lock);
    });

    connect(_pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::streamWindowClosed, _lock.get(),
            &ScreenLock::cancelStreamAcceptance);

    connect(_pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::externalStreamOpening, _lock.get(),
            &ScreenLock::requestStreamAcceptance);

    connect(_lock.get(), &ScreenLock::streamAccepted,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::showWindows);

    connect(_lock.get(), &ScreenLock::streamRejected,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::handleStreamEnd);

    connect(_markers.get(), &Markers::updated, _masterToWallChannel.get(),
            [this](MarkersPtr markers) {
                _masterToWallChannel->sendAsync(markers);
            },
            Qt::DirectConnection);

    if (_deflectServer)
    {
        connect(_masterFromWallChannel.get(),
                &MasterFromWallChannel::receivedRequestFrame,
                _deflectServer.get(), &deflect::server::Server::requestFrame);

        connect(_deflectServer.get(), &deflect::server::Server::receivedFrame,
                _masterToWallChannel.get(), &MasterToWallChannel::sendFrame);
    }

    connect(_masterFromWallChannel.get(),
            &MasterFromWallChannel::pixelStreamClose,
            _pixelStreamWindowManager.get(),
            &PixelStreamWindowManager::handleStreamEnd);

    connect(&_mpiReceiveThread, &QThread::started, _masterFromWallChannel.get(),
            &MasterFromWallChannel::processMessages);

    _mpiSendThread.start();
    _mpiReceiveThread.start();
}

#if TIDE_ENABLE_REST_INTERFACE
void MasterApplication::_initRestInterface()
{
    _logger = std::make_unique<LoggingUtility>();
    _logger->monitor(*_scene);

    _restInterface =
        std::make_unique<RestInterface>(_config->master.webservicePort,
                                        _options, *_scene, *_config);
    _restInterface->exposeStatistics(*_logger);

    connect(_lock.get(), &ScreenLock::lockChanged,
            [this](const bool locked) { _restInterface->lock(locked); });

    const auto& appController = _restInterface->getAppRemoteController();

    connect(&appController, &AppRemoteController::open, _sceneController.get(),
            &SceneController::open);

    connect(&appController, &AppRemoteController::load, _sceneController.get(),
            &SceneController::load);

    connect(&appController, &AppRemoteController::save, _sceneController.get(),
            &SceneController::save);

    connect(&appController, &AppRemoteController::browse,
            _pixelStreamerLauncher.get(),
            &PixelStreamerLauncher::openWebbrowser);

    connect(&appController, &AppRemoteController::openWhiteboard,
            _pixelStreamerLauncher.get(),
            &PixelStreamerLauncher::openWhiteboard);

    connect(&appController, &AppRemoteController::takeScreenshot, this,
            &MasterApplication::_takeScreenshot);

    connect(&appController, &AppRemoteController::powerOff, this,
            &MasterApplication::_suspend);

    connect(&appController, &AppRemoteController::exit, [this]() { exit(); });
}
#endif

#if TIDE_ENABLE_PLANAR_CONTROLLER
void MasterApplication::_initScreenController()
{
    _screenController =
        ScreenControllerFactory::create(_config->master.planarSerialPort);

    _inactivityTimer =
        std::make_unique<InactivityTimer>(_config->settings.inactivityTimeout);

    connect(_inactivityTimer.get(), &InactivityTimer::poweroff, [this]() {
        _screenController->powerOff();
        print_log(LOG_INFO, LOG_POWER,
                  "Powering off the screens on inactivity timeout");
    });

    connect(_screenController.get(), &ScreenController::powerStateChanged,
            [this](const ScreenState state) {
                if (state == ScreenState::ON)
                    _inactivityTimer->restart();
                else
                    _inactivityTimer->stop();
            });

    connect(_screenController.get(), &ScreenController::powerStateChanged,
            [this](const ScreenState state) {
                if (state == ScreenState::OFF)
                    _lock->unlock();
            });

#if TIDE_ENABLE_REST_INTERFACE
    connect(_screenController.get(), &ScreenController::powerStateChanged,
            _logger.get(), &LoggingUtility::logScreenStateChanged);
#endif
}
#endif

void MasterApplication::_suspend()
{
#if TIDE_ENABLE_PLANAR_CONTROLLER
    if (_screenController && _screenController->getState() == ScreenState::ON)
    {
        if (_screenController->powerOff())
            _sceneController->hideLauncher();
        else
            print_log(LOG_ERROR, LOG_POWER, "Could not power off the screens");
    }
#endif
}

void MasterApplication::_resume()
{
#if TIDE_ENABLE_PLANAR_CONTROLLER
    if (_screenController && _screenController->getState() == ScreenState::OFF)
    {
        if (_screenController->powerOn())
            print_log(LOG_INFO, LOG_POWER,
                      "Powered on the screens by touching the wall");
        else
            print_log(LOG_ERROR, LOG_POWER,
                      "Could not power on the screens by touching the wall");
    }
#endif
}

void MasterApplication::_takeScreenshot(const uint surfaceIndex,
                                        const QString filename)
{
    // Don't interrupt an ongoing screenshot operation
    if (_screenshotAssembler && !_screenshotAssembler->isComplete())
        return;

    if (surfaceIndex >= _config->surfaces.size())
        return;

    _screenshotAssembler.reset(
        new ScreenshotAssembler(_config->surfaces[surfaceIndex]));

    connect(_masterFromWallChannel.get(),
            &MasterFromWallChannel::receivedScreenshot,
            _screenshotAssembler.get(), &ScreenshotAssembler::addImage);

    connect(_screenshotAssembler.get(),
            &ScreenshotAssembler::screenshotComplete,
            [filename](const QImage screenshot) { screenshot.save(filename); });

    _masterToWallChannel->sendRequestScreenshot();
}

bool MasterApplication::notify(QObject* receiver, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
    {
        _handle(static_cast<const QTouchEvent*>(event));
    }
    default:
        break;
    }
    return QApplication::notify(receiver, event);
}

void MasterApplication::_handle(const QTouchEvent* event)
{
    if (_inactivityTimer)
        _inactivityTimer->restart();

    if ((uint)event->touchPoints().length() >=
        _config->settings.touchpointsToWakeup)
    {
        _resume();
    }

    const auto wallSize = _config->surfaces[0].getTotalSize();
    auto getWallPos = [wallSize](const QPointF& normalizedPos) {
        return QPointF{normalizedPos.x() * wallSize.width(),
                       normalizedPos.y() * wallSize.height()};
    };

    for (const auto& point : event->touchPoints())
    {
        switch (point.state())
        {
        case Qt::TouchPointPressed:
            _markers->addMarker(point.id(), getWallPos(point.normalizedPos()));
            break;
        case Qt::TouchPointMoved:
            _markers->updateMarker(point.id(),
                                   getWallPos(point.normalizedPos()));
            break;
        case Qt::TouchPointReleased:
            _markers->removeMarker(point.id());
            break;
        case Qt::TouchPointStationary:
            break;
        }
    }
}

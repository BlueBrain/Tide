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

#include "MasterApplication.h"

#include "QmlTypeRegistration.h"
#include "TouchFilter.h"
#include "configuration/Configuration.h"
#include "configuration/SurfaceConfigValidator.h"
#include "control/AppController.h"
#include "gui/MasterQuickView.h"
#include "gui/MasterWindow.h"
#include "network/MasterFromWallChannel.h"
#include "network/MasterToForkerChannel.h"
#include "network/MasterToWallChannel.h"
#include "qml/MasterSurfaceRenderer.h"
#include "scene/Background.h"
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
#include "scene/Markers.h"
#include "scene/Options.h"
#include "scene/Scene.h"
#include "scene/ScreenLock.h"
#include "scene/VectorialContent.h"
#include "tools/MarkersUpdater.h"
#include "tools/ScreenshotAssembler.h"
#include "utils/log.h"

#if TIDE_ENABLE_REST_INTERFACE
#include "rest/RestInterface.h"
#include "tools/ActivityLogger.h"
#endif

#include <deflect/qt/QuickRenderer.h>
#include <deflect/server/Server.h>

#include <QQuickRenderControl>
#include <stdexcept>

namespace
{
const QUrl QML_OFFSCREEN_ROOT_COMPONENT("qrc:/qml/master/OffscreenRoot.qml");

std::unique_ptr<deflect::server::Server> _createDeflectServer()
{
    try
    {
        return std::make_unique<deflect::server::Server>();
    }
    catch (const std::runtime_error& e)
    {
        throw std::runtime_error(std::string("Deflect server error: ") +
                                 e.what());
    }
}
} // namespace

MasterApplication::MasterApplication(int& argc_, char** argv_,
                                     const QString& config,
                                     MPICommunicator& wallSendComm,
                                     MPICommunicator& wallRecvComm,
                                     MPICommunicator& forkerSendComm)
    : QApplication{argc_, argv_}
    , _config{new Configuration{config}}
    , _masterToForkerChannel{new MasterToForkerChannel{forkerSendComm}}
    , _masterToWallChannel{new MasterToWallChannel{wallSendComm}}
    , _masterFromWallChannel{new MasterFromWallChannel{wallRecvComm}}
    , _scene{Scene::create(_config->surfaces)}
    , _session{_scene}
    , _lock{ScreenLock::create()}
    , _markers{Markers::create(0)}
    , _options{Options::create()} // clang-format off
    , _deflectServer{_createDeflectServer()}
#if TIDE_ENABLE_REST_INTERFACE
    , _restInterface{new RestInterface{_config->master.webservicePort, _options,
                                       _session, *_config}}
    , _logger{new ActivityLogger}
#endif
    , _appController{new AppController{_session, *_lock, *_deflectServer,
                                       *_options, *_config}}
    , _markersUpdater{new MarkersUpdater{*_markers,
                                         _config->surfaces[0].getTotalSize()}}
// clang-format on
{
    qml::registerTypes();
    Content::setMaxScale(_config->settings.contentMaxScale);
    VectorialContent::setMaxScale(_config->settings.contentMaxScaleVectorial);

    // don't create touch points for mouse events and vice versa
    setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
    setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);

    _validateConfig();
    _initView();
#if TIDE_ENABLE_REST_INTERFACE
    _connectRestInterface();
#endif
    _setupMPIConnections();
    _masterToWallChannel->send(*_config);
    QApplication::installEventFilter(new TouchFilter(
        QMargins(int(_config->settings.touchPixelMargin.left),
                 int(_config->settings.touchPixelMargin.top),
                 int(_config->settings.touchPixelMargin.right),
                 int(_config->settings.touchPixelMargin.bottom)),
        _config->surfaces[0].getTotalSize()));
}

MasterApplication::~MasterApplication()
{
    _deflectServer.reset();

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
    _appController->load(sessionFile, BoolCallback());
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

void MasterApplication::_initView()
{
    if (_config->master.headless)
        _initOffscreenView();
    else
        _initGUIWindow();
}

void MasterApplication::_initGUIWindow()
{
    _masterWindow.reset(new MasterWindow(_scene, _options, *_config));

    connect(_masterWindow.get(), &MasterWindow::open, _appController.get(),
            &AppController::open);

    connect(_masterWindow.get(), &MasterWindow::load, _appController.get(),
            &AppController::load);

    connect(_masterWindow.get(), &MasterWindow::save, _appController.get(),
            &AppController::save);

    connect(_masterWindow.get(), &MasterWindow::openWebBrowser,
            _appController.get(), &AppController::openWebbrowser);

    connect(_masterWindow.get(), &MasterWindow::openWhiteboard,
            _appController.get(), &AppController::openWhiteboard);

    connect(_masterWindow.get(), &MasterWindow::clear, _appController.get(),
            &AppController::clear);

    _createGUISurfaceRenderers();
}

void MasterApplication::_createGUISurfaceRenderers()
{
    for (auto view : _masterWindow->getQuickViews())
        _createNextSurfaceRenderer(*view->engine(), view->getSurfaceItem());

    _connectGUIMouseEventsToMarkers(*_masterWindow->getQuickViews().front());
}

void MasterApplication::_connectGUIMouseEventsToMarkers(MasterQuickView& view)
{
    connect(&view, &MasterQuickView::mousePressed,
            [this](const auto& pos) { _markersUpdater->mousePressed(pos); });
    connect(&view, &MasterQuickView::mouseMoved,
            [this](const auto& pos) { _markersUpdater->mouseMoved(pos); });
    connect(&view, &MasterQuickView::mouseReleased,
            [this](const auto& pos) { _markersUpdater->mouseReleased(pos); });
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

    _createNextSurfaceRenderer(*engine, *item);
}

void MasterApplication::_createNextSurfaceRenderer(QQmlEngine& engine,
                                                   QQuickItem& parentItem)
{
    _setContextProperties(*engine.rootContext());

    auto renderer = std::make_unique<MasterSurfaceRenderer>(
        _scene->getSurface(_surfaceRenderers.size()), engine, parentItem);

    connect(renderer.get(), &MasterSurfaceRenderer::openLauncher,
            _appController.get(), &AppController::openLauncher);
    connect(renderer.get(), &MasterSurfaceRenderer::open, _appController.get(),
            &AppController::openAll);

    _surfaceRenderers.emplace_back(std::move(renderer));
}

void MasterApplication::_setContextProperties(QQmlContext& context)
{
    context.setContextProperty("options", _options.get());
    context.setContextProperty("lock", _lock.get());
}

#if TIDE_ENABLE_REST_INTERFACE
void MasterApplication::_connectRestInterface()
{
    _logger->monitor(*_scene);
    _restInterface->exposeStatistics(*_logger);

    connect(_lock.get(), &ScreenLock::lockChanged,
            [this](const bool locked) { _restInterface->lock(locked); });

    connect(_appController.get(), &AppController::screenStateChanged,
            _logger.get(), &ActivityLogger::logScreenStateChanged);

    const auto& appRemoteController = _restInterface->getAppRemoteController();

    connect(&appRemoteController, &AppRemoteController::open,
            _appController.get(), &AppController::open);

    connect(&appRemoteController, &AppRemoteController::load,
            _appController.get(), &AppController::load);

    connect(&appRemoteController, &AppRemoteController::save,
            _appController.get(), &AppController::save);

    connect(&appRemoteController, &AppRemoteController::browse,
            _appController.get(), &AppController::openWebbrowser);

    connect(&appRemoteController, &AppRemoteController::openWhiteboard,
            _appController.get(), &AppController::openWhiteboard);

    connect(&appRemoteController, &AppRemoteController::takeScreenshot, this,
            &MasterApplication::_takeScreenshot);

    connect(&appRemoteController, &AppRemoteController::powerOff,
            _appController.get(), &AppController::suspend);

    connect(&appRemoteController, &AppRemoteController::powerOn,
            _appController.get(), &AppController::resume);

    connect(&appRemoteController, &AppRemoteController::exit, this,
            &QCoreApplication::quit);
}
#endif

void MasterApplication::_setupMPIConnections()
{
    _mpiReceiveThread.setObjectName("Recv");
    _mpiSendThread.setObjectName("Send");

    _masterToForkerChannel->moveToThread(&_mpiSendThread);
    _masterToWallChannel->moveToThread(&_mpiSendThread);
    _masterFromWallChannel->moveToThread(&_mpiReceiveThread);

    connect(_appController.get(), &AppController::start,
            _masterToForkerChannel.get(), &MasterToForkerChannel::sendStart);

    connect(_scene.get(), &Scene::modified, _masterToWallChannel.get(),
            [this](ScenePtr scene) {
                _masterToWallChannel->sendAsync(std::move(scene));
            },
            Qt::DirectConnection);

    connect(_options.get(), &Options::updated, _masterToWallChannel.get(),
            [this](OptionsPtr options) {
                _masterToWallChannel->sendAsync(std::move(options));
            },
            Qt::DirectConnection);

    connect(_appController.get(), &AppController::countdownUpdated,
            _masterToWallChannel.get(),
            [this](CountdownStatusPtr status) {
                _masterToWallChannel->sendAsync(std::move(status));
            },
            Qt::DirectConnection);

    connect(_lock.get(), &ScreenLock::modified, [this](ScreenLockPtr lock) {
        _masterToWallChannel->sendAsync(std::move(lock));
    });

    connect(_markers.get(), &Markers::updated, _masterToWallChannel.get(),
            [this](MarkersPtr markers) {
                _masterToWallChannel->sendAsync(std::move(markers));
            },
            Qt::DirectConnection);

    connect(_masterFromWallChannel.get(),
            &MasterFromWallChannel::receivedRequestFrame, _deflectServer.get(),
            &deflect::server::Server::requestFrame);

    connect(_deflectServer.get(), &deflect::server::Server::receivedFrame,
            _masterToWallChannel.get(), &MasterToWallChannel::sendFrame);

    connect(_masterFromWallChannel.get(),
            &MasterFromWallChannel::pixelStreamClose, _appController.get(),
            &AppController::terminateStream);

    connect(&_mpiReceiveThread, &QThread::started, _masterFromWallChannel.get(),
            &MasterFromWallChannel::processMessages);

    _mpiSendThread.start();
    _mpiReceiveThread.start();
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
        break;
    }
    default:
        break;
    }
    return QApplication::notify(receiver, event);
}

void MasterApplication::_handle(const QTouchEvent* event)
{
    _appController->handleTouchEvent(
        static_cast<uint>(event->touchPoints().length()));
    _markersUpdater->update(event);
}

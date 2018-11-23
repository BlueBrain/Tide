/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#include "AppController.h"

#include "PixelStreamWindowManager.h"
#include "configuration/Configuration.h"
#include "control/PixelStreamWindowManager.h"
#include "control/SceneController.h"
#include "control/SessionController.h"
#include "localstreamer/PixelStreamerLauncher.h"
#include "scene/Options.h"
#include "scene/ScreenLock.h"
#include "tools/InactivityTimer.h"

#if TIDE_ENABLE_PLANAR_CONTROLLER
#include "hardware/ScreenControllerFactory.h"
#endif

#include <deflect/server/EventReceiver.h>
#include <deflect/server/Server.h>

AppController::AppController(Session& session, ScreenLock& lock,
                             deflect::server::Server& deflectServer,
                             Options& options, const Configuration& config)
    : _sceneController{new SceneController{*session.getScene(), config.folders}}
    , _sessionController{new SessionController{session, config.folders}}
    , _pixelStreamWindowManager{new PixelStreamWindowManager{
          *session.getScene()}}
    , _pixelStreamerLauncher{new PixelStreamerLauncher{
          *_pixelStreamWindowManager, config}}
    , _lock{lock}
    , _config{config}
{
    if (!config.master.planarSerialPort.isEmpty())
        _initScreenController(config);

    connect(_sessionController.get(), &SessionController::startWebbrowser,
            [this](const auto& webbrowser) {
                _pixelStreamerLauncher->launch(webbrowser);
            });

    connect(_pixelStreamerLauncher.get(), &PixelStreamerLauncher::start, this,
            &AppController::start);

    _connect(lock, *_pixelStreamWindowManager);
    _connect(deflectServer, *_pixelStreamWindowManager);
    _connect(options, *_pixelStreamWindowManager);
}

AppController::~AppController() = default;

void AppController::open(uint surfaceIndex, QString uri, QPointF coords,
                         BoolMsgCallback callback)
{
    _sceneController->open(surfaceIndex, uri, coords, callback);
}

void AppController::openAll(const QStringList& uris)
{
    _sceneController->openAll(uris);
}

void AppController::clear(uint surfaceIndex)
{
    _sceneController->clear(surfaceIndex);
}

void AppController::load(QString uri, BoolCallback callback)
{
    _sessionController->load(uri, callback);
}

void AppController::save(QString uri, BoolCallback callback)
{
    _sessionController->save(uri, callback);
}

void AppController::openLauncher()
{
    _pixelStreamerLauncher->openLauncher();
}

void AppController::openWebbrowser(uint surfaceIndex, QString url, QSize size,
                                   QPointF pos, ushort debugPort)
{
    _pixelStreamerLauncher->openWebbrowser(surfaceIndex, url, size, pos,
                                           debugPort);
}

void AppController::openWhiteboard(uint surfaceIndex)
{
    _pixelStreamerLauncher->openWhiteboard(surfaceIndex);
}

void AppController::terminateStream(const QString& uri)
{
    _pixelStreamWindowManager->handleStreamEnd(uri);
}

void AppController::suspend(BoolCallback callback)
{
#if TIDE_ENABLE_PLANAR_CONTROLLER
    if (_screenController && _screenController->getState() != ScreenState::off)
    {
        auto cb = [this, callback](const bool success) {
            if (success)
            {
                print_log(LOG_INFO, LOG_POWER, "Powered off the screens");
                _sceneController->hideLauncher();
            }
            else
                print_log(LOG_ERROR, LOG_POWER,
                          "Could not power off the screens");

            if (callback)
                callback(success);
        };
        _screenController->powerOff(cb);
        return;
    }
#endif
    if (callback)
        callback(false);
}

void AppController::resume(BoolCallback callback)
{
#if TIDE_ENABLE_PLANAR_CONTROLLER
    if (_screenController && _screenController->getState() != ScreenState::on)
    {
        auto cb = [callback](const bool success) {
            if (success)
                print_log(LOG_INFO, LOG_POWER, "Powered on the screens");
            else
                print_log(LOG_ERROR, LOG_POWER,
                          "Could not power on the screens");

            if (callback)
                callback(success);
        };
        _screenController->powerOn(cb);
        return;
    }
#endif
    if (callback)
        callback(false);
}

void AppController::handleTouchEvent(const uint numTouchPoints)
{
    if (_inactivityTimer)
        _inactivityTimer->restart();

    if (numTouchPoints >= _config.settings.touchpointsToWakeup)
        resume(BoolCallback());
}

void AppController::_initScreenController(const Configuration& config)
{
#if TIDE_ENABLE_PLANAR_CONTROLLER
    _screenController =
        ScreenControllerFactory::create(config.master.planarSerialPort);

    _inactivityTimer =
        std::make_unique<InactivityTimer>(config.settings.inactivityTimeout);

    connect(_inactivityTimer.get(), &InactivityTimer::poweroff, [this]() {
        print_log(LOG_INFO, LOG_POWER,
                  "Powering off the screens on inactivity timeout");
        suspend(BoolCallback());
    });

    connect(_inactivityTimer.get(), &InactivityTimer::countdownUpdated, this,
            &AppController::countdownUpdated);

    connect(_screenController.get(), &ScreenController::powerStateChanged,
            [this](const ScreenState state) {
                if (state == ScreenState::on)
                    _inactivityTimer->restart();
                else
                    _inactivityTimer->stop();
            });

    connect(_screenController.get(), &ScreenController::powerStateChanged,
            [this](const ScreenState state) {
                if (state == ScreenState::off)
                    _lock.unlock();
            });

    connect(_screenController.get(), &ScreenController::powerStateChanged, this,
            &AppController::screenStateChanged);
#else
    put_log(LOG_WARN, LOG_POWER,
            "Can't init screen controller for '%s': Tide was not compiled with "
            "planar controller support",
            config.master.planarSerialPort.toLocal8Bit().constData());
#endif
}
void AppController::_connect(ScreenLock& lock,
                             PixelStreamWindowManager& pixelStreamWindowManager)
{
    connect(&pixelStreamWindowManager,
            &PixelStreamWindowManager::streamWindowClosed, &lock,
            &ScreenLock::cancelStreamAcceptance);

    connect(&pixelStreamWindowManager,
            &PixelStreamWindowManager::externalStreamOpening, &lock,
            &ScreenLock::requestStreamAcceptance);

    connect(&lock, &ScreenLock::streamAccepted, &pixelStreamWindowManager,
            &PixelStreamWindowManager::showWindows);

    connect(&lock, &ScreenLock::streamRejected, &pixelStreamWindowManager,
            &PixelStreamWindowManager::handleStreamEnd);
}

void AppController::_connect(deflect::server::Server& deflectServer,
                             PixelStreamWindowManager& pixelStreamWindowManager)
{
    connect(&deflectServer, &deflect::server::Server::pixelStreamOpened,
            &pixelStreamWindowManager,
            &PixelStreamWindowManager::handleStreamStart);

    connect(&deflectServer, &deflect::server::Server::pixelStreamException,
            [](const QString uri, const QString what) {
                print_log(LOG_WARN, LOG_STREAM,
                          "Stream '%s' encountered an exception: '%s'",
                          uri.toLocal8Bit().constData(),
                          what.toLocal8Bit().constData());
            });

    connect(&deflectServer, &deflect::server::Server::pixelStreamClosed,
            &pixelStreamWindowManager,
            &PixelStreamWindowManager::handleStreamEnd);

    connect(&pixelStreamWindowManager,
            &PixelStreamWindowManager::streamWindowClosed, &deflectServer,
            &deflect::server::Server::closePixelStream);

    connect(&deflectServer, &deflect::server::Server::receivedFrame,
            &pixelStreamWindowManager,
            &PixelStreamWindowManager::updateStreamWindows);

    connect(&pixelStreamWindowManager,
            &PixelStreamWindowManager::requestFirstFrame, &deflectServer,
            &deflect::server::Server::requestFrame);

    connect(&deflectServer, &deflect::server::Server::registerToEvents,
            &pixelStreamWindowManager,
            &PixelStreamWindowManager::registerEventReceiver);

    connect(&deflectServer, &deflect::server::Server::receivedSizeHints,
            &pixelStreamWindowManager,
            &PixelStreamWindowManager::updateSizeHints);

    connect(&deflectServer, &deflect::server::Server::receivedData,
            &pixelStreamWindowManager,
            &PixelStreamWindowManager::sendDataToWindow);
}

void AppController::_connect(Options& options,
                             PixelStreamWindowManager& pixelStreamWindowManager)
{
    pixelStreamWindowManager.setAutoFocusNewWindows(
        options.getAutoFocusPixelStreams());
    connect(&options, &Options::autoFocusPixelStreamsChanged,
            &pixelStreamWindowManager,
            &PixelStreamWindowManager::setAutoFocusNewWindows);
}

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

#include "WallApplication.h"

#include "DataProvider.h"
#include "QmlTypeRegistration.h"
#include "RenderController.h"
#include "WallConfiguration.h"
#include "WallWindow.h"
#include "log.h"
#include "network/MPIChannel.h"
#include "network/WallFromMasterChannel.h"
#include "network/WallToMasterChannel.h"
#include "network/WallToWallChannel.h"

#include <stdexcept>

#include <QQuickRenderControl>
#include <QThreadPool>

WallApplication::WallApplication(int& argc_, char** argv_,
                                 const QString& config,
                                 MPIChannelPtr worldChannel,
                                 MPIChannelPtr wallChannel)
    : QGuiApplication{argc_, argv_}
    , _config{new WallConfiguration{config, worldChannel->getRank()}}
    , _provider{new DataProvider}
    , _wallChannel{new WallToWallChannel{wallChannel}}
{
    core::registerQmlTypes();

    // avoid overcommit for async content loading; consider number of processes
    // on the same machine
    const auto prCount = _config->getProcessCountForHost();
    const int maxThreads = std::max(QThread::idealThreadCount() / prCount, 2);
    QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);

    _initWallWindows();
    _initMPIConnection(worldChannel);
}

WallApplication::~WallApplication()
{
    if (_wallChannel->getRank() == 0)
    {
        // Make sure the send quit happens after any pending sendRequestFrame.
        // The MasterFromWallChannel is waiting for this signal to exit.
        // This step ensures that the queue of messages is flushed and the MPI
        // connection will not block when closing itself.
        QMetaObject::invokeMethod(_toMasterChannel.get(), "sendQuit",
                                  Qt::BlockingQueuedConnection);
    }

    _mpiReceiveThread.quit();
    _mpiReceiveThread.wait();

    _mpiSendThread.quit();
    _mpiSendThread.wait();
}

void WallApplication::_initWallWindows()
{
    std::vector<WallWindow*> windows;
    try
    {
        for (uint screen = 0; screen < _config->getScreens().size(); ++screen)
            windows.push_back(_makeWindow(screen));
    }
    catch (const std::runtime_error& e)
    {
        put_flog(LOG_FATAL, "Error creating WallWindow: '%s'", e.what());
        throw std::runtime_error("WallApplication: initialization failed.");
    }

    const auto swapSync = _config->getSwapSync();
    if (swapSync == SwapSync::hardware)
        put_log(LOG_DEBUG, "Launching with hardware swap synchronization...");

    _renderController.reset(new RenderController(std::move(windows), *_provider,
                                                 *_wallChannel, swapSync));
}

WallWindow* WallApplication::_makeWindow(const uint screen)
{
    return new WallWindow(*_config, screen, *_provider,
                          make_unique<QQuickRenderControl>(this));
}

void WallApplication::_initMPIConnection(MPIChannelPtr worldChannel)
{
    _fromMasterChannel.reset(new WallFromMasterChannel(worldChannel));
    _toMasterChannel.reset(new WallToMasterChannel(worldChannel));

    _fromMasterChannel->moveToThread(&_mpiReceiveThread);
    _toMasterChannel->moveToThread(&_mpiSendThread);

    connect(_fromMasterChannel.get(), &WallFromMasterChannel::receivedQuit,
            _renderController.get(), &RenderController::updateQuit);

    connect(_fromMasterChannel.get(),
            &WallFromMasterChannel::receivedScreenshotRequest,
            _renderController.get(),
            &RenderController::updateRequestScreenshot);

    connect(_fromMasterChannel.get(), SIGNAL(received(DisplayGroupPtr)),
            _renderController.get(), SLOT(updateDisplayGroup(DisplayGroupPtr)));

    connect(_fromMasterChannel.get(), SIGNAL(received(OptionsPtr)),
            _renderController.get(), SLOT(updateOptions(OptionsPtr)));

    connect(_fromMasterChannel.get(), SIGNAL(received(InactivityTimerPtr)),
            _renderController.get(),
            SLOT(updateInactivityTimer(InactivityTimerPtr)));

    connect(_fromMasterChannel.get(), SIGNAL(received(MarkersPtr)),
            _renderController.get(), SLOT(updateMarkers(MarkersPtr)));

    connect(_fromMasterChannel.get(), SIGNAL(received(deflect::FramePtr)),
            _renderController.get(), SLOT(requestRender()));

    connect(_renderController.get(), &RenderController::screenshotRendered,
            _toMasterChannel.get(), &WallToMasterChannel::sendScreenshot);

    connect(_fromMasterChannel.get(), SIGNAL(received(deflect::FramePtr)),
            _provider.get(), SLOT(setNewFrame(deflect::FramePtr)));

    if (_wallChannel->getRank() == 0)
    {
        connect(_provider.get(), &DataProvider::requestFrame,
                _toMasterChannel.get(), &WallToMasterChannel::sendRequestFrame);
        connect(_provider.get(), &DataProvider::closePixelStream,
                _toMasterChannel.get(),
                &WallToMasterChannel::sendPixelStreamClose);
    }

    connect(&_mpiReceiveThread, &QThread::started, _fromMasterChannel.get(),
            &WallFromMasterChannel::processMessages);

    _mpiReceiveThread.setObjectName("Recv");
    _mpiSendThread.setObjectName("Send");
    _mpiReceiveThread.start();
    _mpiSendThread.start();
}

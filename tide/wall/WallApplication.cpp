/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "WallApplication.h"

#include "DataProvider.h"
#include "log.h"
#include "network/MPIChannel.h"
#include "network/WallFromMasterChannel.h"
#include "network/WallToMasterChannel.h"
#include "network/WallToWallChannel.h"
#include "QmlTypeRegistration.h"
#include "RenderController.h"
#include "WallConfiguration.h"
#include "WallWindow.h"

#include <stdexcept>

#include <QThreadPool>
#include <QQuickRenderControl>

WallApplication::WallApplication( int& argc_, char** argv_,
                                  const QString& config,
                                  MPIChannelPtr worldChannel,
                                  MPIChannelPtr wallChannel )
    : QGuiApplication( argc_, argv_ )
    , _config( new WallConfiguration( config, worldChannel->getRank( )))
    , _wallChannel( new WallToWallChannel( wallChannel ))
{
    core::registerQmlTypes();

    // avoid overcommit for async content loading; consider number of processes
    // on the same machine
    const auto prCount = _config->getProcessCountForHost();
    const int maxThreads = std::max( QThread::idealThreadCount() / prCount, 2 );
    QThreadPool::globalInstance()->setMaxThreadCount( maxThreads );

    _initWallWindow();
    _initMPIConnection( worldChannel );
}

WallApplication::~WallApplication()
{
    if( _wallChannel->getRank() == 0 )
    {
        // Make sure the send quit happens after any pending sendRequestFrame.
        // The MasterFromWallChannel is waiting for this signal to exit.
        // This step ensures that the queue of messages is flushed and the MPI
        // connection will not block when closing itself.
        QMetaObject::invokeMethod( _toMasterChannel.get(), "sendQuit",
                                   Qt::BlockingQueuedConnection );
    }

    _mpiReceiveThread.quit();
    _mpiReceiveThread.wait();

    _mpiSendThread.quit();
    _mpiSendThread.wait();
}

void WallApplication::_initWallWindow()
{
    try
    {
        _window = new WallWindow( *_config, new QQuickRenderControl( this ),
                                  *_wallChannel );
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Error creating WallWindow: '%s'",
                  e.what( ));
        throw std::runtime_error( "WallApplication: initialization failed." );
    }

    _renderController.reset( new RenderController( *_window ));
}

void WallApplication::_initMPIConnection( MPIChannelPtr worldChannel )
{
    _fromMasterChannel.reset( new WallFromMasterChannel( worldChannel ));
    _toMasterChannel.reset( new WallToMasterChannel( worldChannel ));

    _fromMasterChannel->moveToThread( &_mpiReceiveThread );
    _toMasterChannel->moveToThread( &_mpiSendThread );

    connect( _fromMasterChannel.get(), SIGNAL( receivedQuit( )),
             _renderController.get(), SLOT( updateQuit( )));

    connect( _fromMasterChannel.get(),
             &WallFromMasterChannel::receivedScreenshotRequest,
             _renderController.get(),
             &RenderController::updateRequestScreenshot );

    connect( _fromMasterChannel.get(), SIGNAL( received( DisplayGroupPtr )),
             _renderController.get(), SLOT( updateDisplayGroup( DisplayGroupPtr )));

    connect( _fromMasterChannel.get(), SIGNAL( received( OptionsPtr )),
             _renderController.get(), SLOT( updateOptions( OptionsPtr )));

    connect( _fromMasterChannel.get(), SIGNAL( received( MarkersPtr )),
             _renderController.get(), SLOT( updateMarkers( MarkersPtr )));

    connect( _fromMasterChannel.get(),
             SIGNAL( received( deflect::FramePtr )),
             &_window->getDataProvider(),
             SLOT( setNewFrame( deflect::FramePtr )));

    connect( _fromMasterChannel.get(), SIGNAL( received( deflect::FramePtr )),
             _renderController.get(), SLOT( requestRender( )));

    connect( _renderController.get(), &RenderController::screenshotRendered,
             _toMasterChannel.get(), &WallToMasterChannel::sendScreenshot );

    if( _wallChannel->getRank() == 0 )
    {
        connect( &_window->getDataProvider(), &DataProvider::requestFrame,
                 _toMasterChannel.get(),
                 &WallToMasterChannel::sendRequestFrame );
    }

    connect( &_mpiReceiveThread, SIGNAL( started( )),
             _fromMasterChannel.get(), SLOT( processMessages( )));

    _mpiReceiveThread.setObjectName( "Recv" );
    _mpiSendThread.setObjectName( "Send" );
    _mpiReceiveThread.start();
    _mpiSendThread.start();
}

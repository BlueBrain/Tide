/*********************************************************************/
/* Copyright (c) 2014-2016, EPFL/Blue Brain Project                  */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "CommandLineParameters.h"
#include "localstreamer/PixelStreamerLauncher.h"
#include "log.h"
#include "MasterConfiguration.h"
#include "network/MasterToForkerChannel.h"
#include "network/MasterToWallChannel.h"
#include "network/MasterFromWallChannel.h"
#include "PixelStreamWindowManager.h"
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
#include "scene/Options.h"
#include "scene/Markers.h"
#include "StateSerializationHelper.h"
#include "QmlTypeRegistration.h"
#include "ui/DisplayGroupView.h"
#include "ui/MasterWindow.h"

#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
#  include "MultitouchListener.h"
#endif

#if TIDE_ENABLE_REST_INTERFACE
#  include "ContentLoader.h"
#  include "LoggingUtility.h"
#  include "rest/RestInterface.h"
#endif

#include <deflect/EventReceiver.h>
#include <deflect/FrameDispatcher.h>
#include <deflect/Server.h>

#include <stdexcept>

namespace
{
const int MOUSE_MARKER_ID = INT_MAX; // TUIO touch point IDs start at 0
}

MasterApplication::MasterApplication( int& argc_, char** argv_,
                                      MPIChannelPtr worldChannel,
                                      MPIChannelPtr forkChannel )
    : QApplication( argc_, argv_ )
    , _masterToForkerChannel( new MasterToForkerChannel( forkChannel ))
    , _masterToWallChannel( new MasterToWallChannel( worldChannel ))
    , _masterFromWallChannel( new MasterFromWallChannel( worldChannel ))
    , _markers( new Markers )
{
    master::registerQmlTypes();

    // don't create touch points for mouse events and vice versa
    setAttribute( Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false );
    setAttribute( Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false );

    CommandLineParameters options( argc_, argv_ );
    if( options.getHelp( ))
        options.showSyntax();

    if( !_createConfig( options.getConfigFilename( )))
        throw std::runtime_error( "MasterApplication: initialization failed." );

    _init();

    const QString& session = options.getSessionFilename();
    if( !session.isEmpty( ))
        _loadSessionOp.setFuture(
                    StateSerializationHelper( _displayGroup ).load( session ));
}

MasterApplication::~MasterApplication()
{
    _deflectServer.reset();
    _pixelStreamerLauncher.reset();

    // Make sure the send quit happens after any pending send operation;
    // If a send operation is not matched by a receive, the MPI connection
    // will block indefintely when trying to disconnect.
    QMetaObject::invokeMethod( _masterToForkerChannel.get(), "sendQuit",
                               Qt::BlockingQueuedConnection );
    QMetaObject::invokeMethod( _masterToWallChannel.get(), "sendQuit",
                               Qt::BlockingQueuedConnection );
    _mpiSendThread.quit();
    _mpiSendThread.wait();

    // This will wait for the quit reply from the first wall process
    _mpiReceiveThread.quit();
    _mpiReceiveThread.wait();
}

void MasterApplication::_init()
{
    _displayGroup.reset( new DisplayGroup( _config->getTotalSize( )));

    _masterWindow.reset( new MasterWindow( _displayGroup, *_config ));
    _pixelStreamWindowManager.reset(
                new PixelStreamWindowManager( *_displayGroup ));

    connect( &_loadSessionOp, &QFutureWatcher<DisplayGroupConstPtr>::finished,
             [this]()
    {
        auto group = _loadSessionOp.result();
        if( !group )
            return;

        _displayGroup->setContentWindows( group->getContentWindows( ));
        _displayGroup->setShowWindowTitles( group->getShowWindowTitles( ));
        _masterWindow->getOptions()->setShowWindowTitles(
                    group->getShowWindowTitles( ));
    });

    _initPixelStreamLauncher();
    _startDeflectServer();
    _initMPIConnection();

    // send initial display group to wall processes so that they at least the
    // real display group size to compute correct sizes for full screen etc.
    // which is vital for the following restoreBackground().
    _masterToWallChannel->sendAsync( _displayGroup );

    _restoreBackground();

#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
    _initTouchListener();
#endif

#if TIDE_ENABLE_REST_INTERFACE
    _initRestInterface();
#endif
}

bool MasterApplication::_createConfig( const QString& filename )
{
    try
    {
        _config.reset( new MasterConfiguration( filename ));
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Could not load configuration. '%s'", e.what( ));
        return false;
    }
    return true;
}

void MasterApplication::_startDeflectServer()
{
    if( _deflectServer )
        return;
    try
    {
        _deflectServer.reset( new deflect::Server );
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Could not start Deflect server: '%s'", e.what( ));
        return;
    }

    deflect::FrameDispatcher& dispatcher =
            _deflectServer->getPixelStreamDispatcher();

    connect( &dispatcher, &deflect::FrameDispatcher::openPixelStream,
             _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::openPixelStreamWindow );
    connect( &dispatcher, &deflect::FrameDispatcher::deletePixelStream,
             _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::closePixelStreamWindow );
    connect( _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::pixelStreamWindowClosed,
             &dispatcher, &deflect::FrameDispatcher::deleteStream );
}

void MasterApplication::_restoreBackground()
{
    OptionsPtr options = _masterWindow->getOptions();
    options->setBackgroundColor( _config->getBackgroundColor( ));

    const QString& uri = _config->getBackgroundUri();
    if( !uri.isEmpty( ))
    {
        ContentPtr content = ContentFactory::getContent( uri );
        if( !content )
            content = ContentFactory::getErrorContent();
        options->setBackgroundContent( content );
    }
}

void MasterApplication::_initPixelStreamLauncher()
{
    _pixelStreamerLauncher.reset(
             new PixelStreamerLauncher( *_pixelStreamWindowManager, *_config ));

    connect( _masterWindow.get(),
             &MasterWindow::openWebBrowser,
             _pixelStreamerLauncher.get(),
             &PixelStreamerLauncher::openWebBrowser );
    connect( _masterWindow.get(), &MasterWindow::openLauncher,
             _pixelStreamerLauncher.get(),
             &PixelStreamerLauncher::openLauncher );
    connect( _masterWindow.get(), &MasterWindow::hideLauncher,
             _pixelStreamerLauncher.get(),
             &PixelStreamerLauncher::hideLauncher );
}

void MasterApplication::_initMPIConnection()
{
    _masterToForkerChannel->moveToThread( &_mpiSendThread );
    _masterToWallChannel->moveToThread( &_mpiSendThread );
    _masterFromWallChannel->moveToThread( &_mpiReceiveThread );

    connect( _pixelStreamerLauncher.get(), &PixelStreamerLauncher::start,
             _masterToForkerChannel.get(), &MasterToForkerChannel::sendStart );

    connect( _displayGroup.get(), &DisplayGroup::modified,
             _masterToWallChannel.get(),
             [this]( DisplayGroupPtr displayGroup )
                { _masterToWallChannel->sendAsync( displayGroup ); },
             Qt::DirectConnection );

    connect( _masterWindow->getOptions().get(), &Options::updated,
             _masterToWallChannel.get(),
             [this]( OptionsPtr options )
                { _masterToWallChannel->sendAsync( options ); },
             Qt::DirectConnection );

    connect( _markers.get(), &Markers::updated,
             _masterToWallChannel.get(),
             [this]( MarkersPtr markers )
                { _masterToWallChannel->sendAsync( markers ); },
             Qt::DirectConnection );

    connect( &_deflectServer->getPixelStreamDispatcher(),
             &deflect::FrameDispatcher::sendFrame,
             _masterToWallChannel.get(),
             &MasterToWallChannel::send );
    connect( &_deflectServer->getPixelStreamDispatcher(),
             &deflect::FrameDispatcher::sendFrame,
             _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::updateStreamDimensions );
    connect( _deflectServer.get(),
             &deflect::Server::registerToEvents,
             _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::registerEventReceiver );
    connect( _deflectServer.get(), &deflect::Server::receivedSizeHints,
             _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::updateSizeHints );
    connect( _deflectServer.get(), &deflect::Server::receivedData,
             _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::sendDataToWindow );

    connect( _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::pixelStreamWindowClosed,
             _deflectServer.get(), &deflect::Server::onPixelStreamerClosed );
    connect( _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::eventRegistrationReply,
             _deflectServer.get(),
             &deflect::Server::onEventRegistrationReply );
    connect( _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::requestFirstFrame,
             &_deflectServer->getPixelStreamDispatcher(),
             &deflect::FrameDispatcher::requestFrame );

    _pixelStreamWindowManager->setAutoFocusNewWindows(
                _masterWindow->getOptions()->getAutoFocusPixelStreams( ));
    connect( _masterWindow->getOptions().get(),
             &Options::autoFocusPixelStreamsChanged,
             _pixelStreamWindowManager.get(),
             &PixelStreamWindowManager::setAutoFocusNewWindows );

    connect( _masterFromWallChannel.get(),
             &MasterFromWallChannel::receivedRequestFrame,
             &_deflectServer->getPixelStreamDispatcher(),
             &deflect::FrameDispatcher::requestFrame );

    connect( &_mpiReceiveThread, &QThread::started,
             _masterFromWallChannel.get(),
             &MasterFromWallChannel::processMessages );

    _mpiSendThread.start();
    _mpiReceiveThread.start();
}

#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
void MasterApplication::_initTouchListener()
{
    DisplayGroupView* view = _masterWindow->getDisplayGroupView();
    auto mapFunc = std::bind( &DisplayGroupView::mapToWallPos, view,
                              std::placeholders::_1 );
    _touchInjector.reset( new deflect::qt::TouchInjector{ *view, mapFunc } );
    _touchListener.reset( new MultitouchListener( ));

    connect( _touchListener.get(), &MultitouchListener::touchPointAdded,
             _touchInjector.get(), &deflect::qt::TouchInjector::addTouchPoint );
    connect( _touchListener.get(), &MultitouchListener::touchPointUpdated,
             _touchInjector.get(),
             &deflect::qt::TouchInjector::updateTouchPoint );
    connect( _touchListener.get(), &MultitouchListener::touchPointRemoved,
             _touchInjector.get(),
             &deflect::qt::TouchInjector::removeTouchPoint );

    const auto wallSize = _config->getTotalSize();
    auto getWallPos = [wallSize]( const QPointF& normalizedPos )
    {
        return QPointF{ normalizedPos.x() * wallSize.width(),
                        normalizedPos.y() * wallSize.height( )};
    };
    connect( _touchListener.get(), &MultitouchListener::touchPointAdded,
             [this, getWallPos]( const int id, const QPointF normalizedPos )
    {
        _markers->addMarker( id, getWallPos( normalizedPos ));
    });
    connect( _touchListener.get(), &MultitouchListener::touchPointUpdated,
             [this, getWallPos]( const int id, const QPointF normalizedPos )
    {
        _markers->updateMarker( id, getWallPos( normalizedPos ));
    });
    connect( _touchListener.get(), &MultitouchListener::touchPointRemoved,
             [this]( const int id, const QPointF )
    {
        _markers->removeMarker( id );
    });

    connect( view, &DisplayGroupView::mousePressed, [this]( const QPointF pos )
    {
        _markers->addMarker( MOUSE_MARKER_ID, pos );
    });
    connect( view, &DisplayGroupView::mouseMoved, [this]( const QPointF pos )
    {
        _markers->updateMarker( MOUSE_MARKER_ID, pos );
    });
    connect( view, &DisplayGroupView::mouseReleased, [this]( const QPointF )
    {
        _markers->removeMarker( MOUSE_MARKER_ID );
    });
}
#endif

#if TIDE_ENABLE_REST_INTERFACE
void MasterApplication::_initRestInterface()
{
    _restInterface = make_unique<RestInterface>( _config->getWebServicePort(),
                                                 _masterWindow->getOptions( ));
    _logger = make_unique<LoggingUtility>();

    connect( _restInterface.get(), &RestInterface::browse, [this]( QString uri )
    {
#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
        if( uri.isEmpty( ))
            uri = _config->getWebBrowserDefaultURL();
        _pixelStreamerLauncher->openWebBrowser( QPointF(), QSize(), uri );
#else
        put_flog( LOG_INFO, "Can't browse url '%s', Tide was compiled without"
                            "webbrowser support",
                  uri.toLocal8Bit().constData( ));
#endif
    });

    connect( _restInterface.get(), &RestInterface::whiteboard, [this]()
    {
        _pixelStreamerLauncher->openWhiteboard();
    });

    connect( _restInterface.get(), &RestInterface::open, [this]( QString uri )
    {
        ContentLoader( _displayGroup ).load( uri );
    });
    connect( _restInterface.get(), &RestInterface::load, [this]( QString uri )
    {
        _loadSessionOp.setFuture(
                    StateSerializationHelper( _displayGroup ).load( uri ));
    });
    connect( _restInterface.get(), &RestInterface::save, [this]( QString uri )
    {
        _displayGroup->setShowWindowTitles(
                    _masterWindow->getOptions()->getShowWindowTitles( ));
        _saveSessionOp.setFuture(
                    StateSerializationHelper( _displayGroup ).save( uri ));
    });
    connect( _restInterface.get(), &RestInterface::clear, [this]()
    {
        _displayGroup->clear();
    });

    connect( _displayGroup.get(), &DisplayGroup::contentWindowAdded,
             _logger.get(), &LoggingUtility::contentWindowAdded );

    connect( _displayGroup.get(), &DisplayGroup::contentWindowRemoved,
            _logger.get(), &LoggingUtility::contentWindowRemoved );

    connect( _displayGroup.get(), &DisplayGroup::contentWindowMovedToFront,
           _logger.get(), &LoggingUtility::contentWindowMovedToFront );

    _restInterface.get()->setLogger( *(_logger.get()) );
}
#endif

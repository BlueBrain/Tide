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

#include "log.h"
#include "CommandLineParameters.h"
#include "MasterWindow.h"
#include "DisplayGroup.h"
#include "DisplayGroupView.h"
#include "ContentFactory.h"
#include "MasterConfiguration.h"
#include "MasterToForkerChannel.h"
#include "MasterToWallChannel.h"
#include "MasterFromWallChannel.h"
#include "Options.h"
#include "Markers.h"
#include "QmlTypeRegistration.h"

#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
#  include "MultiTouchListener.h"
#endif

#include "localstreamer/PixelStreamerLauncher.h"
#include "StateSerializationHelper.h"
#include "PixelStreamWindowManager.h"

#if TIDE_ENABLE_REST_INTERFACE
#  include "ContentLoader.h"
#  include "LoggingUtility.h"
#  include "rest/RestInterface.h"
#endif

#include <deflect/EventReceiver.h>
#include <deflect/FrameDispatcher.h>
#include <deflect/Server.h>

#include <stdexcept>

MasterApplication::MasterApplication( int& argc_, char** argv_,
                                      MPIChannelPtr worldChannel,
                                      MPIChannelPtr forkChannel )
    : QApplication( argc_, argv_ )
    , masterToForkerChannel_( new MasterToForkerChannel( forkChannel ))
    , masterToWallChannel_( new MasterToWallChannel( worldChannel ))
    , masterFromWallChannel_( new MasterFromWallChannel( worldChannel ))
    , markers_( new Markers )
{
    master::registerQmlTypes();

    // don't create touch points for mouse events and vice versa
    setAttribute( Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false );
    setAttribute( Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false );

    CommandLineParameters options( argc_, argv_ );
    if( options.getHelp( ))
        options.showSyntax();

    if( !createConfig( options.getConfigFilename( )))
        throw std::runtime_error( "MasterApplication: initialization failed." );

    displayGroup_.reset( new DisplayGroup( config_->getTotalSize( )));

    init();

    const QString& session = options.getSessionFilename();
    if( !session.isEmpty( ))
    {
        StateSerializationHelper( displayGroup_ ).load( session );
        masterWindow_->getOptions()->setShowWindowTitles(
                    displayGroup_->getShowWindowTitles( ));
    }
}

MasterApplication::~MasterApplication()
{
    deflectServer_.reset();

    masterToForkerChannel_->sendQuit();
    masterToWallChannel_->sendQuit();

    mpiSendThread_.quit();
    mpiSendThread_.wait();

    mpiReceiveThread_.quit();
    mpiReceiveThread_.wait();;
}

void MasterApplication::init()
{
    masterWindow_.reset( new MasterWindow( displayGroup_, *config_ ));
    pixelStreamWindowManager_.reset(
                new PixelStreamWindowManager( *displayGroup_ ));

    initPixelStreamLauncher();
    startDeflectServer();
    initMPIConnection();

    // send initial display group to wall processes so that they at least the
    // real display group size to compute correct sizes for full screen etc.
    // which is vital for the following restoreBackground().
    masterToWallChannel_->sendAsync( displayGroup_ );

    restoreBackground();

#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
    initTouchListener();
#endif

#if TIDE_ENABLE_REST_INTERFACE
    _initRestInterface();
#endif
}

bool MasterApplication::createConfig( const QString& filename )
{
    try
    {
        config_.reset( new MasterConfiguration( filename ));
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Could not load configuration. '%s'", e.what( ));
        return false;
    }
    return true;
}

void MasterApplication::startDeflectServer()
{
    if( deflectServer_ )
        return;
    try
    {
        deflectServer_.reset( new deflect::Server );
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Could not start Deflect server: '%s'", e.what( ));
        return;
    }

    deflect::FrameDispatcher& dispatcher =
            deflectServer_->getPixelStreamDispatcher();

    connect( &dispatcher, &deflect::FrameDispatcher::openPixelStream,
             pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::openPixelStreamWindow );
    connect( &dispatcher, &deflect::FrameDispatcher::deletePixelStream,
             pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::closePixelStreamWindow );
    connect( pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::pixelStreamWindowClosed,
             &dispatcher, &deflect::FrameDispatcher::deleteStream );
}

void MasterApplication::restoreBackground()
{
    OptionsPtr options = masterWindow_->getOptions();
    options->setBackgroundColor( config_->getBackgroundColor( ));

    const QString& uri = config_->getBackgroundUri();
    if( !uri.isEmpty( ))
    {
        ContentPtr content = ContentFactory::getContent( uri );
        if( !content )
            content = ContentFactory::getErrorContent();
        options->setBackgroundContent( content );
    }
}

void MasterApplication::initPixelStreamLauncher()
{
    pixelStreamerLauncher_.reset(
             new PixelStreamerLauncher( *pixelStreamWindowManager_, *config_ ));

    connect( masterWindow_.get(),
             &MasterWindow::openWebBrowser,
             pixelStreamerLauncher_.get(),
             &PixelStreamerLauncher::openWebBrowser );
    connect( masterWindow_.get(), &MasterWindow::openLauncher,
             pixelStreamerLauncher_.get(),
             &PixelStreamerLauncher::openLauncher );
    connect( masterWindow_.get(), &MasterWindow::hideLauncher,
             pixelStreamerLauncher_.get(),
             &PixelStreamerLauncher::hideLauncher );
}

void MasterApplication::initMPIConnection()
{
    masterToForkerChannel_->moveToThread( &mpiSendThread_ );
    masterToWallChannel_->moveToThread( &mpiSendThread_ );
    masterFromWallChannel_->moveToThread( &mpiReceiveThread_ );

    connect( pixelStreamerLauncher_.get(), &PixelStreamerLauncher::start,
             masterToForkerChannel_.get(), &MasterToForkerChannel::sendStart );

    connect( displayGroup_.get(), &DisplayGroup::modified,
             masterToWallChannel_.get(),
             [this]( DisplayGroupPtr displayGroup )
                { masterToWallChannel_->sendAsync( displayGroup ); },
             Qt::DirectConnection );

    connect( masterWindow_->getOptions().get(), &Options::updated,
             masterToWallChannel_.get(),
             [this]( OptionsPtr options )
                { masterToWallChannel_->sendAsync( options ); },
             Qt::DirectConnection );

    connect( markers_.get(), &Markers::updated,
             masterToWallChannel_.get(),
             [this]( MarkersPtr markers )
                { masterToWallChannel_->sendAsync( markers ); },
             Qt::DirectConnection );

    connect( &deflectServer_->getPixelStreamDispatcher(),
             &deflect::FrameDispatcher::sendFrame,
             masterToWallChannel_.get(),
             &MasterToWallChannel::send );
    connect( &deflectServer_->getPixelStreamDispatcher(),
             &deflect::FrameDispatcher::sendFrame,
             pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::updateStreamDimensions );
    connect( deflectServer_.get(),
             &deflect::Server::registerToEvents,
             pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::registerEventReceiver );
    connect( deflectServer_.get(), &deflect::Server::receivedSizeHints,
             pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::updateSizeHints );

    connect( pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::pixelStreamWindowClosed,
             deflectServer_.get(), &deflect::Server::onPixelStreamerClosed );
    connect( pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::eventRegistrationReply,
             deflectServer_.get(),
             &deflect::Server::onEventRegistrationReply );

    pixelStreamWindowManager_->setAutoFocusNewWindows(
                masterWindow_->getOptions()->getAutoFocusPixelStreams( ));
    connect( masterWindow_->getOptions().get(),
             &Options::autoFocusPixelStreamsChanged,
             pixelStreamWindowManager_.get(),
             &PixelStreamWindowManager::setAutoFocusNewWindows );

    connect( masterFromWallChannel_.get(),
             &MasterFromWallChannel::receivedRequestFrame,
             &deflectServer_->getPixelStreamDispatcher(),
             &deflect::FrameDispatcher::requestFrame );

    connect( &mpiReceiveThread_, &QThread::started,
             masterFromWallChannel_.get(),
             &MasterFromWallChannel::processMessages );

    mpiSendThread_.start();
    mpiReceiveThread_.start();
}

#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
void MasterApplication::initTouchListener()
{
    DisplayGroupView* view = masterWindow_->getDisplayGroupView();
    touchListener_.reset( new MultiTouchListener(
                              *view, config_->getTotalSize( )));
    connect( touchListener_.get(), &MultiTouchListener::touchPointAdded,
             markers_.get(), &Markers::addMarker );
    connect( touchListener_.get(), &MultiTouchListener::touchPointUpdated,
             markers_.get(), &Markers::updateMarker );
    connect( touchListener_.get(), &MultiTouchListener::touchPointRemoved,
             markers_.get(), &Markers::removeMarker );
}
#endif

#if TIDE_ENABLE_REST_INTERFACE
void MasterApplication::_initRestInterface()
{
    _restInterface = make_unique<RestInterface>( config_->getWebServicePort(),
                                                 masterWindow_->getOptions( ));
    _logger = make_unique<LoggingUtility>();

    connect( _restInterface.get(), &RestInterface::browse, [this]( QString uri )
    {
        if( uri.isEmpty( ))
            uri = config_->getWebBrowserDefaultURL();
        pixelStreamerLauncher_->openWebBrowser( QPointF(), QSize(), uri );
    });
    connect( _restInterface.get(), &RestInterface::open, [this]( QString uri ) {
        ContentLoader( displayGroup_ ).load( uri );
        masterWindow_->getOptions()->setShowWindowTitles(
                    displayGroup_->getShowWindowTitles( ));
    });
    connect( _restInterface.get(), &RestInterface::load, [this]( QString uri ) {
        StateSerializationHelper( displayGroup_ ).load( uri );
    });
    connect( _restInterface.get(), &RestInterface::save, [this]( QString uri ) {
        displayGroup_->setShowWindowTitles(
                    masterWindow_->getOptions()->getShowWindowTitles( ));
        StateSerializationHelper( displayGroup_ ).save( uri );
    });
    connect( _restInterface.get(), &RestInterface::clear, [this]() {
        displayGroup_->clear();
    });
    connect( displayGroup_.get(), &DisplayGroup::contentWindowAdded,
             _logger.get(), &LoggingUtility::contentWindowAdded );

    connect( displayGroup_.get(), &DisplayGroup::contentWindowRemoved,
            _logger.get(), &LoggingUtility::contentWindowRemoved );

    connect( displayGroup_.get(), &DisplayGroup::contentWindowMovedToFront,
           _logger.get(), &LoggingUtility::contentWindowMovedToFront );

    connect( masterWindow_.get(), &MasterWindow::hideLauncher,
              _logger.get(), &LoggingUtility::launcherHide );
    connect( masterWindow_.get(), &MasterWindow::openLauncher,
              _logger.get(), &LoggingUtility::launcherShow );

    _restInterface.get()->setLogger( *(_logger.get()) );
}
#endif

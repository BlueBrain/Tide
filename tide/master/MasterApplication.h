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

#ifndef MASTERAPPLICATION_H
#define MASTERAPPLICATION_H

#include "config.h"
#include "types.h"

#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
#include <deflect/qt/TouchInjector.h>
#endif

#include <QApplication>
#include <QFutureWatcher>
#include <QThread>

class MasterToWallChannel;
class MasterToForkerChannel;
class MasterFromWallChannel;
class MasterWindow;
class PixelStreamerLauncher;
class PixelStreamWindowManager;
class MasterConfiguration;
class MultitouchListener;
class RestInterface;
class LoggingUtility;

/**
 * The main application for the Master process.
 */
class MasterApplication : public QApplication
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param argc Command line argument count (required by QApplication)
     * @param argv Command line arguments (required by QApplication)
     * @param worldChannel The world MPI channel
     * @param forkChannel The MPI channel for forking processes
     * @throw std::runtime_error if an error occured during initialization
     */
    MasterApplication( int &argc, char **argv, MPIChannelPtr worldChannel,
                       MPIChannelPtr forkChannel );

    /** Destructor */
    virtual ~MasterApplication();

private:
    std::unique_ptr<MasterToForkerChannel> _masterToForkerChannel;
    std::unique_ptr<MasterToWallChannel> _masterToWallChannel;
    std::unique_ptr<MasterFromWallChannel> _masterFromWallChannel;
    std::unique_ptr<MasterWindow> _masterWindow;
    std::unique_ptr<MasterConfiguration> _config;
    std::unique_ptr<deflect::Server> _deflectServer;
    std::unique_ptr<PixelStreamerLauncher> _pixelStreamerLauncher;
    std::unique_ptr<PixelStreamWindowManager> _pixelStreamWindowManager;
#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
    std::unique_ptr<MultitouchListener> _touchListener;
    std::unique_ptr<deflect::qt::TouchInjector> _touchInjector;
#endif
#if TIDE_ENABLE_REST_INTERFACE
    std::unique_ptr<RestInterface> _restInterface;
    std::unique_ptr<LoggingUtility> _logger;
#endif
    QFutureWatcher<DisplayGroupConstPtr> _loadSessionOp;
    QFutureWatcher<bool> _saveSessionOp;

    DisplayGroupPtr _displayGroup;
    OptionsPtr _options;
    MarkersPtr _markers;

    QThread _mpiSendThread;
    QThread _mpiReceiveThread;

    void _init();
    bool _createConfig( const QString& filename );
    void _startDeflectServer();
    void _restoreBackground();
    void _initPixelStreamLauncher();
    void _initMPIConnection();
#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
    void _initTouchListener();
#endif
#if TIDE_ENABLE_REST_INTERFACE
    void _initRestInterface();
#endif
};

#endif

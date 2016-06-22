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

#include <QApplication>
#include <QFutureWatcher>
#include <QThread>

#include <boost/scoped_ptr.hpp>

class MasterToWallChannel;
class MasterToForkerChannel;
class MasterFromWallChannel;
class MasterWindow;
class PixelStreamerLauncher;
class PixelStreamWindowManager;
class MasterConfiguration;
class MultiTouchListener;
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
    boost::scoped_ptr<MasterToForkerChannel> masterToForkerChannel_;
    boost::scoped_ptr<MasterToWallChannel> masterToWallChannel_;
    boost::scoped_ptr<MasterFromWallChannel> masterFromWallChannel_;
    boost::scoped_ptr<MasterWindow> masterWindow_;
    boost::scoped_ptr<MasterConfiguration> config_;
    boost::scoped_ptr<deflect::Server> deflectServer_;
    boost::scoped_ptr<PixelStreamerLauncher> pixelStreamerLauncher_;
    boost::scoped_ptr<PixelStreamWindowManager> pixelStreamWindowManager_;
#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
    boost::scoped_ptr<MultiTouchListener> touchListener_;
#endif
#if TIDE_ENABLE_REST_INTERFACE
    std::unique_ptr<RestInterface> _restInterface;
    std::unique_ptr<LoggingUtility> _logger;
#endif
    QFutureWatcher<DisplayGroupConstPtr> _loadSessionOp;
    QFutureWatcher<bool> _saveSessionOp;

    DisplayGroupPtr displayGroup_;
    MarkersPtr markers_;

    QThread mpiSendThread_;
    QThread mpiReceiveThread_;

    void init();
    bool createConfig( const QString& filename );
    void startDeflectServer();
    void restoreBackground();
    void initPixelStreamLauncher();
    void initMPIConnection();

#if TIDE_ENABLE_TUIO_TOUCH_LISTENER
    void initTouchListener();
#endif
#if TIDE_ENABLE_REST_INTERFACE
    void _initRestInterface();
#endif
};

#endif // MASTERAPPLICATION_H

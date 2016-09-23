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

#ifndef WALLAPPLICATION_H
#define WALLAPPLICATION_H

#include "types.h"

#include <QApplication>
#include <QThread>
#include <boost/scoped_ptr.hpp>

class RenderController;
class WallFromMasterChannel;
class WallToMasterChannel;
class WallToWallChannel;

/**
 * The main application for Wall processes.
 *
 * The main loop exec() exits when the WallWindow is deleted
 * (quitOnLastWindowClosed propery).
 */
class WallApplication : public QApplication
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param argc Command line argument count (required by QApplication)
     * @param argv Command line arguments (required by QApplication)
     * @param worldChannel The world MPI channel
     * @param wallChannel The wall MPI channel
     * @throw std::runtime_error if an error occured during initialization
     */
    WallApplication( int &argc, char **argv, MPIChannelPtr worldChannel,
                     MPIChannelPtr wallChannel );

    /** Destructor */
    virtual ~WallApplication();

private:
    std::unique_ptr<WallConfiguration> _config;
    WallWindow* _window; // deleteLater from syncQuit in RenderController
    std::unique_ptr<RenderController> _renderController;

    std::unique_ptr<WallFromMasterChannel> _fromMasterChannel;
    std::unique_ptr<WallToMasterChannel> _toMasterChannel;
    std::unique_ptr<WallToWallChannel> _wallChannel;

    QThread _mpiSendThread;
    QThread _mpiReceiveThread;

    bool _createConfig( const QString& filename, const int rank );
    void _initWallWindow();
    void _initMPIConnection( MPIChannelPtr worldChannel );
};

#endif

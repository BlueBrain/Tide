/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
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

#ifndef WALLAPPLICATION_H
#define WALLAPPLICATION_H

#include "types.h"

#include <QGuiApplication>
#include <QThread>

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
class WallApplication : public QGuiApplication
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param argc Command line argument count (required by QApplication)
     * @param argv Command line arguments (required by QApplication)
     * @param masterRecvComm The communicator to receive from master process
     * @param masterSendComm The communicator to send messages to master process
     * @param wallToWallComm The communicator for scene synchronization
     * @param swapSyncBarrier The network barrier for frame swap synchronization
     * @throw std::runtime_error if an error occured during initialization
     */
    WallApplication(int& argc, char** argv, MPICommunicator& masterRecvComm,
                    MPICommunicator& masterSendComm,
                    MPICommunicator& wallToWallComm,
                    NetworkBarrier& swapSyncBarrier);

    /** Destructor */
    virtual ~WallApplication();

private:
    std::unique_ptr<WallConfiguration> _config;
    std::unique_ptr<DataProvider> _provider;
    std::unique_ptr<RenderController> _renderController;

    std::unique_ptr<WallFromMasterChannel> _fromMasterChannel;
    std::unique_ptr<WallToMasterChannel> _toMasterChannel;
    std::unique_ptr<WallToWallChannel> _wallChannel;

    QThread _mpiSendThread;
    QThread _mpiReceiveThread;

    void _initMPIConnections();
    void _terminateMPIConnections();
};

#endif

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

#ifndef WALLFROMMASTERCHANNEL_H
#define WALLFROMMASTERCHANNEL_H

#include "network/ReceiveBuffer.h"
#include "types.h"

#include <QObject>

/**
 * Receiving channel from the master application to the wall processes.
 */
class WallFromMasterChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WallFromMasterChannel)

public:
    /** Constructor */
    WallFromMasterChannel(MPICommunicator& communicator);

    /**
     * Receive the inital Configuration sent by the master process.
     * @return configuration object.
     */
    Configuration receiveConfiguration();

public slots:
    /**
     * Process messages until the QUIT message is received.
     * This method is blocking and should be called from the processing thread.
     */
    void processMessages();

signals:
    /**
     * Emitted when a scene was recieved.
     * @param scene The Scene that was received.
     */
    void received(ScenePtr scene);

    /**
     * Emitted when new Options were recieved.
     * @param options The options that were received.
     */
    void received(OptionsPtr options);

    /** Emitted when a new CountdownStatus was received.
     * @param status The status that was received.
     */
    void received(CountdownStatusPtr status);

    /**
     * Emitted when new ScreenLock was recieved.
     * @param lock The ScreenLock that was received.
     */
    void received(ScreenLockPtr lock);

    /**
     * Emitted when new Markers were recieved.
     * @param markers The markers that were received.
     */
    void received(MarkersPtr markers);

    /**
     * Emitted when a new PixelStream frame was recieved.
     * @param frame The frame that was received.
     */
    void received(deflect::server::FramePtr frame);

    /**
     * Emitted when a screenshot was requested.
     */
    void receivedScreenshotRequest();

    /**
     * Emitted when the quit message was recieved.
     */
    void receivedQuit();

private:
    MPICommunicator& _communicator;
    ReceiveBuffer _buffer;
    bool _processMessages = true;

    void receiveMessage();

    void receiveBroadcast(const size_t messageSize);
    template <typename T>
    T receiveBinaryBroadcast(const size_t messageSize);
    template <typename T>
    T receiveJsonBroadcast(const size_t messageSize);
    template <typename T>
    T receiveQObjectBroadcast(const size_t messageSize);
};

#endif

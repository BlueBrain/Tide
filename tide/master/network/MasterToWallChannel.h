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

#ifndef MASTERTOWALLCHANNEL_H
#define MASTERTOWALLCHANNEL_H

#include "network/MessageHeader.h"
#include "types.h"

#include <QObject>

/**
 * Sending channel from the master application to the wall processes.
 *
 * This class is designed to be moved to a separate QThread.
 *
 * The methods in this class are NOT thread-safe.
 *
 * The send() functions are fully synchronous and rely on Qt::QueuedConnection
 * for safe inter-thread communication.
 *
 * The sendAsync() functions are a workaround for objects that cannot be passed
 * by copy and also cannot provide a thread-safe serialize() function.
 * They can be called directly from the main thread (Qt::DirectConnection).
 * The given object is serialized synchronously (in the calling thread), then
 * the serialized data is sent asynchronously in the MasterToWallChannel's
 * thread.
 */
class MasterToWallChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MasterToWallChannel)

public:
    /** Constructor */
    MasterToWallChannel(MPICommunicator& communicator);

public slots:
    /**
     * Send the given Scene to the wall processes.
     * @param scene The Scene to send
     */
    void sendAsync(ScenePtr scene);

    /**
     * Send the given Options to the wall processes.
     * @param options The options to send
     */
    void sendAsync(OptionsPtr options);

    /**
     * Send the given CountdownStatus to the wall processes.
     * @param status The countdownStatus to send
     */
    void sendAsync(CountdownStatusPtr status);

    /**
     * Send the Lock to the wall processes.
     * @param lock The lock to send
     */
    void sendAsync(ScreenLockPtr lock);

    /**
     * Send the given Markers to the wall processes.
     * @param markers The markers to send
     */
    void sendAsync(MarkersPtr markers);

    /**
     * Send pixel stream frame to the wall processes.
     * @param frame The frame to send
     */
    void sendFrame(deflect::server::FramePtr frame);

    /**
     * Send the configuration to the wall processes.
     * @param config The configuration to send
     */
    void send(const Configuration& config);

    /**
     * Send a screenshot request to the wall processes.
     */
    void sendRequestScreenshot();

    /**
     * Send quit message to the wall processes, terminating the application.
     */
    void sendQuit();

private:
    MPICommunicator& _communicator;

    template <typename T>
    void broadcast(const T& object, const MessageType type);
    template <typename T>
    void broadcastAsync(const T& object, const MessageType type);

private slots:
    void _broadcast(MessageType type, std::string data);
};

#endif

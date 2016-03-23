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

#ifndef MASTERTOWALLCHANNEL_H
#define MASTERTOWALLCHANNEL_H

#include "types.h"
#include "MPIHeader.h"
#include "SerializeBuffer.h"

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

public:
    /** Constructor */
    MasterToWallChannel( MPIChannelPtr mpiChannel );

public slots:
    /**
     * Send the given DisplayGroup to the wall processes.
     * @param displayGroup The DisplayGroup to send
     */
    void sendAsync( DisplayGroupPtr displayGroup );

    /**
     * Send the given Options to the wall processes.
     * @param options The options to send
     */
    void sendAsync( OptionsPtr options );

    /**
     * Send the given Markers to the wall processes.
     * @param markers The markers to send
     */
    void sendAsync( MarkersPtr markers );

    /**
     * Send pixel stream frame to the wall processes.
     * @param frame The frame to send
     */
    void send( deflect::FramePtr frame );

    /**
     * Send quit message to the wall processes, terminating the application.
     */
    void sendQuit();

private:
    Q_DISABLE_COPY( MasterToWallChannel )

    MPIChannelPtr _mpiChannel;
    SerializeBuffer _buffer;
    SerializeBuffer _asyncBuffer;

    template< typename T >
    void broadcast( const T& object, const MPIMessageType type );
    template< typename T >
    void broadcastAsync( const T& object, const MPIMessageType type );

private slots:
    void _broadcast( MPIMessageType type, std::string data );
};

#endif // MASTERTOWALLCHANNEL_H

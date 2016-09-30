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

#ifndef WALLFROMMASTERCHANNEL_H
#define WALLFROMMASTERCHANNEL_H

#include "types.h"
#include "network/ReceiveBuffer.h"

#include <QObject>

/**
 * Receiving channel from the master application to the wall processes.
 */
class WallFromMasterChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( WallFromMasterChannel )

public:
    /** Constructor */
    WallFromMasterChannel( MPIChannelPtr mpiChannel );

    /** Check if a message is available from the Master process. */
    bool isMessageAvailable();

    /**
     * Receive a message.
     * A received() signal will be emitted according to the message type.
     * This method is blocking.
     */
    void receiveMessage();

public slots:
    /**
     * Process messages until the QUIT message is received.
     */
    void processMessages();

signals:
    /**
     * Emitted when a displayGroup was recieved
     * @see receiveMessage()
     * @param displayGroup The DisplayGroup that was received
     */
    void received( DisplayGroupPtr displayGroup );

    /**
     * Emitted when new Options were recieved
     * @see receiveMessage()
     * @param options The options that were received
     */
    void received( OptionsPtr options );

    /**
     * Emitted when new Markers were recieved
     * @see receiveMessage()
     * @param markers The markers that were received
     */
    void received( MarkersPtr markers );

    /**
     * Emitted when a new PixelStream frame was recieved
     * @see receiveMessage()
     * @param frame The frame that was received
     */
    void received( deflect::FramePtr frame );

    /**
     * Emitted when the quit message was recieved
     * @see receiveMessage()
     */
    void receivedQuit();

private:
    MPIChannelPtr _mpiChannel;
    ReceiveBuffer _buffer;
    bool _processMessages;

    template <typename T>
    T receiveBroadcast( const size_t messageSize );
    template <typename T>
    T receiveQObjectBroadcast( const size_t messageSize );
};

#endif

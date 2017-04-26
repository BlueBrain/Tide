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

#ifndef WALLTOWALLCHANNEL_H
#define WALLTOWALLCHANNEL_H

#include "network/ReceiveBuffer.h"
#include "types.h"

#include <QObject>

#include <chrono>

/**
 * Communication channel between the Wall processes.
 */
class WallToWallChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WallToWallChannel)

public:
    using clock = std::chrono::high_resolution_clock;

    /** Constructor */
    WallToWallChannel(MPIChannelPtr mpiChannel);

    /** @return The rank of this process. */
    int getRank() const;

    /**
     * Get the sum of the given local values across all processes.
     * @param localValue The value to sum
     * @return the sum of the localValues
     */
    int globalSum(int localValue) const;

    /** Check if all processes are ready to perform a common action. */
    bool allReady(bool isReady) const;

    /** Get the current timestamp, synchronized accross processes. */
    clock::time_point getTime() const;

    /** Synchronize clock time across all processes. */
    void synchronizeClock();

    /** Block execution until all programs have reached the barrier. */
    void globalBarrier() const;

    /** Check that all processes have the same version of an object. */
    bool checkVersion(uint64_t version) const;

    /**
     * Elect a leader amongst wall processes.
     * @param isCandidate Is this process a candidate.
     * @return the rank of the leader, or -1 if no leader could be elected.
     */
    int electLeader(bool isCandidate);

    /**
     * Broadcast a timestamp.
     * All other processes must recieve it with receiveTimestampBroadcast().
     */
    void broadcast(double timestamp);

    /** Receive a timestamp broadcasted by broadcast(timestamp). */
    double receiveTimestampBroadcast(int src);

private:
    MPIChannelPtr _mpiChannel;
    ReceiveBuffer _buffer;
    clock::time_point _timestamp;

    void _sendClock();
    void _receiveClock();
};

#endif

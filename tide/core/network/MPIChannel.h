/*********************************************************************/
/* Copyright (c) 2014-2015, EPFL/Blue Brain Project                  */
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

#ifndef MPICHANNEL_H
#define MPICHANNEL_H

#include "MPIHeader.h"
#include "types.h"

#include <mpi.h>

class MPIContext;

/**
 * The result of an MPIChannel::probe() operation
 */
struct ProbeResult
{
    /** The source process that has sent a message */
    const int src;

    /** The size of the message */
    const int size;

    /** The type of the message */
    const MPIMessageType message;

    /** @return True if the probe was successful and receive() is safe */
    bool isValid() const { return size >= 0; }
};

/**
 * Handle MPI communications between all Tide instances.
 */
class MPIChannel
{
public:
    /**
     * Create a new channel, initializing the MPI context.
     * This constructor should only be used once per program.
     * Use the alternative constructor to create additional channels which
     * share the primary MPIContext.
     * @param argc main program arguments count
     * @param argv main program arguments
     */
    MPIChannel(int argc, char* argv[]);

    /**
     * Create a new channel from splitting its parent channel.
     * @param parent The parent context to split, sharing the same MPIContext
     * @param color All processes with the same color belong to the same channel
     * @param key If provided, used to order the new ranks for the new channel
     */
    MPIChannel(const MPIChannel& parent, int color, int key);

    /** Destructor, closes the MPI channel. */
    ~MPIChannel();

    /** Get the rank of this process. */
    int getRank() const;

    /** Get the number of processes in this channel. */
    int getSize() const;

    /** Block execution until all participants have reached the barrier. */
    void globalBarrier() const;

    /**
     * Get the sum of the given local values across all processes.
     * @param localValue The value to sum
     * @return the sum of the localValues
     */
    int globalSum(int localValue) const;

    /**
     * Send data to a single process
     * @param type The type of data to send
     * @param serializedData The serialized data
     * @param dest The destination process
     */
    void send(MPIMessageType type, const std::string& serializedData, int dest);

    /**
     * Send a signal to all processes
     * @param type The type of signal
     */
    void sendAll(MPIMessageType type);

    /**
     * Send a brodcast message to all other processes
     * @param type The message type
     * @param serializedData The serialized data
     */
    void broadcast(MPIMessageType type, const std::string& serializedData);

    /** Nonblocking probe for messages from a given source */
    bool isMessageAvailable(int src);

    /**
     * Perform a blocking probe operation that returns if a message is pending
     * @param src The source process of where to probe on, default
     * MPI_ANY_SOURCE
     * @param tag The message tag of interest, default MPI_ANY_TAG
     * @return The probe result for a subsequent receive()
     */
    ProbeResult probe(int src = MPI_ANY_SOURCE, int tag = MPI_ANY_TAG);

    /**
     * Receive a header from a specific process.
     * This call is blocking.
     * @see isMessageAvailable()
     * @param src The source process
     * @return The header containing the message type and size
     */
    MPIHeader receiveHeader(int src);

    /**
     * Receive a message from a specific process.
     * This call is blocking.
     * @see receiveHeader()
     * @param dataBuffer The target data buffer
     * @param messageSize The number of bytes to receive
     * @param src The source process
     * @param tag The message tag/type, see probe()
     */
    void receive(char* dataBuffer, size_t messageSize, int src, int tag = 0);

    /**
     * Recieve a broadcast.
     * This call is blocking.
     * @see receiveHeader()
     * @param dataBuffer The target data buffer
     * @param messageSize The number of bytes to receive
     * @param src The source process
     */
    void receiveBroadcast(char* dataBuffer, size_t messageSize, int src);

    /**
     * Gather the values accross all the processes.
     * @param value The local value
     * @return A vector of values of size getSize(), ordered by process rank
     */
    std::vector<uint64_t> gatherAll(uint64_t value);

private:
    std::shared_ptr<MPIContext> _mpiContext;
    MPI_Comm _mpiComm;
    int _mpiRank;
    int _mpiSize;

    bool _isValid(const int dest) const;
    void _send(const MPIHeader& header, const int dest);
};

#endif

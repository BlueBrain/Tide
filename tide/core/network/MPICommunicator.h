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

#ifndef MPICOMMUNICATOR_H
#define MPICOMMUNICATOR_H

#include "NetworkBarrier.h"
#include "network/MessageHeader.h"
#include "types.h"

#include <mpi.h>

class MPIContext;

/**
 * The result of a probe operation on the network communicator.
 */
struct ProbeResult
{
    /** The source process that has sent a message */
    const int src;

    /** The size of the message */
    const int size;

    /** The type of the message */
    const MessageType messageType;

    /** @return True if the probe was successful and receive() is safe */
    bool isValid() const { return size >= 0; }
};

/**
 * Handle network communication between a set of MPI processes.
 */
class MPICommunicator : public NetworkBarrier
{
public:
    /**
     * Create a new communicator, initializing the MPI context.
     *
     * This constructor should only be used once per program.
     * Use the alternative constructor to create additional communicators which
     * share the primary MPIContext.
     *
     * @param argc main program arguments count
     * @param argv main program arguments
     */
    MPICommunicator(int argc, char* argv[]);

    /**
     * Create a communicator by splitting a parent one.
     *
     * The new ranks are ordered according to the ranks in the parent.
     *
     * @param parent The parent context to split, sharing the same MPIContext.
     * @param color All processes with the same color belong to the same group.
     */
    MPICommunicator(const MPICommunicator& parent, int color);

    /** Destructor, closes the MPI communicator. */
    ~MPICommunicator();

    /** Get the rank of this process in this group. */
    int getRank() const;

    /** Get the number of processes in this group. */
    int getSize() const;

    /** @name One-to-one communication. */
    //@{
    /**
     * Send data to a single process.
     * @param type The type of data to send
     * @param serializedData The serialized data
     * @param dest The destination process
     */
    void send(MessageType type, const std::string& serializedData, int dest);

    /**
     * Perform a blocking probe operation.
     * This allows receiving messages of any type and size from any source
     * atomically, without the need for transmitting a separate message header.
     * @see receive()
     * @param src The source process of where to probe on (default: any)
     * @param tag The message tag of interest (default: any)
     * @return The probe result for a subsequent receive()
     */
    ProbeResult probe(int src = MPI_ANY_SOURCE, int tag = MPI_ANY_TAG);

    /**
     * Receive a message from a specific process.
     * This call is blocking.
     * @see probe()
     * @param src The source process
     * @param dataBuffer The target data buffer
     * @param messageSize The number of bytes to receive
     * @param tag The message tag/type, see probe()
     */
    void receive(int src, char* dataBuffer, size_t messageSize, int tag);
    //@}

    /** @name Collective communication. */
    //@{
    /**
     * Broadcast a signal to all processes.
     * @see receiveBroadcastHeader()
     * @param type The type of signal
     */
    void broadcast(MessageType type);

    /**
     * Brodcast a message to all other processes.
     * @see receiveBroadcastHeader()
     * @param type The message type
     * @param data The serialized payload
     */
    void broadcast(MessageType type, const std::string& data);
    void broadcast(MessageType type, const QByteArray& data);

    /**
     * Receive a header broadcast by a specific process.
     * This call is blocking.
     * @see receiveBroadcast()
     * @param src The source process
     * @return The header containing the message type and size
     */
    MessageHeader receiveBroadcastHeader(int src);

    /**
     * Recieve a broadcast.
     * This call is blocking.
     * @see receiveBroadcastHeader()
     * @param src The source process
     * @param dataBuffer The target data buffer
     * @param messageSize The number of bytes to receive
     */
    void receiveBroadcast(int src, char* dataBuffer, size_t messageSize);
    //@}

    /** @name Collective operations. */
    //@{
    /** Block execution until all participants have reached the barrier. */
    void globalBarrier() const final;

    /**
     * Get the sum of the given local values across all processes.
     * @param localValue The value to sum
     * @return the sum of the localValues
     */
    int globalSum(int localValue) const;

    /**
     * Gather the values accross all the processes.
     * @param value The local value
     * @return A vector of values of size getSize(), ordered by process rank
     */
    std::vector<uint64_t> gatherAll(uint64_t value);
    //@}

private:
    std::shared_ptr<MPIContext> _mpiContext;
    MPI_Comm _mpiComm;
    int _mpiRank = -1;
    int _mpiSize = -1;

    void _broadcast(const MessageHeader& mh);
    void _broadcast(const char* data, const size_t size);
    bool _isValidAndNotSelf(const int dest) const;
};

#endif

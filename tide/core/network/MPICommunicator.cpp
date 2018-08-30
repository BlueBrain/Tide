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

#include "MPICommunicator.h"

#include "MPIContext.h"
#include "MPINospin.h"

#include "utils/log.h"

// #define instead of a function so that print_log prints the correct reference
#define MPI_CHECK(func)                                                 \
    {                                                                   \
        const int err = (func);                                         \
        if (err != MPI_SUCCESS)                                         \
            print_log(LOG_ERROR, LOG_MPI, "Error detected! (%d)", err); \
    }

MPICommunicator::MPICommunicator(int argc, char* argv[])
    : _mpiContext(new MPIContext(argc, argv))
    , _mpiComm(MPI_COMM_WORLD)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &_mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &_mpiSize);
}

MPICommunicator::MPICommunicator(const MPICommunicator& parent, const int color)
    : _mpiContext(parent._mpiContext)
    , _mpiComm(MPI_COMM_NULL)
{
    MPI_Comm_split(parent._mpiComm, color, parent.getRank(), &_mpiComm);
    MPI_Comm_rank(_mpiComm, &_mpiRank);
    MPI_Comm_size(_mpiComm, &_mpiSize);
}

MPICommunicator::~MPICommunicator()
{
    if (_mpiComm != MPI_COMM_WORLD)
        MPI_Comm_disconnect(&_mpiComm);
}

int MPICommunicator::getRank() const
{
    return _mpiRank;
}

int MPICommunicator::getSize() const
{
    return _mpiSize;
}

void MPICommunicator::globalBarrier() const
{
    MPI_Barrier(_mpiComm);
}

int MPICommunicator::globalSum(const int localValue) const
{
    int globalValue = 0;
    MPI_Allreduce((void*)&localValue, (void*)&globalValue, 1, MPI_INT, MPI_SUM,
                  _mpiComm);
    return globalValue;
}

std::vector<uint64_t> MPICommunicator::gatherAll(const uint64_t value)
{
    std::vector<uint64_t> results(_mpiSize);
    MPI_CHECK(MPI_Allgather((void*)&value, 1, MPI_LONG_LONG_INT,
                            (void*)results.data(), 1, MPI_LONG_LONG_INT,
                            _mpiComm));
    return results;
}

void MPICommunicator::send(const MessageType type,
                           const std::string& serializedData, const int dest)
{
    if (!_isValidAndNotSelf(dest))
        return;

    MPI_CHECK(MPI_Send_Nospin((void*)serializedData.data(),
                              serializedData.size(), MPI_BYTE, dest, int(type),
                              _mpiComm));
}

ProbeResult MPICommunicator::probe(const int src, const int tag)
{
    MPI_Status status;
    MPI_CHECK(MPI_Probe_Nospin(src, tag, _mpiComm, &status));

    int count = MPI_UNDEFINED;
    MPI_CHECK(MPI_Get_count(&status, MPI_BYTE, &count));

    return ProbeResult{status.MPI_SOURCE, count, MessageType(status.MPI_TAG)};
}

void MPICommunicator::receive(const int src, char* dataBuffer,
                              const size_t messageSize, const int tag)
{
    MPI_Status status;
    MPI_CHECK(MPI_Recv_Nospin((void*)dataBuffer, messageSize, MPI_BYTE, src,
                              tag, _mpiComm, &status));

    // Validate the number of bytes received
    int count = 0;
    MPI_CHECK(MPI_Get_count(&status, MPI_BYTE, &count));
    if (count != (int)messageSize)
        print_log(LOG_ERROR, LOG_MPI, "incorrect bytes count: %d / %d", count,
                  messageSize);
}

void MPICommunicator::broadcast(const MessageType type)
{
    _broadcast(MessageHeader{type, 0});
}

void MPICommunicator::broadcast(const MessageType type, const std::string& data)
{
    broadcast(type, QByteArray::fromRawData(data.data(), data.size()));
}

void MPICommunicator::broadcast(const MessageType type, const QByteArray& data)
{
    _broadcast(MessageHeader{type, (uint)data.size()});
    _broadcast(data.constData(), data.size());
}

MessageHeader MPICommunicator::receiveBroadcastHeader(const int src)
{
    // No-spin so that waiting for a message in a thread does not burn 100% CPU.
    // This does not reduce broadcast performance (tideBenchmarkMPI).
    MessageHeader mh;
    MPI_CHECK(MPI_Bcast_Nospin((void*)&mh, sizeof(MessageHeader), MPI_BYTE, src,
                               _mpiComm));
    return mh;
}

void MPICommunicator::receiveBroadcast(const int src, char* dataBuffer,
                                       const size_t messageSize)
{
    // Use regular MPI_Bcast for transfering the payload. The no-spin version
    // brings no benefits once the header has been received; but it degrades the
    // broadcast performance by an order of magnitude (tideBenchmarkMPI).
    MPI_CHECK(
        MPI_Bcast((void*)dataBuffer, messageSize, MPI_BYTE, src, _mpiComm));
}

void MPICommunicator::_broadcast(const MessageHeader& mh)
{
    MPI_CHECK(MPI_Bcast_Nospin((void*)&mh, sizeof(MessageHeader), MPI_BYTE,
                               _mpiRank, _mpiComm));
}

void MPICommunicator::_broadcast(const char* data, const size_t size)
{
    MPI_CHECK(
        MPI_Bcast(const_cast<char*>(data), size, MPI_BYTE, _mpiRank, _mpiComm));
}

bool MPICommunicator::_isValidAndNotSelf(const int dest) const
{
    return dest != _mpiRank && dest >= 0 && dest < _mpiSize;
}

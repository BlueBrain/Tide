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

#include "MPIChannel.h"

#include "MPIContext.h"
#include "MPINospin.h"

#include "log.h"

// #define instead of a function so that put_flog() prints the correct reference
#define MPI_CHECK(func)                                                   \
    {                                                                     \
        const int err = (func);                                           \
        if (err != MPI_SUCCESS)                                           \
            put_facility_flog(LOG_ERROR, LOG_MPI, "Error detected! (%d)", \
                              err);                                       \
    }

MPIChannel::MPIChannel(int argc, char* argv[])
    : _mpiContext(new MPIContext(argc, argv))
    , _mpiComm(MPI_COMM_WORLD)
    , _mpiRank(-1)
    , _mpiSize(-1)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &_mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &_mpiSize);
}

MPIChannel::MPIChannel(const MPIChannel& parent, const int color, const int key)
    : _mpiContext(parent._mpiContext)
    , _mpiComm(MPI_COMM_NULL)
    , _mpiRank(-1)
    , _mpiSize(-1)
{
    MPI_Comm_split(parent._mpiComm, color, key, &_mpiComm);
    MPI_Comm_rank(_mpiComm, &_mpiRank);
    MPI_Comm_size(_mpiComm, &_mpiSize);
}

MPIChannel::~MPIChannel()
{
    if (_mpiComm != MPI_COMM_WORLD)
        MPI_Comm_disconnect(&_mpiComm);
}

int MPIChannel::getRank() const
{
    return _mpiRank;
}

int MPIChannel::getSize() const
{
    return _mpiSize;
}

void MPIChannel::globalBarrier() const
{
    MPI_Barrier(_mpiComm);
}

int MPIChannel::globalSum(const int localValue) const
{
    int globalValue = 0;
    MPI_Allreduce((void*)&localValue, (void*)&globalValue, 1, MPI_INT, MPI_SUM,
                  _mpiComm);
    return globalValue;
}

bool MPIChannel::isMessageAvailable(const int src)
{
    int flag;
    MPI_Status status;
    MPI_Iprobe(src, 0, _mpiComm, &flag, &status);

    return (bool)flag;
}

void MPIChannel::send(const MPIMessageType type,
                      const std::string& serializedData, const int dest)
{
    if (!_isValid(dest))
        return;

    MPI_CHECK(MPI_Send_Nospin((void*)serializedData.data(),
                              serializedData.size(), MPI_BYTE, dest, int(type),
                              _mpiComm));
}

void MPIChannel::sendAll(const MPIMessageType type)
{
    MPIHeader mh;
    mh.size = 0;
    mh.type = type;

    for (int i = 0; i < _mpiSize; ++i)
        _send(mh, i);
}

void MPIChannel::broadcast(const MPIMessageType type,
                           const std::string& serializedData)
{
    MPIHeader mh;
    mh.size = serializedData.size();
    mh.type = type;

    for (int i = 0; i < _mpiSize; ++i)
        _send(mh, i);

    MPI_CHECK(MPI_Bcast((void*)serializedData.data(), serializedData.size(),
                        MPI_BYTE, _mpiRank, _mpiComm));
}

MPIHeader MPIChannel::receiveHeader(const int src)
{
    MPI_Status status;
    MPIHeader mh;
    MPI_CHECK(MPI_Recv_Nospin((void*)&mh, sizeof(MPIHeader), MPI_BYTE, src, 0,
                              _mpiComm, &status));
    return mh;
}

ProbeResult MPIChannel::probe(const int src, const int tag)
{
    MPI_Status status;
    MPI_CHECK(MPI_Probe_Nospin(src, tag, _mpiComm, &status));

    int count = MPI_UNDEFINED;
    MPI_CHECK(MPI_Get_count(&status, MPI_BYTE, &count));

    return ProbeResult{status.MPI_SOURCE, count,
                       MPIMessageType(status.MPI_TAG)};
}

void MPIChannel::receive(char* dataBuffer, const size_t messageSize,
                         const int src, const int tag)
{
    MPI_Status status;
    MPI_CHECK(MPI_Recv_Nospin((void*)dataBuffer, messageSize, MPI_BYTE, src,
                              tag, _mpiComm, &status));

    // Validate the number of bytes received
    int count = 0;
    MPI_CHECK(MPI_Get_count(&status, MPI_BYTE, &count));
    if (count != (int)messageSize)
        put_facility_flog(LOG_ERROR, LOG_MPI, "incorrect bytes count: %d / %d",
                          count, messageSize);
}

void MPIChannel::receiveBroadcast(char* dataBuffer, const size_t messageSize,
                                  const int src)
{
    MPI_CHECK(
        MPI_Bcast((void*)dataBuffer, messageSize, MPI_BYTE, src, _mpiComm));
}

std::vector<uint64_t> MPIChannel::gatherAll(const uint64_t value)
{
    std::vector<uint64_t> results(_mpiSize);
    MPI_CHECK(MPI_Allgather((void*)&value, 1, MPI_LONG_LONG_INT,
                            (void*)results.data(), 1, MPI_LONG_LONG_INT,
                            _mpiComm));
    return results;
}

bool MPIChannel::_isValid(const int dest) const
{
    return dest != _mpiRank && dest >= 0 && dest < _mpiSize;
}

void MPIChannel::_send(const MPIHeader& header, const int dest)
{
    if (!_isValid(dest))
        return;

    MPI_CHECK(MPI_Send_Nospin((void*)&header, sizeof(MPIHeader), MPI_BYTE, dest,
                              0, _mpiComm));
}

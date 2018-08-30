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

#include "WallToWallChannel.h"

#include "network/MPICommunicator.h"
#include "serialization/chrono.h"
#include "serialization/utils.h"
#include "utils/log.h"

#define RANK0 0

WallToWallChannel::WallToWallChannel(MPICommunicator& communicator)
    : _communicator{communicator}
{
}

int WallToWallChannel::getRank() const
{
    return _communicator.getRank();
}

int WallToWallChannel::globalSum(const int localValue) const
{
    return _communicator.globalSum(localValue);
}

bool WallToWallChannel::allReady(const bool isReady) const
{
    return _communicator.globalSum(isReady ? 1 : 0) == _communicator.getSize();
}

WallToWallChannel::clock::time_point WallToWallChannel::getTime() const
{
    return _timestamp;
}

void WallToWallChannel::synchronizeClock()
{
    if (_communicator.getRank() == RANK0)
        _sendClock();
    else
        _receiveClock();
}

bool WallToWallChannel::checkVersion(const uint64_t version) const
{
    const auto versions = _communicator.gatherAll(version);
    for (const auto& v : versions)
    {
        if (v != version)
            return false;
    }
    return true;
}

int WallToWallChannel::electLeader(const bool isCandidate)
{
    const int status = isCandidate ? (1 << getRank()) : 0;
    int globalStatus = globalSum(status);

    if (globalStatus <= 0)
        return -1;

    int leader = 0;
    while (globalStatus > 1)
    {
        globalStatus = globalStatus >> 1;
        ++leader;
    }
    return leader;
}

void WallToWallChannel::broadcast(const double timestamp)
{
    _communicator.broadcast(MessageType::TIMESTAMP,
                            serialization::toBinary(timestamp));
}

double WallToWallChannel::receiveTimestampBroadcast(const int src)
{
    const auto header = _communicator.receiveBroadcastHeader(src);
    assert(header.type == MessageType::TIMESTAMP);

    _buffer.setSize(header.size);
    _communicator.receiveBroadcast(src, _buffer.data(), _buffer.size());

    return serialization::get<double>(_buffer);
}

void WallToWallChannel::_sendClock()
{
    assert(_communicator.getRank() == RANK0);

    _timestamp = clock::now();

    _communicator.broadcast(MessageType::FRAME_CLOCK,
                            serialization::toBinary(_timestamp));
}

void WallToWallChannel::_receiveClock()
{
    assert(_communicator.getRank() != RANK0);

    const auto header = _communicator.receiveBroadcastHeader(RANK0);
    assert(header.type == MessageType::FRAME_CLOCK);

    _buffer.setSize(header.size);
    _communicator.receiveBroadcast(RANK0, _buffer.data(), _buffer.size());

    _timestamp = serialization::get<clock::time_point>(_buffer);
}

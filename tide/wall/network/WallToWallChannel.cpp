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

#include "WallToWallChannel.h"

#include "log.h"
#include "network/MPIChannel.h"
#include "serialization/utils.h"
#include "serialization/chrono.h"

#define RANK0 0

WallToWallChannel::WallToWallChannel( MPIChannelPtr mpiChannel )
    : _mpiChannel( mpiChannel )
{
}

int WallToWallChannel::getRank() const
{
    return _mpiChannel->getRank();
}

int WallToWallChannel::globalSum( const int localValue ) const
{
    return _mpiChannel->globalSum(localValue);
}

bool WallToWallChannel::allReady( const bool isReady ) const
{
    return _mpiChannel->globalSum( isReady ? 1 : 0 ) == _mpiChannel->getSize();
}

WallToWallChannel::clock::time_point WallToWallChannel::getTime() const
{
    return _timestamp;
}

void WallToWallChannel::synchronizeClock()
{
    if( _mpiChannel->getRank() == RANK0 )
        _sendClock();
    else
        _receiveClock();
}

void WallToWallChannel::globalBarrier() const
{
    _mpiChannel->globalBarrier();
}

bool WallToWallChannel::checkVersion( const uint64_t version ) const
{
    std::vector<uint64_t> versions = _mpiChannel->gatherAll( version );

    for( std::vector<uint64_t>::const_iterator it = versions.begin();
         it != versions.end(); ++it )
    {
        if( *it != version )
            return false;
    }
    return true;
}

int WallToWallChannel::electLeader( const bool isCandidate )
{
    const int status = isCandidate ? (1 << getRank()) : 0;
    int globalStatus = globalSum( status );

    if( globalStatus <= 0 )
        return -1;

    int leader = 0;
    while( globalStatus > 1 )
    {
        globalStatus = globalStatus >> 1;
        ++leader;
    }
    return leader;
}

void WallToWallChannel::broadcast( const double timestamp )
{
    _mpiChannel->broadcast( MPI_MESSAGE_TYPE_TIMESTAMP,
                            serialization::toBinary( timestamp ));
}

double WallToWallChannel::receiveTimestampBroadcast( const int src )
{
    MPIHeader header = _mpiChannel->receiveHeader( src );
    assert( header.type == MPI_MESSAGE_TYPE_TIMESTAMP );

    _buffer.setSize( header.size );
    _mpiChannel->receiveBroadcast( _buffer.data(), _buffer.size(), src );

    return serialization::get<double>( _buffer );
}

void WallToWallChannel::_sendClock()
{
    assert( _mpiChannel->getRank() == RANK0 );

    _timestamp = clock::now();

    _mpiChannel->broadcast( MPI_MESSAGE_TYPE_FRAME_CLOCK,
                            serialization::toBinary( _timestamp ));
}

void WallToWallChannel::_receiveClock()
{
    assert( _mpiChannel->getRank() != RANK0 );

    MPIHeader header = _mpiChannel->receiveHeader( RANK0 );
    assert( header.type == MPI_MESSAGE_TYPE_FRAME_CLOCK );

    _buffer.setSize( header.size );
    _mpiChannel->receiveBroadcast( _buffer.data(), _buffer.size(), RANK0 );

    _timestamp = serialization::get<clock::time_point>( _buffer );
}

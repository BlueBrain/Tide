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

#include "MasterToWallChannel.h"

#include "CountdownStatus.h"
#include "ScreenLock.h"
#include "network/MPIChannel.h"
#include "scene/Background.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "scene/Markers.h"
#include "scene/Options.h"
#include "serialization/utils.h"

#include <deflect/Frame.h>

MasterToWallChannel::MasterToWallChannel(MPIChannelPtr mpiChannel)
    : _mpiChannel(mpiChannel)
{
}

template <typename T>
void MasterToWallChannel::broadcast(const T& object, const MPIMessageType type)
{
    _mpiChannel->broadcast(type, serialization::toBinary(object));
}

template <typename T>
void MasterToWallChannel::broadcastAsync(const T& object,
                                         const MPIMessageType type)
{
    const auto data = serialization::toBinary(object);

    QMetaObject::invokeMethod(this, "_broadcast", Qt::QueuedConnection,
                              Q_ARG(MPIMessageType, type),
                              Q_ARG(std::string, data));
}

void MasterToWallChannel::sendAsync(BackgroundPtr background)
{
    broadcastAsync(background, MPIMessageType::BACKGROUND);
}

void MasterToWallChannel::sendAsync(DisplayGroupPtr displayGroup)
{
    broadcastAsync(displayGroup, MPIMessageType::DISPLAYGROUP);
}

void MasterToWallChannel::sendAsync(OptionsPtr options)
{
    broadcastAsync(options, MPIMessageType::OPTIONS);
}

void MasterToWallChannel::sendAsync(CountdownStatusPtr status)
{
    broadcastAsync(status, MPIMessageType::COUNTDOWN_STATUS);
}

void MasterToWallChannel::sendAsync(ScreenLockPtr lock)
{
    broadcastAsync(lock, MPIMessageType::LOCK);
}

void MasterToWallChannel::sendAsync(MarkersPtr markers)
{
    broadcastAsync(markers, MPIMessageType::MARKERS);
}

void MasterToWallChannel::send(deflect::FramePtr frame)
{
    assert(!frame->segments.empty() && "received an empty frame");
#if BOOST_VERSION >= 106000
    broadcast(frame, MPIMessageType::PIXELSTREAM);
#else
    // WAR missing support for std::shared_ptr
    broadcast(*frame, MPIMessageType::PIXELSTREAM);
#endif
}

void MasterToWallChannel::sendRequestScreenshot()
{
    _mpiChannel->sendAll(MPIMessageType::IMAGE);
}

void MasterToWallChannel::sendQuit()
{
    _mpiChannel->sendAll(MPIMessageType::QUIT);
}

// cppcheck-suppress passedByValue
void MasterToWallChannel::_broadcast(const MPIMessageType type,
                                     const std::string data)
{
    _mpiChannel->broadcast(type, data);
}

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

#include "WallFromMasterChannel.h"

#include "configuration/Configuration.h"
#include "network/MPIChannel.h"
#include "scene/CountdownStatus.h"
#include "scene/Markers.h"
#include "scene/Options.h"
#include "scene/Scene.h"
#include "scene/ScreenLock.h"
#include "scene/Window.h"
#include "serialization/utils.h"
#include "json/serialization.h"
#include "json/templates.h"

#include <deflect/server/Frame.h>

#include <QApplication>

namespace
{
const int RANK0 = 0;
}

WallFromMasterChannel::WallFromMasterChannel(MPIChannelPtr mpiChannel)
    : _mpiChannel{mpiChannel}
    , _processMessages{true}
{
}

Configuration WallFromMasterChannel::receiveConfiguration()
{
    const auto mh = _mpiChannel->receiveHeader(RANK0);
    if (mh.type != MPIMessageType::CONFIG)
        throw std::logic_error("Configuation object expected from master");
    return receiveJsonBroadcast<Configuration>(mh.size);
}

void WallFromMasterChannel::processMessages()
{
    while (_processMessages)
        receiveMessage();
}

void WallFromMasterChannel::receiveMessage()
{
    const auto mh = _mpiChannel->receiveHeader(RANK0);
    switch (mh.type)
    {
    case MPIMessageType::SCENE:
        emit received(receiveQObjectBroadcast<ScenePtr>(mh.size));
        break;
    case MPIMessageType::OPTIONS:
        emit received(receiveQObjectBroadcast<OptionsPtr>(mh.size));
        break;
    case MPIMessageType::LOCK:
        emit received(receiveQObjectBroadcast<ScreenLockPtr>(mh.size));
        break;
    case MPIMessageType::MARKERS:
        emit received(receiveQObjectBroadcast<MarkersPtr>(mh.size));
        break;
    case MPIMessageType::COUNTDOWN_STATUS:
        emit received(receiveQObjectBroadcast<CountdownStatusPtr>(mh.size));
        break;
    case MPIMessageType::PIXELSTREAM:
#if BOOST_VERSION >= 106000
        emit received(
            receiveBinaryBroadcast<deflect::server::FramePtr>(mh.size));
#else
        // WAR missing support for std::shared_ptr
        // The copy of the Frame object is not too expensive because its
        // Tiles' imageData are QByteArray (implicitly shared).
        emit received(std::make_shared<deflect::server::Frame>(
            receiveBinaryBroadcast<deflect::server::Frame>(mh.size)));
#endif
        break;
    case MPIMessageType::IMAGE:
        emit receivedScreenshotRequest();
        break;
    case MPIMessageType::QUIT:
        _processMessages = false;
        emit receivedQuit();
        break;
    case MPIMessageType::CONFIG:
        throw std::logic_error("Configuation object not expected at runtime");
        break;
    default:
        break;
    }
}

void WallFromMasterChannel::receiveBroadcast(const size_t messageSize)
{
    _buffer.setSize(messageSize);
    _mpiChannel->receiveBroadcast(_buffer.data(), messageSize, RANK0);
}

template <typename T>
T WallFromMasterChannel::receiveBinaryBroadcast(const size_t messageSize)
{
    receiveBroadcast(messageSize);
    return serialization::get<T>(_buffer);
}

template <typename T>
T WallFromMasterChannel::receiveJsonBroadcast(const size_t messageSize)
{
    receiveBroadcast(messageSize);
    const auto data = QByteArray::fromRawData(_buffer.data(), _buffer.size());
    return json::unpack<T>(data);
}

template <typename T>
T WallFromMasterChannel::receiveQObjectBroadcast(const size_t messageSize)
{
    auto qobject = receiveBinaryBroadcast<T>(messageSize);
    qobject->moveToThread(QApplication::instance()->thread());
    return qobject;
}

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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "MasterToWallChannel.h"

#include "network/MPICommunicator.h"
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

MasterToWallChannel::MasterToWallChannel(MPICommunicator& communicator)
    : _communicator{communicator}
{
}

template <typename T>
void MasterToWallChannel::broadcast(const T& object, const MessageType type)
{
    _communicator.broadcast(type, serialization::toBinary(object));
}

template <typename T>
void MasterToWallChannel::broadcastAsync(const T& object,
                                         const MessageType type)
{
    const auto data = serialization::toBinary(object);

    QMetaObject::invokeMethod(this, "_broadcast", Qt::QueuedConnection,
                              Q_ARG(MessageType, type),
                              Q_ARG(std::string, data));
}

void MasterToWallChannel::sendAsync(ScenePtr scene)
{
    broadcastAsync(scene, MessageType::SCENE);
}

void MasterToWallChannel::sendAsync(OptionsPtr options)
{
    broadcastAsync(options, MessageType::OPTIONS);
}

void MasterToWallChannel::sendAsync(CountdownStatusPtr status)
{
    broadcastAsync(status, MessageType::COUNTDOWN_STATUS);
}

void MasterToWallChannel::sendAsync(ScreenLockPtr lock)
{
    broadcastAsync(lock, MessageType::LOCK);
}

void MasterToWallChannel::sendAsync(MarkersPtr markers)
{
    broadcastAsync(markers, MessageType::MARKERS);
}

void MasterToWallChannel::sendFrame(deflect::server::FramePtr frame)
{
    assert(!frame->tiles.empty() && "received an empty frame");
#if BOOST_VERSION >= 106000
    broadcast(frame, MessageType::PIXELSTREAM);
#else
    // WAR missing support for std::shared_ptr
    broadcast(*frame, MessageType::PIXELSTREAM);
#endif
}

void MasterToWallChannel::send(const Configuration& config)
{
    _communicator.broadcast(MessageType::CONFIG, json::pack(config));
}

void MasterToWallChannel::sendRequestScreenshot()
{
    _communicator.broadcast(MessageType::IMAGE);
}

void MasterToWallChannel::sendQuit()
{
    _communicator.broadcast(MessageType::QUIT);
}

// cppcheck-suppress passedByValue
void MasterToWallChannel::_broadcast(const MessageType type,
                                     const std::string data)
{
    _communicator.broadcast(type, data);
}

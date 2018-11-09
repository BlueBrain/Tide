/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
/*                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>*/
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

#include "MasterFromWallChannel.h"

#include "network/MPICommunicator.h"
#include "serialization/utils.h"
#include "utils/log.h"

MasterFromWallChannel::MasterFromWallChannel(MPICommunicator& communicator)
    : _communicator{communicator}
{
    if (_communicator.getSize() < 2)
    {
        print_log(LOG_WARN, LOG_MPI, "Channel has no Wall receiver");
        _processMessages = false;
    }
}

void MasterFromWallChannel::processMessages()
{
    while (_processMessages)
    {
        const auto result = _communicator.probe();
        if (!result.isValid())
        {
            print_log(LOG_ERROR, LOG_MPI, "Invalid probe result size: %d",
                      result.size);
            continue;
        }

        _buffer.setSize(result.size);
        _communicator.receive(result.src, _buffer.data(), _buffer.size(),
                              int(result.messageType));

        switch (result.messageType)
        {
        case MessageType::REQUEST_FRAME:
        {
            emit receivedRequestFrame(serialization::get<QString>(_buffer));
            break;
        }
        case MessageType::IMAGE:
        {
            QImage image;
            QPoint index;
            serialization::fromBinary(_buffer, image, index);
            emit receivedScreenshot(image, index);
            break;
        }
        case MessageType::PIXELSTREAM_CLOSE:
            emit pixelStreamClose(serialization::get<QString>(_buffer));
            break;
        case MessageType::QUIT:
            _processMessages = false;
            break;
        default:
            print_log(LOG_WARN, LOG_MPI, "Invalid message type: %d",
                      result.messageType);
            break;
        }
    }
}

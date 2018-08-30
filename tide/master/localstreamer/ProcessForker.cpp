/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#include "ProcessForker.h"

#include "network/MPICommunicator.h"
#include "network/ReceiveBuffer.h"
#include "serialization/utils.h"
#include "utils/log.h"

#include <QProcess>

ProcessForker::ProcessForker(MPICommunicator& communicator)
    : _communicator{communicator}
{
}

void ProcessForker::run()
{
    ReceiveBuffer buffer;

    while (_processMessages)
    {
        const auto result = _communicator.probe();
        if (!result.isValid())
        {
            print_log(LOG_ERROR, LOG_MPI, "Invalid probe result size: %d",
                      result.size);
            continue;
        }

        buffer.setSize(result.size);
        _communicator.receive(result.src, buffer.data(), buffer.size(),
                              int(result.messageType));

        switch (result.messageType)
        {
        case MessageType::START_PROCESS:
        {
            const auto string = serialization::get<QString>(buffer);
            const auto args = string.split('#');
            if (args.length() != 3)
            {
                print_log(LOG_WARN, LOG_MPI, "Invalid command: '%d'",
                          string.toLocal8Bit().constData());
                break;
            }
            _launch(args[0], args[1], args[2].split(';'));
            break;
        }
        case MessageType::QUIT:
            _processMessages = false;
            break;
        default:
            print_log(LOG_WARN, LOG_MPI, "Invalid message type: '%d'",
                      result.messageType);
            break;
        }
    }
}

void ProcessForker::_launch(const QString& command, const QString& workingDir,
                            const QStringList& env)
{
    for (const auto& var : env)
    {
        // Know Qt bug: QProcess::setProcessEnvironment() does not work with
        // startDetached(). Calling qputenv() directly as a workaround.
        const auto kv = var.split("=");
        if (kv.length() == 2 &&
            !qputenv(kv[0].toLocal8Bit().constData(), kv[1].toLocal8Bit()))
        {
            print_log(LOG_ERROR, LOG_GENERAL, "Setting %s ENV variable failed.",
                      var.toLocal8Bit().constData());
        }
    }

    // forcefully disable touch point compression to ensure every touch point
    // update is received on the QML side (e.g. necessary for the whiteboard).
    // See undocumented QML_NO_TOUCH_COMPRESSION env variable in
    // <qt5-source>/qtdeclarative/src/quick/items/qquickwindow.cpp
    qputenv("QML_NO_TOUCH_COMPRESSION", "1");

    auto process = new QProcess();
    process->setWorkingDirectory(workingDir);
    process->startDetached(command);
}

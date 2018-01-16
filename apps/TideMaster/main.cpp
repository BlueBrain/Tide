/*********************************************************************/
/* Copyright (c) 2011-2012, The University of Texas at Austin.       */
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
/*                          Raphael.Dumusc@epfl.ch                   */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#include "log.h"

#include "CommandLineParameters.h"
#include "MasterApplication.h"
#include "network/MPIChannel.h"

#include <QThreadPool>

#include <memory>
#include <stdexcept>

int main(int argc, char* argv[])
{
    logger_id = "master";
    qInstallMessageHandler(qtMessageLogger);

    COMMAND_LINE_PARSER_CHECK(CommandLineParameters, "tideMaster");

    // Load virtualkeyboard input context plugin
    qputenv("QT_IM_MODULE", QByteArray("virtualkeyboard"));

    // For TuioTouch plugin in headless mode
    qputenv("QT_TUIOTOUCH_DELIVER_WITHOUT_FOCUS", QByteArray("1"));
    // WAR bug in TuioTouch plugin with http_proxy in Qt 5.8.0 [QTBUG-58706]
    if (QString(qVersion()) == "5.8.0")
        qunsetenv("http_proxy");

    {
        MPIChannelPtr worldChannel(new MPIChannel(argc, argv));
        if (worldChannel->getSize() < 2)
        {
            std::cerr << "MPI group size < 2 detected. Use tide script or check"
                         " MPI configuration."
                      << std::endl;
            return EXIT_FAILURE;
        }

        const int rank = worldChannel->getRank();
        MPIChannelPtr localChannel(new MPIChannel(*worldChannel, 0, rank));
        MPIChannelPtr mainChannel(new MPIChannel(*worldChannel, 1, rank));

        std::unique_ptr<MasterApplication> app;
        try
        {
            const auto config = commandLine.getConfigFilename();
            app.reset(new MasterApplication(argc, argv, config, mainChannel,
                                            localChannel));
        }
        catch (const std::exception& e)
        {
            print_log(LOG_FATAL, LOG_GENERAL,
                      "Could not initialize application. %s", e.what());

            // Avoid MPI deadlock, tell the other applications to quit
            // (normally done by MasterApplication destructor).
            localChannel->send(MPIMessageType::QUIT, "", 1);
            mainChannel->sendAll(MPIMessageType::QUIT);

            return EXIT_FAILURE;
        }

        const auto& session = commandLine.getSessionFilename();
        if (!session.isEmpty())
            app->load(session);

        app->exec(); // enter Qt event loop

        print_log(LOG_DEBUG, LOG_GENERAL, "waiting for threads to finish...");
        QThreadPool::globalInstance()->waitForDone();
    }
    print_log(LOG_DEBUG, LOG_GENERAL, "done.");
    return EXIT_SUCCESS;
}

/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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

#include "WallApplication.h"
#include "WallConfiguration.h"
#include "network/MPIChannel.h"
#include "utils/CommandLineParser.h"
#include "utils/log.h"

#include <QThreadPool>

#include <memory>
#include <stdexcept>

namespace
{
class CommandLineParameters : public CommandLineParser
{
public:
    void showSyntax(const std::string& appName) const final
    {
        std::cout << "Usage: mpiexec " << appName << "\n\n";
        std::cout << desc << std::endl;
        std::cout << "Do not launch manually, use 'tide' script instead.\n";
    }
};
}

int main(int argc, char* argv[])
{
    COMMAND_LINE_PARSER_CHECK(CommandLineParameters, "tideWall");

    // Load virtualkeyboard input context plugin
    qputenv("QT_IM_MODULE", QByteArray("virtualkeyboard"));

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
        logger_id = QString("wall%1").arg(rank).toStdString();
        qInstallMessageHandler(qtMessageLogger);

        MPIChannelPtr localChannel(new MPIChannel(*worldChannel, 1, rank));
        MPIChannelPtr mainChannel(new MPIChannel(*worldChannel, 1, rank));

        std::unique_ptr<WallApplication> app;
        try
        {
            app.reset(
                new WallApplication(argc, argv, mainChannel, localChannel));
        }
        catch (const std::exception& e)
        {
            print_log(LOG_FATAL, LOG_GENERAL,
                      "Could not initialize application. %s", e.what());

            // Always send QUIT to the master application that will wait on it
            // to exit (normally done by WallApplication destructor).
            if (localChannel->getRank() == 0)
                mainChannel->send(MPIMessageType::QUIT, "", 0);

            return EXIT_FAILURE;
        }
        app->exec(); // enter Qt event loop

        print_log(LOG_DEBUG, LOG_GENERAL, "waiting for threads to finish...");
        QThreadPool::globalInstance()->waitForDone();
    }
    print_log(LOG_DEBUG, LOG_GENERAL, "done.");
    return EXIT_SUCCESS;
}

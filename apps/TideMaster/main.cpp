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

#include "utils/log.h"

#include "CommandLineParameters.h"
#include "MasterApplication.h"
#include "network/MPICommunicator.h"

#include <QThreadPool>

#include <memory>
#include <stdexcept>

namespace
{
void setupEnvVariables()
{
    // Load virtualkeyboard input context plugin
    qputenv("QT_IM_MODULE", QByteArray("virtualkeyboard"));

    if (qgetenv("QT_QPA_GENERIC_PLUGINS").contains("TuioTouch"))
    {
        // For TuioTouch plugin in headless mode
        qputenv("QT_TUIOTOUCH_DELIVER_WITHOUT_FOCUS", QByteArray("1"));

        // WAR bug in TuioTouch plugin with http_proxy in Qt 5.8.0 [QTBUG-58706]
        if (QString(qVersion()) == "5.8.0")
            qunsetenv("http_proxy");
    }
}
}

int main(int argc, char* argv[])
{
    logger_id = "master";
    qInstallMessageHandler(qtMessageLogger);

    COMMAND_LINE_PARSER_CHECK(CommandLineParameters, "tideMaster");

    setupEnvVariables();

    {
        auto worldComm = MPICommunicator{argc, argv};
        if (worldComm.getSize() < 2)
        {
            std::cerr << "MPI group size < 2 detected. Use tide script or check"
                         " MPI parameters."
                      << std::endl;
            return EXIT_FAILURE;
        }
        // Init communicators: all MPI processes must follow the same steps
        auto masterForkerComm = MPICommunicator{worldComm, 0};
        auto wallSwapSyncComm = MPICommunicator{worldComm, 0};
        auto masterWallComm = MPICommunicator{worldComm, 1};
        auto wallMasterComm = MPICommunicator{worldComm, 1};

        Q_UNUSED(wallSwapSyncComm);

        try
        {
            const auto config = commandLine.getConfigFilename();
            MasterApplication app(argc, argv, config, masterWallComm,
                                  wallMasterComm, masterForkerComm);

            const auto& session = commandLine.getSessionFilename();
            if (!session.isEmpty())
                app.load(session);

            app.exec(); // enter Qt event loop

            print_log(LOG_DEBUG, LOG_GENERAL,
                      "waiting for threads to finish...");
            QThreadPool::globalInstance()->waitForDone();
        }
        catch (const std::exception& e)
        {
            print_log(LOG_FATAL, LOG_GENERAL,
                      "Could not initialize application. %s", e.what());

            // Avoid MPI deadlock, tell the other applications to quit
            // (normally done by MasterApplication destructor).
            masterForkerComm.send(MessageType::QUIT, "", 1);
            masterWallComm.broadcast(MessageType::QUIT);

            return EXIT_FAILURE;
        }
    } // close MPI connections
    print_log(LOG_DEBUG, LOG_GENERAL, "done.");
    return EXIT_SUCCESS;
}

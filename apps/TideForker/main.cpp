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

#include "tide/core/network/MPICommunicator.h"
#include "tide/core/utils/CommandLineParser.h"
#include "tide/core/utils/log.h"
#include "tide/master/localstreamer/ProcessForker.h"

class CommandLineHelper : public CommandLineParser
{
public:
    void showSyntax(const std::string& appName) const final
    {
        std::cout << "Usage: " << appName << " is an internal subprocess of "
                                             "tideMaster, do not run manually."
                  << std::endl;
    }
};

int main(int argc, char* argv[])
{
    logger_id = "forker";
    qInstallMessageHandler(qtMessageLogger);

    COMMAND_LINE_PARSER_CHECK(CommandLineHelper, "tideForker");

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
        auto masterWallComm = MPICommunicator{worldComm, 0};
        auto wallMasterComm = MPICommunicator{worldComm, 0};

        Q_UNUSED(wallSwapSyncComm);
        Q_UNUSED(masterWallComm);
        Q_UNUSED(wallMasterComm);

        ProcessForker{masterForkerComm}.run();
    } // close MPI connections
    print_log(LOG_DEBUG, LOG_GENERAL, "done.");
    return EXIT_SUCCESS;
}

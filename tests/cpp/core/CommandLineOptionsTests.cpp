/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE CommandLineOptions
#include <boost/test/unit_test.hpp>

#include "localstreamer/CommandLineOptions.h"
#include "types.h"

#include <QStringList>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE(testCommandLineDefaults)
{
    CommandLineOptions options;

    BOOST_CHECK(!options.getHelp());
    BOOST_CHECK_EQUAL(options.streamId, "");
    BOOST_CHECK_EQUAL(options.width, 0);
    BOOST_CHECK_EQUAL(options.height, 0);
    BOOST_CHECK_EQUAL(options.url, "");
    BOOST_CHECK_EQUAL(options.contentsDir, "");
    BOOST_CHECK_EQUAL(options.sessionsDir, "");
    BOOST_CHECK_EQUAL(options.webservicePort, 0);
    BOOST_CHECK_EQUAL(options.demoServiceUrl, "");
    BOOST_CHECK_EQUAL(options.showPowerButton, false);
    BOOST_CHECK_EQUAL(options.saveDir, "");

    BOOST_CHECK_EQUAL(options.getCommandLine().toStdString(), "");
}

void setOptionParameters(CommandLineOptions& options)
{
    options.streamId = "MyStreamer";
    options.width = 480;
    options.height = 640;
    options.url = "http://www.perdu.com";
    options.contentsDir = "/var/contents";
    options.sessionsDir = "/var/sessions";
    options.webservicePort = 8888;
    options.demoServiceUrl = "http://demoservice.bbp.epfl.ch";
    options.showPowerButton = true;
    options.saveDir = "/tmp/whiteboards";
}

void checkOptionParameters(const CommandLineOptions& options)
{
    BOOST_CHECK(!options.getHelp());
    BOOST_CHECK_EQUAL(options.streamId, "MyStreamer");
    BOOST_CHECK_EQUAL(options.width, 480);
    BOOST_CHECK_EQUAL(options.height, 640);
    BOOST_CHECK_EQUAL(options.url, "http://www.perdu.com");
    BOOST_CHECK_EQUAL(options.contentsDir, "/var/contents");
    BOOST_CHECK_EQUAL(options.sessionsDir, "/var/sessions");
    BOOST_CHECK_EQUAL(options.webservicePort, 8888);
    BOOST_CHECK_EQUAL(options.demoServiceUrl, "http://demoservice.bbp.epfl.ch");
    BOOST_CHECK_EQUAL(options.showPowerButton, true);
    BOOST_CHECK_EQUAL(options.saveDir, "/tmp/whiteboards");

    BOOST_CHECK_EQUAL(options.getCommandLine().toStdString(),
                      "--streamid MyStreamer "
                      "--width 480 "
                      "--height 640 "
                      "--url http://www.perdu.com "
                      "--contentsDir /var/contents "
                      "--sessionsDir /var/sessions "
                      "--webservicePort 8888 "
                      "--demoServiceUrl http://demoservice.bbp.epfl.ch "
                      "--showPowerButton "
                      "--saveDir /tmp/whiteboards");
}

BOOST_AUTO_TEST_CASE(testCommandLineManualCreation)
{
    CommandLineOptions options;
    setOptionParameters(options);

    checkOptionParameters(options);
}

BOOST_AUTO_TEST_CASE(testCommandLineFromCommandLineArguments)
{
    CommandLineOptions options;
    setOptionParameters(options);

    // Create a c-style representation of the command line options
    QStringList arguments = options.getCommandLineArguments();
    int argc = arguments.size() + 1;
    std::vector<char*> argv(argc);
    std::vector<char>* argList = new std::vector<char>[argc];

    // Program name
    {
        const std::string tmp("/test/program");
        argList[0].assign(tmp.begin(), tmp.end());
        argList[0].push_back('\0');
        argv[0] = argList[0].data();
    }
    // Command line arguments
    for (int i = 1; i < argc; ++i)
    {
        const std::string tmp = arguments.at(i - 1).toStdString();
        argList[i].assign(tmp.begin(), tmp.end());
        argList[i].push_back('\0');
        argv[i] = argList[i].data();
    }

    CommandLineOptions optionsDeserialized(argc, argv.data());
    checkOptionParameters(optionsDeserialized);
    delete[] argList;
}

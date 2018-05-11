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

#include "CommandLineOptions.h"

#include <QStringList>
#include <iostream>

namespace po = boost::program_options;

CommandLineOptions::CommandLineOptions()
{
    _fillDesc();
}

CommandLineOptions::CommandLineOptions(const int argc, char** argv)
{
    _fillDesc();
    parse(argc, argv);
}

void CommandLineOptions::parse(const int argc, char** argv)
{
    CommandLineParser::parse(argc, argv);
    if (getHelp())
        return;

    streamId = vm["streamid"].as<std::string>().c_str();
    width = vm["width"].as<unsigned int>();
    height = vm["height"].as<unsigned int>();
    url = vm["url"].as<std::string>().c_str();
    contentsDir = vm["contentsDir"].as<std::string>().c_str();
    sessionsDir = vm["sessionsDir"].as<std::string>().c_str();
    webservicePort = vm["webservicePort"].as<uint16_t>();
    demoServiceUrl = vm["demoServiceUrl"].as<std::string>().c_str();
    demoServiceImageDir = vm["demoServiceImageDir"].as<std::string>().c_str();
    showPowerButton = vm["showPowerButton"].as<bool>();
    saveDir = vm["saveDir"].as<std::string>().c_str();
}

void CommandLineOptions::_fillDesc()
{
    // clang-format off
    desc.add_options()
        ("streamid", po::value<std::string>()->required(),
         "unique identifier for this stream")
        ("width", po::value<unsigned int>()->default_value(0),
         "width of the stream in pixels")
        ("height", po::value<unsigned int>()->default_value(0),
         "height of the stream in pixels")
        ("url", po::value<std::string>()->default_value(""),
         "webbrowser only: url to open")
        ("config", po::value<std::string>()->default_value(""),
         "launcher & whiteboard: Tide configuation file for extra settings")
        ("contentsDir", po::value<std::string>()->default_value(""),
         "launcher: directory where the contents are located")
        ("sessionsDir", po::value<std::string>()->default_value(""),
         "launcher: directory where the session files are located")
        ("webservicePort", po::value<uint16_t>()->default_value(0),
         "launcher: port where the master application's webservice is running")
        ("demoServiceUrl", po::value<std::string>()->default_value(""),
         "launcher: url for the demo service")
        ("demoServiceImageDir", po::value<std::string>()->default_value(""),
         "launcher: folder for the images for the demo service")
        ("showPowerButton", po::bool_switch()->default_value(false),
         "launcher: show the power button")
        ("saveDir", po::value<std::string>()->default_value(""),
         "whiteboard: directory to use for saving images")
    ;
    // clang-format on
}

void CommandLineOptions::showSyntax(const std::string& appName) const
{
    CommandLineParser::showSyntax(appName);
    std::cout << std::endl;
    std::cout << "Normally launched as a subprocess by tideMaster, not "
                 "intended to be run manually."
              << std::endl;
}

QString CommandLineOptions::getCommandLine() const
{
    return getCommandLineArguments().join(' ');
}

QStringList CommandLineOptions::getCommandLineArguments() const
{
    QStringList arguments;

    if (!streamId.isEmpty())
        arguments << "--streamid" << streamId;

    if (width > 0)
        arguments << "--width" << QString::number(width);

    if (height > 0)
        arguments << "--height" << QString::number(height);

    if (!url.isEmpty())
        arguments << "--url" << url;

    if (!contentsDir.isEmpty())
        arguments << "--contentsDir" << contentsDir;

    if (!sessionsDir.isEmpty())
        arguments << "--sessionsDir" << sessionsDir;

    if (webservicePort > 0)
        arguments << "--webservicePort" << QString::number((int)webservicePort);

    if (!demoServiceUrl.isEmpty())
        arguments << "--demoServiceUrl" << demoServiceUrl;

    if (!demoServiceImageDir.isEmpty())
        arguments << "--demoServiceImageDir" << demoServiceImageDir;

    if (showPowerButton)
        arguments << "--showPowerButton";

    if (!saveDir.isEmpty())
        arguments << "--saveDir" << saveDir;

    return arguments;
}

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

    _streamId = vm["streamid"].as<std::string>().c_str();
    _url = vm["url"].as<std::string>().c_str();
    _width = vm["width"].as<unsigned int>();
    _height = vm["height"].as<unsigned int>();
    _configuration = vm["config"].as<std::string>().c_str();
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

    if (!_streamId.isEmpty())
        arguments << "--streamid" << _streamId;

    if (_width > 0)
        arguments << "--width" << QString::number(_width);

    if (_height > 0)
        arguments << "--height" << QString::number(_height);

    if (!_url.isEmpty())
        arguments << "--url" << _url;

    if (!_configuration.isEmpty())
        arguments << "--config" << _configuration;

    return arguments;
}

const QString& CommandLineOptions::getUrl() const
{
    return _url;
}

const QString& CommandLineOptions::getStreamId() const
{
    return _streamId;
}

unsigned int CommandLineOptions::getWidth() const
{
    return _width;
}

unsigned int CommandLineOptions::getHeight() const
{
    return _height;
}

const QString& CommandLineOptions::getConfiguration() const
{
    return _configuration;
}

void CommandLineOptions::setUrl(const QString& url)
{
    _url = url;
}

void CommandLineOptions::setStreamId(const QString& id)
{
    _streamId = id;
}

void CommandLineOptions::setConfiguration(const QString& file)
{
    _configuration = file;
}

void CommandLineOptions::setWidth(const unsigned int width)
{
    _width = width;
}

void CommandLineOptions::setHeight(const unsigned int height)
{
    _height = height;
}

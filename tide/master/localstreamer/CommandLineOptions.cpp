/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#include <iostream>
#include <boost/program_options.hpp>
#include <QStringList>

CommandLineOptions::CommandLineOptions()
    : getHelp_(false)
    , streamerType_(PS_UNKNOWN)
    , width_(0)
    , height_(0)
    , desc_("Allowed options")
{
    initDesc();
}

CommandLineOptions::CommandLineOptions(int &argc, char **argv)
    : getHelp_(false)
    , streamerType_(PS_UNKNOWN)
    , width_(0)
    , height_(0)
    , desc_("Allowed options")
{
    initDesc();
    parseCommandLineArguments(argc, argv);
}

void CommandLineOptions::initDesc()
{
    desc_.add_options()
        ("help", "produce help message")
        ("name", boost::program_options::value<std::string>()->default_value(""),
                 "unique identifier for this stream")
        ("type", boost::program_options::value<std::string>()->default_value(""),
                 "streamer type [webkit | dock]")
        ("width", boost::program_options::value<unsigned int>()->default_value(0),
                 "width of the stream in pixel")
        ("height", boost::program_options::value<unsigned int>()->default_value(0),
                 "height of the stream in pixel")
        ("url", boost::program_options::value<std::string>()->default_value(""), "webkit only: url")
        ("rootdir", boost::program_options::value<std::string>()->default_value(""), "dock only: root directory")
    ;
}

bool CommandLineOptions::getHelp() const
{
    return getHelp_;
}

PixelStreamerType CommandLineOptions::getPixelStreamerType() const
{
    return streamerType_;
}

const QString& CommandLineOptions::getUrl() const
{
    return url_;
}

const QString& CommandLineOptions::getRootDir() const
{
    return rootDir_;
}

const QString& CommandLineOptions::getName() const
{
    return name_;
}

unsigned int CommandLineOptions::getWidth() const
{
    return width_;
}

unsigned int CommandLineOptions::getHeight() const
{
    return height_;
}

void CommandLineOptions::setHelp(const bool set)
{
    getHelp_ = set;
}

void CommandLineOptions::setPixelStreamerType(const PixelStreamerType type)
{
    streamerType_ = type;
}

void CommandLineOptions::setUrl(const QString& url)
{
    url_ = url;
}

void CommandLineOptions::setRootDir(const QString& dir)
{
    rootDir_ = dir;
}

void CommandLineOptions::setName(const QString& name)
{
    name_ = name;
}

void CommandLineOptions::setWidth(const unsigned int width)
{
    width_ = width;
}

void CommandLineOptions::setHeight(const unsigned int height)
{
    height_ = height;
}

QString CommandLineOptions::getCommandLine() const
{
    return getCommandLineArguments().join(" ");
}

QStringList CommandLineOptions::getCommandLineArguments() const
{
    QStringList arguments;

    if (streamerType_ != PS_UNKNOWN)
        arguments << "--type" << getStreamerTypeString(streamerType_);

    if (width_ > 0)
        arguments << "--width" << QString::number(width_);

    if (height_ > 0)
        arguments << "--height" << QString::number(height_);

    if (getHelp_)
        arguments << "--help";

    if (!name_.isEmpty())
        arguments << "--name" << name_;

    if (!url_.isEmpty())
        arguments << "--url" << url_;

    if (!rootDir_.isEmpty())
        arguments << "--rootdir" << rootDir_;

    return arguments;
}

void CommandLineOptions::parseCommandLineArguments(int &argc, char **argv)
{
    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc_), vm);
        boost::program_options::notify(vm);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    getHelp_ = vm.count("help");
    name_ = vm["name"].as<std::string>().c_str();
    streamerType_ = getStreamerType(vm["type"].as<std::string>().c_str());
    url_ = vm["url"].as<std::string>().c_str();
    width_ = vm["width"].as<unsigned int>();
    height_ = vm["height"].as<unsigned int>();
    rootDir_ = vm["rootdir"].as<std::string>().c_str();
}

void CommandLineOptions::showSyntax() const
{
    std::cout << desc_;
}


/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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
#include <QStringList>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

CommandLineOptions::CommandLineOptions()
{
    _initDesc();
}

CommandLineOptions::CommandLineOptions( int& argc, char** argv )
{
    _initDesc();
    _parseCommandLineArguments( argc, argv );
}

void CommandLineOptions::_initDesc()
{
    _desc.add_options()
        ("help", "produce help message")
        ("streamname", po::value<std::string>()->default_value( "" ),
         "unique identifier for this stream")
        ("type", po::value<std::string>()->default_value( "" ),
         "streamer type [webkit]")
        ("width", po::value<unsigned int>()->default_value( 0 ),
         "width of the stream in pixel")
        ("height", po::value<unsigned int>()->default_value( 0 ),
         "height of the stream in pixel")
        ("url", po::value<std::string>()->default_value( "" ),
         "webkit only: url")
        ("config", po::value<std::string>()->default_value( "" ),
         "Launcher only: Tide xml configuation file")
    ;
}

bool CommandLineOptions::getHelp() const
{
    return _getHelp;
}

PixelStreamerType CommandLineOptions::getPixelStreamerType() const
{
    return _streamerType;
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

void CommandLineOptions::setHelp( const bool set )
{
    _getHelp = set;
}

void CommandLineOptions::setPixelStreamerType( const PixelStreamerType type )
{
    _streamerType = type;
}

void CommandLineOptions::setUrl( const QString& url )
{
    _url = url;
}

void CommandLineOptions::setStreamId( const QString& name )
{
    _streamId = name;
}

void CommandLineOptions::setConfiguration( const QString& file )
{
    _configuration = file;
}

void CommandLineOptions::setWidth( const unsigned int width )
{
    _width = width;
}

void CommandLineOptions::setHeight( const unsigned int height )
{
    _height = height;
}

QString CommandLineOptions::getCommandLine() const
{
    return getCommandLineArguments().join( ' ' );
}

QStringList CommandLineOptions::getCommandLineArguments() const
{
    QStringList arguments;

    if( _streamerType != PS_UNKNOWN )
        arguments << "--type" << getStreamerTypeString( _streamerType );

    if( _width > 0 )
        arguments << "--width" << QString::number( _width );

    if( _height > 0 )
        arguments << "--height" << QString::number( _height );

    if( _getHelp )
        arguments << "--help";

    if( !_streamId.isEmpty( ))
        arguments << "--streamname" << _streamId;

    if( !_url.isEmpty( ))
        arguments << "--url" << _url;

    if( !_configuration.isEmpty( ))
        arguments << "--config" << _configuration;

    return arguments;
}

void CommandLineOptions::_parseCommandLineArguments( int& argc, char** argv )
{
    po::variables_map vm;
    try
    {
        po::store( po::parse_command_line( argc, argv, _desc ), vm );
        po::notify( vm );
    }
    catch( const std::exception& e )
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    _getHelp = vm.count( "help" );
    _streamId = vm["streamname"].as<std::string>().c_str();
    _streamerType = getStreamerType(vm["type"].as<std::string>().c_str());
    _url = vm["url"].as<std::string>().c_str();
    _width = vm["width"].as<unsigned int>();
    _height = vm["height"].as<unsigned int>();
    _configuration = vm["config"].as<std::string>().c_str();
}

void CommandLineOptions::showSyntax() const
{
    std::cout << _desc;
}

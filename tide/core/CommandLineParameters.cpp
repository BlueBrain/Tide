/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "CommandLineParameters.h"

#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

CommandLineParameters::CommandLineParameters( int& argc, char** argv )
    : _desc("Allowed options")
    , _getHelp(false)
{
    _initDesc();
    _parseCommandLineArguments(argc, argv);
}


bool CommandLineParameters::getHelp() const
{
    return _getHelp;
}

void CommandLineParameters::showSyntax() const
{
    std::cout << _desc;
}

void CommandLineParameters::_initDesc()
{
    _desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>()->default_value(""),
                 "path to configuration file")
        ("sessionfile", po::value<std::string>()->default_value(""),
                 "path to an initial session file")
    ;
}

void CommandLineParameters::_parseCommandLineArguments( int& argc, char** argv )
{
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, _desc), vm);
        po::notify(vm);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    _getHelp = vm.count("help");
    _configFilename = vm["config"].as<std::string>().c_str();
    _sessionFilename = vm["sessionfile"].as<std::string>().c_str();
}

const QString& CommandLineParameters::getConfigFilename() const
{
    return _configFilename;
}

const QString& CommandLineParameters::getSessionFilename() const
{
    return _sessionFilename;
}

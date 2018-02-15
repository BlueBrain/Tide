/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include "log.h"
#include <boost/program_options.hpp>

/**
 * Standard macro to check for --help and syntax error of command line args.
 */
#define COMMAND_LINE_PARSER_CHECK(ParserClass, AppName)                     \
    ParserClass commandLine;                                                \
    try                                                                     \
    {                                                                       \
        commandLine.parse(argc, argv);                                      \
    }                                                                       \
    catch (const boost::program_options::error& e)                          \
    {                                                                       \
        print_log(LOG_FATAL, LOG_GENERAL, "failed to start: %s", e.what()); \
        return EXIT_FAILURE;                                                \
    }                                                                       \
    if (commandLine.getHelp())                                              \
    {                                                                       \
        commandLine.showSyntax(AppName);                                    \
        return EXIT_SUCCESS;                                                \
    }

/**
 * Basic command line arguments parser with [-h;--help] handling.
 */
class CommandLineParser
{
public:
    /** Constructor. */
    CommandLineParser();

    /** Virtual destructor. */
    virtual ~CommandLineParser();

    /**
     * Try to parse the command line arguments.
     *
     * @param argc number of command line arguments.
     * @param argv array of command line arguments.
     *
     * @throw boost::program_options::error on parsing error.
     */
    virtual void parse(int argc, char** argv);

    /** Was the --help flag given. */
    bool getHelp() const;

    /**
     * Print syntax to std::out.
     * @param appName The name of the executable to print in the output.
     */
    virtual void showSyntax(const std::string& appName) const;

protected:
    /** The list of options which already includes "--help". */
    boost::program_options::options_description desc{"Allowed options"};

    /** The list of positional options. */
    boost::program_options::positional_options_description pos_desc;

    /** Contains the results of the parse() operation. */
    boost::program_options::variables_map vm;
};

#endif

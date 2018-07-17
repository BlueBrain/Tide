/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
/*                     Raphael.Dumusc@epfl.ch                        */
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

#include "tide/core/configuration/Configuration.h"
#include "tide/core/configuration/ConfigurationWriter.h"
#include "tide/core/utils/CommandLineParser.h"

#include <QCoreApplication>

#include <iostream>

class ConverterOptions : public CommandLineParser
{
public:
    ConverterOptions()
    {
        using boost::program_options::value;
        using boost::program_options::bool_switch;

        pos_desc.add("infile", 1);
        pos_desc.add("outfile", 1);
        desc.add_options()("infile", value<std::string>()->required(),
                           "Input Tide configuration file (.xml|.json)");
        desc.add_options()("outfile", value<std::string>()->required(),
                           "Output Tide configuration file (.json)");
        desc.add_options()("full,f", bool_switch()->default_value(false),
                           "Export all values (don't omit default ones).");
    }
    std::string input() const { return vm["infile"].as<std::string>(); }
    std::string output() const { return vm["outfile"].as<std::string>(); }
    bool full() const { return vm["full"].as<bool>(); }
};

int main(int argc, char* argv[])
{
    COMMAND_LINE_PARSER_CHECK(ConverterOptions, "tideConverter");

    QCoreApplication app{argc, argv};

    try
    {
        const auto config = Configuration{commandLine.input().c_str()};
        const auto writer = ConfigurationWriter{config};
        using Format = ConfigurationWriter::Format;
        const auto format = commandLine.full() ? Format::full : Format::minimal;
        writer.write(commandLine.output().c_str(), format);
        return EXIT_SUCCESS;
    }
    catch (const std::runtime_error&)
    {
        std::cerr << "conversion failed" << std::endl;
    }
    return EXIT_FAILURE;
}

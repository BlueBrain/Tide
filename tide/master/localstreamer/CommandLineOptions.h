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

#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <QString>
#include <boost/program_options/options_description.hpp>

#include "PixelStreamerType.h"

/**
 * Command line options to pass startup parameters to a localstreamer application.
 */
class CommandLineOptions
{
public:
    /** Construct an empty instance */
    CommandLineOptions();

    /** Construct from command line parameters */
    CommandLineOptions(int &argc, char **argv);

    /** Print syntax to std::out */
    void showSyntax() const;

    /** @name Getters */
    //@{
    bool getHelp() const;
    PixelStreamerType getPixelStreamerType() const;
    const QString& getUrl() const;
    const QString& getRootDir() const;
    const QString& getName() const;
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    //@}

    /** Get the command line arguments corresponding to this object */
    QStringList getCommandLineArguments() const;

    /** Get the command line arguments joined in a single line (convenience function) */
    QString getCommandLine() const;

    /** @name Setters */
    //@{
    void setHelp(const bool set);
    void setPixelStreamerType(const PixelStreamerType type);
    void setUrl(const QString& url);
    void setRootDir(const QString& dir);
    void setName(const QString& name);
    void setWidth(const unsigned int width);
    void setHeight(const unsigned int height);
    //@}

private:
    void initDesc();
    void parseCommandLineArguments(int &argc, char **argv);

    bool getHelp_;
    PixelStreamerType streamerType_;
    QString url_;
    QString rootDir_;
    QString name_;
    unsigned int width_, height_;

    boost::program_options::options_description desc_;
};

#endif // COMMANDLINEOPTIONS_H

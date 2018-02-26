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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "configuration/Process.h"
#include "configuration/Screen.h"
#include "configuration/Surface.h"
#include "scene/Background.h"

/**
 * The Configuration manages all the parameters needed to setup the processes.
 *
 * @warning: this class can only be used AFTER creating a QApplication.
 */
class Configuration
{
public:
    /** Default constructor used for deserialization. */
    Configuration() = default;

    /**
     * Constructor
     * @param filename \see Configuration
     * @throw std::runtime_error if the file could not be read
     */
    Configuration(const QString& filename);

    /** The list of display surfaces. */
    std::vector<Surface> surfaces;

    /** The list of render processes. */
    std::vector<Process> processes;

    /** Background content and color. */
    BackgroundPtr background = Background::create();

    struct Folders
    {
        /** Root directory for opening contents. */
        QString contents;

        /** Sessions directory. */
        QString sessions;

        /** Directory for uploading temporay contents via web interface. */
        QString tmp;

        /** Directory for saving session contents uploaded via web interface. */
        QString upload;
    } folders;

    struct Global
    {
        SwapSync swapsync = SwapSync::software;
    } global;

    struct Launcher
    {
        /** DISPLAY identifier in string format for the Launcher. */
        QString display;

        /** URL of Rendering Resources Manager's API for the demo service. */
        QString demoServiceUrl;

        /** Image folder for the demo service. */
        QString demoServiceImageDir;
    } launcher;

    struct Master
    {
        /** Hostname where the application is running. */
        QString hostname;

        /** Display identifier in string format matching DISPLAY env_var. */
        QString display;

        /** Master application is headless (no visible window) */
        bool headless = false;

        /** Serial port used to control Planar equipment. */
        QString planarSerialPort;

        /** Port for the WebService server to listen for incoming requests. */
        uint16_t webservicePort = 8888;
    } master;

    struct Settings
    {
        /** Informative name of the installation. */
        QString infoName;

        /** Timeout in minutes after which the screens should be turned off. */
        uint inactivityTimeout = 60;

        /** Number of touch points required to power on screens. */
        uint touchpointsToWakeup = 1;

        /** Maximum scaling factor for bitmap contents. */
        double contentMaxScale = 0.0;

        /** Maximum scaling factor for vectorial contents. */
        double contentMaxScaleVectorial = 0.0;
    } settings;

    struct Webbrowser
    {
        /** URL used as start page when opening a Web Browser. */
        QString defaultUrl;

        /** Default size to use when opening a Web Browser. */
        QSize defaultSize{1280, 1024};
    } webbrowser;

    struct Whiteboard
    {
        /** Directory used for saving whiteboard images. */
        QString saveDir;

        /** Default size to use when opening a Web Browser. */
        QSize defaultSize{1920, 1080};
    } whiteboard;

    /**
     * Save the configuration to the current file.
     * @return true on success, false on failure.
     */
    bool save() const;

    /**
     * Save a copy of the configuration to the specified file.
     * @param filename destination file.
     * @return true on success, false on failure.
     */
    bool save(const QString& filename) const;

    /** @return a default configuration, useful for comparing default values. */
    static Configuration defaults();

private:
    QString _filename;

    bool _isXml() const;
    void _loadJson();
    void _loadXml();
    bool _saveJson(const QString& saveFilename) const;
    bool _saveXml(const QString& saveFilename) const;
    void _setDefaults();
};

#endif

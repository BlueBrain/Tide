/*********************************************************************/
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
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

#ifndef MASTERCONFIGURATION_H
#define MASTERCONFIGURATION_H

#include "Configuration.h"

class QXmlQuery;

/**
 * The MasterConfiguration manages all the parameters needed to setup the
 * Master process.
 *
 * @warning: this class can only be used AFTER creating a QApplication.
 */
class MasterConfiguration : public Configuration
{
public:
    /**
     * Constructor
     * @param filename \see Configuration
     * @throw std::runtime_error if the file could not be read
     */
    MasterConfiguration(const QString& filename);

    /**
     * @return true if the master application is headless (no visible window)
     */
    bool getHeadless() const;

    /**
     * Get the root directory for opening contents.
     * @return directory path
     */
    const QString& getContentDir() const;

    /**
     * Get the directory for saving session contents uploaded via web interface.
     * @return directory path
     */
    const QString& getUploadDir() const;

    /**
     * Get the sessions directory
     * @return directory path
     */
    const QString& getSessionsDir() const;

    /**
     * Get the DISPLAY identifier in string format for starting the Launcher.
     */
    const QString& getLauncherDisplay() const;

    /**
     * Get the url for the demo service
     * @return base url of Rendering Resources Manager's API
     */
    const QString& getDemoServiceUrl() const;

    /**
     * Get the image folder for the demo service
     * @return directory with the images
     */
    const QString& getDemoServiceImageFolder() const;

    /**
     * Get the Application Launcher QML file
     * @return file path
     */
    const QString& getAppLauncherFile() const;

    /**
     * Get the port name used to control Planar equipment
     * @return empty string if unspecified
     */
    QString getPlanarSerialPort() const;

    /**
     * Get the port where the WebService server will be listening for incoming
     * requests.
     * @return port for WebService server
     */
    int getWebServicePort() const;

    /**
     * Get the URL used as start page when opening a Web Browser.
     * @return The URL defined in the configuration file, or a default value if
     * none is found.
     */
    const QString& getWebBrowserDefaultURL() const;

    /**
     * Get the folder used for saving whiteboard images
     * @return The URL defined in the configuration file, or a default value if
     * none is found.
     */
    const QString& getWhiteboardSaveFolder() const;

    /**
     * Get the URI to the Content to be used as background
     * @return empty string if unspecified
     */
    const QString& getBackgroundUri() const;

    /**
     * Get the uniform color to use for Background
     * @return defaults to black if unspecified
     */
    const QColor& getBackgroundColor() const;

    /**
     * Set the background color
     * @param color
     */
    void setBackgroundColor(const QColor& color);

    /**
     * Set the URI to the Content to be used as background
     * @param uri empty string to use no background content
     */
    void setBackgroundUri(const QString& uri);

    /**
     * Save the configuration to the current xml file.
     * @return true on succes, false on failure
     */
    bool save() const;

    /**
     * Save the configuration to the specified xml file.
     * @param filename destination file
     * @return true on succes, false on failure
     */
    bool save(const QString& filename) const;

private:
    void loadMasterSettings();
    void loadMasterProcessInfo(QXmlQuery& query);
    void loadContentDirectory(QXmlQuery& query);
    void loadLauncherSettings(QXmlQuery& query);
    void loadPlanarSettings(QXmlQuery& query);
    void loadSessionsDirectory(QXmlQuery& query);
    void loadUploadDirectory(QXmlQuery& query);
    void loadWebService(QXmlQuery& query);
    void loadWhiteboard(QXmlQuery& query);
    void loadAppLauncher(QXmlQuery& query);
    void loadWebBrowserStartURL(QXmlQuery& query);
    void loadBackgroundProperties(QXmlQuery& query);

    bool _headless = false;
    QString _contentDir;
    QString _sessionsDir;
    QString _uploadDir;
    QString _appLauncherFile;

    QString _launcherDisplay;
    QString _demoServiceUrl;
    QString _demoServiceImageFolder;
    QString _whiteboardSaveUrl;

    int _webServicePort;
    QString _webBrowserDefaultURL;

    QString _backgroundUri;
    QColor _backgroundColor;

    QString _planarSerialPort;
};

#endif

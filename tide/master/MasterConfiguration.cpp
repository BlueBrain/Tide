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

#include "MasterConfiguration.h"

#include "log.h"

#include <QDomElement>
#include <QtXmlPatterns>
#include <stdexcept>

namespace
{
const int DEFAULT_WEBSERVICE_PORT = 8888;
const int DEFAULT_PLANAR_TIMEOUT = 60;
const QString DEFAULT_URL("http://www.google.com");
const QString DEFAULT_WHITEBOARD_SAVE_FOLDER("/tmp/");
}

MasterConfiguration::MasterConfiguration(const QString& filename)
    : Configuration(filename)
    , _webServicePort(DEFAULT_WEBSERVICE_PORT)
    , _backgroundColor(Qt::black)
    , _planarTimeout(DEFAULT_PLANAR_TIMEOUT)
{
    loadMasterSettings();
}

void MasterConfiguration::loadMasterSettings()
{
    QXmlQuery query;
    if (!query.setFocus(QUrl(_filename)))
        throw std::runtime_error("Invalid master configuration file: '" +
                                 _filename.toStdString() + "'");

    loadMasterProcessInfo(query);
    loadContentDirectory(query);
    loadSessionsDirectory(query);
    loadUploadDirectory(query);
    loadLauncherSettings(query);
    loadWebService(query);
    loadAppLauncher(query);
    loadWebBrowserStartURL(query);
    loadWhiteboard(query);
    loadBackgroundProperties(query);
    loadPlanarSettings(query);
}

void MasterConfiguration::loadMasterProcessInfo(QXmlQuery& query)
{
    query.setQuery("string(/configuration/masterProcess/@headless)");
    getBool(query, _headless);
}

void MasterConfiguration::loadContentDirectory(QXmlQuery& query)
{
    query.setQuery("string(/configuration/dock/@directory)");
    getString(query, _contentDir);
    if (_contentDir.isEmpty())
        _contentDir = QDir::homePath();
}

void MasterConfiguration::loadSessionsDirectory(QXmlQuery& query)
{
    query.setQuery("string(/configuration/sessions/@directory)");
    getString(query, _sessionsDir);
    if (_sessionsDir.isEmpty())
        _sessionsDir = QDir::homePath();
}

void MasterConfiguration::loadUploadDirectory(QXmlQuery& query)
{
    query.setQuery("string(/configuration/webservice/@uploadDirectory)");
    getString(query, _uploadDir);
    if (_uploadDir.isEmpty())
        _uploadDir = QDir::tempPath();
}

void MasterConfiguration::loadLauncherSettings(QXmlQuery& query)
{
    query.setQuery("string(/configuration/launcher/@display)");
    getString(query, _launcherDisplay);

    query.setQuery("string(/configuration/launcher/@demoServiceUrl)");
    getString(query, _demoServiceUrl);

    query.setQuery("string(/configuration/launcher/@demoServiceImageFolder)");
    getString(query, _demoServiceImageFolder);
}

void MasterConfiguration::loadPlanarSettings(QXmlQuery& query)
{
    query.setQuery("string(/configuration/planar/@serialport)");
    getString(query, _planarSerialPort);

    query.setQuery("string(/configuration/planar/@timeout)");
    getInt(query, _planarTimeout);
}

void MasterConfiguration::loadWebService(QXmlQuery& query)
{
    query.setQuery("string(/configuration/webservice/@port)");
    getUShort(query, _webServicePort);
}

void MasterConfiguration::loadWhiteboard(QXmlQuery& query)
{
    query.setQuery("string(/configuration/whiteboard/@saveUrl)");
    getString(query, _whiteboardSaveUrl);
    if (_whiteboardSaveUrl.isEmpty())
        _whiteboardSaveUrl = DEFAULT_WHITEBOARD_SAVE_FOLDER;
}

void MasterConfiguration::loadAppLauncher(QXmlQuery& query)
{
    query.setQuery("string(/configuration/applauncher/@qml)");
    getString(query, _appLauncherFile);
}

void MasterConfiguration::loadWebBrowserStartURL(QXmlQuery& query)
{
    query.setQuery("string(/configuration/webbrowser/@defaultURL)");
    getString(query, _webBrowserDefaultURL);
    if (_webBrowserDefaultURL.isEmpty())
        _webBrowserDefaultURL = DEFAULT_URL;
}

void MasterConfiguration::loadBackgroundProperties(QXmlQuery& query)
{
    query.setQuery("string(/configuration/background/@uri)");
    getString(query, _backgroundUri);

    QString queryResult;
    query.setQuery("string(/configuration/background/@color)");
    if (getString(query, queryResult))
    {
        const QColor newColor(queryResult);
        if (newColor.isValid())
            _backgroundColor = newColor;
    }
}

bool MasterConfiguration::getHeadless() const
{
    return _headless;
}

const QString& MasterConfiguration::getContentDir() const
{
    return _contentDir;
}

QString MasterConfiguration::getPlanarSerialPort() const
{
    return _planarSerialPort;
}

int MasterConfiguration::getPlanarTimeout() const
{
    return _planarTimeout;
}

const QString& MasterConfiguration::getSessionsDir() const
{
    return _sessionsDir;
}

const QString& MasterConfiguration::getUploadDir() const
{
    return _uploadDir;
}

const QString& MasterConfiguration::getLauncherDisplay() const
{
    return _launcherDisplay;
}

const QString& MasterConfiguration::getDemoServiceUrl() const
{
    return _demoServiceUrl;
}

const QString& MasterConfiguration::getDemoServiceImageFolder() const
{
    return _demoServiceImageFolder;
}

const QString& MasterConfiguration::getAppLauncherFile() const
{
    return _appLauncherFile;
}

uint16_t MasterConfiguration::getWebServicePort() const
{
    return _webServicePort;
}

const QString& MasterConfiguration::getWebBrowserDefaultURL() const
{
    return _webBrowserDefaultURL;
}

const QString& MasterConfiguration::getWhiteboardSaveFolder() const
{
    return _whiteboardSaveUrl;
}

const QString& MasterConfiguration::getBackgroundUri() const
{
    return _backgroundUri;
}

const QColor& MasterConfiguration::getBackgroundColor() const
{
    return _backgroundColor;
}

void MasterConfiguration::setBackgroundColor(const QColor& color)
{
    _backgroundColor = color;
}

void MasterConfiguration::setBackgroundUri(const QString& uri)
{
    _backgroundUri = uri;
}

bool MasterConfiguration::save() const
{
    return save(_filename);
}

bool MasterConfiguration::save(const QString& filename) const
{
    QDomDocument doc("XmlDoc");
    QFile infile(_filename);
    if (!infile.open(QIODevice::ReadOnly))
    {
        put_flog(LOG_ERROR, "could not open configuration file: '%s'",
                 filename.toLocal8Bit().constData());
        return false;
    }
    doc.setContent(&infile);
    infile.close();

    QDomElement root = doc.documentElement();

    QDomElement background = root.firstChildElement("background");
    if (background.isNull())
    {
        background = doc.createElement("background");
        root.appendChild(background);
    }
    background.setAttribute("uri", _backgroundUri);
    background.setAttribute("color", _backgroundColor.name());

    QFile outfile(filename);
    if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        put_flog(LOG_ERROR, "could not save configuration file: '%s'",
                 filename.toLocal8Bit().constData());
        return false;
    }
    QTextStream out(&outfile);
    out << doc.toString(4);
    outfile.close();
    return true;
}

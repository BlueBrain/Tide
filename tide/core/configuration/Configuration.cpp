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

#include "Configuration.h"

#include "XmlParser.h"
#include "utils/log.h"
#include "json/json.h"
#include "json/serialization.h"

#include <QDir>
#include <QDomElement>
#include <QTextStream>

#include <stdexcept>

namespace
{
const QString defaultWebbrowserUrl{"http://www.google.com"};
const QString uri{"string(/configuration/%1/@%2)"};
const QString countProcessesUri{"string(count(//process))"};
const QString processUri{"string(//process[%1]/@%2)"};
const QString dimensionsUri{"string(/configuration/dimensions/@%1)"};
const QString fullscreenUri{"string(/configuration/dimensions/@fullscreen)"};
const QString countScreensUri{"string(count(//process[%1]/screen))"};
const QString setupUri{"string(/configuration/setup/@%1)"};
const QString screenUri{"string(//process[%1]/screen[%2]/@%3)"};

const char* invalidScreenCount{
    "Could not determine the number of screens for this process"};

Screen parseScreen(XmlParser& parser, const int screenIndex,
                   const QString& xpathIndex,
                   const deflect::View defaultStereoMode,
                   const QString& defaultDisplay, const bool fullscreenMode)
{
    Screen screen;
    screen.stereoMode = defaultStereoMode;
    screen.fullscreen = fullscreenMode;

    const auto xpathScreenIndex = QString::number(screenIndex + 1);
    const auto propUri = screenUri.arg(xpathIndex, xpathScreenIndex, "%1");

    parser.get(propUri.arg("display"), screen.display);
    if (screen.display.isEmpty())
        screen.display = defaultDisplay;

    parser.get(propUri.arg("stereo"), screen.stereoMode);

    int value = 0;
    if (parser.get(propUri.arg("x"), value))
        screen.position.setX(value);
    if (parser.get(propUri.arg("y"), value))
        screen.position.setY(value);
    if (parser.get(propUri.arg("i"), value))
        screen.globalIndex.setX(value);
    if (parser.get(propUri.arg("j"), value))
        screen.globalIndex.setY(value);

    return screen;
}

SurfaceConfig parseSurface(XmlParser& parser)
{
    SurfaceConfig surface;
    parser.get(dimensionsUri.arg("numScreensX"), surface.screenCountX);
    parser.get(dimensionsUri.arg("numScreensY"), surface.screenCountY);
    parser.get(dimensionsUri.arg("displayWidth"), surface.displayWidth);
    parser.get(dimensionsUri.arg("displayHeight"), surface.displayHeight);
    parser.get(dimensionsUri.arg("bezelWidth"), surface.bezelWidth);
    parser.get(dimensionsUri.arg("bezelHeight"), surface.bezelHeight);
    parser.get(dimensionsUri.arg("displaysPerScreenX"),
               surface.displaysPerScreenX);
    parser.get(dimensionsUri.arg("displaysPerScreenY"),
               surface.displaysPerScreenY);
    parser.get(dimensionsUri.arg("physicalWidth"), surface.dimensions.rwidth());
    parser.get(dimensionsUri.arg("physicalHeight"),
               surface.dimensions.rheight());

    QString backgroundUri;
    if (parser.get(uri.arg("background", "uri"), backgroundUri))
        surface.background->setUri(backgroundUri);
    QColor backgroundColor;
    if (parser.get(uri.arg("background", "color"), backgroundColor))
        surface.background->setColor(backgroundColor);

    if (surface.displaysPerScreenY == 0 || surface.displaysPerScreenX == 0)
        throw std::invalid_argument("displaysPerScreenX/Y cannot be 0");
    if (surface.screenCountY == 0 || surface.screenCountX == 0)
        throw std::invalid_argument("screenCountX/Y cannot be 0");
    if (surface.displayWidth == 0 || surface.displayHeight == 0)
        throw std::invalid_argument("displayWidth/Height cannot be 0");

    return surface;
}

Process parseProcess(XmlParser& parser, const int processIndex)
{
    Process process;

    // xpath starts from 1
    const auto xpathIndex = QString::number(processIndex + 1);

    parser.get(processUri.arg(xpathIndex, "host"), process.host);

    // global fullscreen flag
    int value = 0;
    parser.get(fullscreenUri, value);
    const bool fullscreen = (value != 0);

    // legacy global settings
    auto stereoMode = deflect::View::mono;
    parser.get(processUri.arg(xpathIndex, "stereo"), stereoMode);
    QString display;
    parser.get(processUri.arg(xpathIndex, "display"), display);

    // read all screens for this process
    int screenCount = 0;
    parser.get(countScreensUri.arg(xpathIndex), screenCount);
    if (screenCount < 1)
        throw std::runtime_error(invalidScreenCount);

    for (int i = 0; i < screenCount; ++i)
    {
        process.screens.emplace_back(parseScreen(parser, i, xpathIndex,
                                                 stereoMode, display,
                                                 fullscreen));
    }

    return process;
}
}

Configuration::Configuration(const QString& filename)
    : _filename{filename}
{
    if (_isXml())
        _loadXml();
    else
        _loadJson();
    _setDefaults();
}

bool Configuration::saveBackgroundChanges() const
{
    return saveBackgroundChanges(_filename);
}

bool Configuration::saveBackgroundChanges(const QString& filename) const
{
    return _isXml() ? _saveXml(filename) : _saveJson(filename);
}

bool Configuration::_isXml() const
{
    return QFileInfo{_filename}.suffix() == "xml";
}

void Configuration::_loadJson()
{
    json::deserialize(json::read(_filename), *this);
}

void Configuration::_loadXml()
{
    XmlParser parser{_filename};

    surfaces.emplace_back(parseSurface(parser));

    auto processCount = 0;
    parser.get(countProcessesUri, processCount);
    for (auto i = 0; i < processCount; ++i)
        processes.emplace_back(parseProcess(parser, i));

    parser.get(uri.arg("setup", "swapsync"), global.swapsync);
    parser.get(uri.arg("masterProcess", "headless"), master.headless);
    parser.get(uri.arg("masterProcess", "host"), master.hostname);
    parser.get(uri.arg("masterProcess", "display"), master.display);
    parser.get(uri.arg("dock", "directory"), folders.contents);
    parser.get(uri.arg("sessions", "directory"), folders.sessions);
    parser.get(uri.arg("webservice", "uploadDirectory"), folders.upload);
    parser.get(uri.arg("webservice", "tmpDirectory"), folders.tmp);
    parser.get(uri.arg("launcher", "display"), launcher.display);
    parser.get(uri.arg("launcher", "demoServiceUrl"), launcher.demoServiceUrl);
    parser.get(uri.arg("info", "name"), settings.infoName);
    parser.get(uri.arg("planar", "serialport"), master.planarSerialPort);
    parser.get(uri.arg("planar", "timeout"), settings.inactivityTimeout);
    parser.get(uri.arg("planar", "touchpointsToPower"),
               settings.touchpointsToWakeup);
    parser.get(uri.arg("webservice", "port"), master.webservicePort);
    parser.get(uri.arg("webbrowser", "defaultURL"), webbrowser.defaultUrl);
    parser.get(uri.arg("webbrowser", "defaultWidth"),
               webbrowser.defaultSize.rwidth());
    parser.get(uri.arg("webbrowser", "defaultHeight"),
               webbrowser.defaultSize.rheight());
    parser.get(uri.arg("whiteboard", "saveUrl"), whiteboard.saveDir);
    parser.get(uri.arg("whiteboard", "defaultWidth"),
               whiteboard.defaultSize.rwidth());
    parser.get(uri.arg("whiteboard", "defaultHeight"),
               whiteboard.defaultSize.rheight());
    parser.get(uri.arg("content", "maxScale"), settings.contentMaxScale);
    parser.get(uri.arg("content", "maxScaleVectorial"),
               settings.contentMaxScaleVectorial);
}

bool Configuration::_saveJson(const QString& filename) const
{
    try
    {
        auto config = json::read(_filename);

        auto jsonSurfaces = config["surfaces"].toArray();
        if (static_cast<uint>(jsonSurfaces.size()) != surfaces.size())
            return false;

        for (auto i = 0u; i < surfaces.size(); ++i)
        {
            auto surface = jsonSurfaces[i].toObject();
            surface["background"] = json::serialize(*surfaces[i].background);
            jsonSurfaces[i] = surface;
        }
        config["surfaces"] = jsonSurfaces;
        json::write(config, filename);
        return true;
    }
    catch (const std::runtime_error&)
    {
        return false;
    }
}

bool Configuration::_saveXml(const QString& filename) const
{
    QDomDocument doc("XmlDoc");
    QFile infile(_filename);
    if (!infile.open(QIODevice::ReadOnly))
    {
        print_log(LOG_ERROR, LOG_GENERAL,
                  "could not open configuration file: '%s'",
                  _filename.toLocal8Bit().constData());
        return false;
    }
    doc.setContent(&infile);
    infile.close();

    auto root = doc.documentElement();
    auto bg = root.firstChildElement("background");
    if (bg.isNull())
    {
        bg = doc.createElement("background");
        root.appendChild(bg);
    }
    bg.setAttribute("uri", surfaces[0].background->getUri());
    bg.setAttribute("color", surfaces[0].background->getColor().name());

    QFile outfile(filename);
    if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        print_log(LOG_ERROR, LOG_GENERAL,
                  "could not save configuration file: '%s'",
                  filename.toLocal8Bit().constData());
        return false;
    }
    QTextStream out(&outfile);
    out << doc.toString(4);
    outfile.close();
    return true;
}

void Configuration::_setDefaults()
{
    if (folders.contents.isEmpty())
        folders.contents = QDir::homePath();
    if (folders.sessions.isEmpty())
        folders.sessions = QDir::homePath();
    if (folders.tmp.isEmpty())
        folders.tmp = QDir::tempPath();
    if (folders.upload.isEmpty())
        folders.upload = QDir::tempPath();
    if (whiteboard.saveDir.isEmpty())
        whiteboard.saveDir = QDir::tempPath();
    if (webbrowser.defaultUrl.isEmpty())
        webbrowser.defaultUrl = defaultWebbrowserUrl;
}

Configuration Configuration::defaults()
{
    Configuration config;
    config.surfaces.resize(1);
    config.processes.resize(1);
    config.processes[0].screens.resize(1);
    config._setDefaults();
    return config;
}

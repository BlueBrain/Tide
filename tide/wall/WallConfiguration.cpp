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

#include "WallConfiguration.h"

#include <QtXmlPatterns>
#include <stdexcept>

namespace
{
deflect::View _toView(const QString& viewString)
{
    if (viewString == "left")
        return deflect::View::left_eye;
    else if (viewString == "right")
        return deflect::View::right_eye;
    else
        return deflect::View::mono;
}
}

WallConfiguration::WallConfiguration(const QString& filename,
                                     const int processIndex)
    : Configuration(filename)
    , _processIndex(processIndex)
{
    assert(processIndex > 0 &&
           "WallConfiguration::loadWallSettings is only"
           "valid for processes of rank > 0");
    _loadWallSettings();
}

void WallConfiguration::_loadWallSettings()
{
    const int xpathIndex = getProcessIndex(); // xpath index also starts from 1

    QXmlQuery query;
    if (!query.setFocus(QUrl(_filename)))
        throw std::runtime_error("Invalid configuration file: '" +
                                 _filename.toStdString() + "'");

    QString queryResult;

    // read host
    query.setQuery(QString("string(//process[%1]/@host)").arg(xpathIndex));
    if (query.evaluateTo(&queryResult))
        _host = queryResult.remove(QRegExp("[\\n\\t\\r]"));

    int value = 0;

    // read number of wall processes on the same host
    query.setQuery(
        QString("string(count(//process[@host eq '%1']))").arg(_host));
    if (!getInt(query, value) || value < 1)
        throw std::runtime_error(
            "Could not determine the number of wall processes on that host");
    _processCountForHost = value;

    // read stereo mode for the process (legacy)
    query.setQuery(QString("string(//process[%1]/@stereo)").arg(xpathIndex));
    if (getString(query, queryResult))
        _stereoMode = _toView(queryResult);

    // read display mode for the process (legacy, optional)
    query.setQuery(QString("string(//process[%1]/@display)").arg(xpathIndex));
    if (query.evaluateTo(&queryResult))
        _display = queryResult.remove(QRegExp("[\\n\\t\\r]"));

    // read all screens for this process
    query.setQuery(
        QString("string(count(//process[%1]/screen))").arg(xpathIndex));
    if (!getInt(query, value))
        throw std::runtime_error(
            "Could not determine the number of screens for this process");

    for (int i = 1; i <= value; ++i) // xpath index starts from 1
        _screens.emplace_back(_loadScreenSettings(query, i));
}

ScreenConfiguration WallConfiguration::_loadScreenSettings(
    QXmlQuery& query, const int screenIndex) const
{
    const int xpathIndex = getProcessIndex(); // xpath index also starts from 1
    QString queryResult;
    int value = 0;

    ScreenConfiguration screen;

    query.setQuery(QString("string(//process[%1]/screen[%2]/@display)")
                       .arg(xpathIndex)
                       .arg(screenIndex));
    if (query.evaluateTo(&queryResult))
        screen.display = queryResult.remove(QRegExp("[\\n\\t\\r]"));
    if (screen.display.isEmpty())
        screen.display = _display;

    query.setQuery(QString("string(//process[%1]/screen[%2]/@x)")
                       .arg(xpathIndex)
                       .arg(screenIndex));
    if (getInt(query, value))
        screen.position.setX(value);

    query.setQuery(QString("string(//process[%1]/screen[%2]/@y)")
                       .arg(xpathIndex)
                       .arg(screenIndex));
    if (getInt(query, value))
        screen.position.setY(value);

    query.setQuery(QString("string(//process[%1]/screen[%2]/@i)")
                       .arg(xpathIndex)
                       .arg(screenIndex));
    if (getInt(query, value))
        screen.globalIndex.setX(value);

    query.setQuery(QString("string(//process[%1]/screen[%2]/@j)")
                       .arg(xpathIndex)
                       .arg(screenIndex));
    if (getInt(query, value))
        screen.globalIndex.setY(value);

    // read stereo mode
    query.setQuery(QString("string(//process[%1]/screen[%2]/@stereo)")
                       .arg(xpathIndex)
                       .arg(screenIndex));
    if (getString(query, queryResult) && !queryResult.isEmpty())
        screen.stereoMode = _toView(queryResult);
    else
        screen.stereoMode = _stereoMode;

    return screen;
}

int WallConfiguration::getProcessIndex() const
{
    return _processIndex;
}

const QString& WallConfiguration::getHost() const
{
    return _host;
}

const std::vector<ScreenConfiguration>& WallConfiguration::getScreens() const
{
    return _screens;
}

int WallConfiguration::getProcessCountForHost() const
{
    return _processCountForHost;
}

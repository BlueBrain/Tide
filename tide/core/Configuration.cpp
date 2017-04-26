/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "scene/VectorialContent.h"

#include <QtXmlPatterns>

#include <stdexcept>

namespace
{
const QRegExp TRIM_REGEX("[\\n\\t\\r]");
}

Configuration::Configuration(const QString& filename)
    : _filename(filename)
    , _totalScreenCountX(0)
    , _totalScreenCountY(0)
    , _screenWidth(0)
    , _screenHeight(0)
    , _mullionWidth(0)
    , _mullionHeight(0)
    , _fullscreen(false)
{
    _load();
}

const QString& Configuration::getFilename() const
{
    return _filename;
}

void Configuration::_load()
{
    QXmlQuery query;
    if (!query.setFocus(QUrl(_filename)))
        throw std::runtime_error("Invalid configuration file: '" +
                                 _filename.toStdString() + "'");

    query.setQuery("string(/configuration/dimensions/@numTilesWidth)");
    getInt(query, _totalScreenCountX);

    query.setQuery("string(/configuration/dimensions/@numTilesHeight)");
    getInt(query, _totalScreenCountY);

    query.setQuery("string(/configuration/dimensions/@screenWidth)");
    getInt(query, _screenWidth);

    query.setQuery("string(/configuration/dimensions/@screenHeight)");
    getInt(query, _screenHeight);

    query.setQuery("string(/configuration/dimensions/@mullionWidth)");
    getInt(query, _mullionWidth);

    query.setQuery("string(/configuration/dimensions/@mullionHeight)");
    getInt(query, _mullionHeight);

    int fullscreen = 0;
    query.setQuery("string(/configuration/dimensions/@fullscreen)");
    getInt(query, fullscreen);
    _fullscreen = fullscreen != 0;

    double value = 0.0;
    query.setQuery("string(/configuration/content/@maxScale)");
    if (getDouble(query, value))
        Content::setMaxScale(value);

    query.setQuery("string(/configuration/content/@maxScaleVectorial)");
    if (getDouble(query, value))
        VectorialContent::setMaxScale(value);
}

int Configuration::getTotalScreenCountX() const
{
    return _totalScreenCountX;
}

int Configuration::getTotalScreenCountY() const
{
    return _totalScreenCountY;
}

int Configuration::getScreenWidth() const
{
    return _screenWidth;
}

int Configuration::getScreenHeight() const
{
    return _screenHeight;
}

int Configuration::getMullionWidth() const
{
    return _mullionWidth;
}

int Configuration::getMullionHeight() const
{
    return _mullionHeight;
}

int Configuration::getTotalWidth() const
{
    return _totalScreenCountX * _screenWidth +
           (_totalScreenCountX - 1) * getMullionWidth();
}

int Configuration::getTotalHeight() const
{
    return _totalScreenCountY * _screenHeight +
           (_totalScreenCountY - 1) * getMullionHeight();
}

QSize Configuration::getTotalSize() const
{
    return QSize(getTotalWidth(), getTotalHeight());
}

double Configuration::getAspectRatio() const
{
    return double(getTotalWidth()) / getTotalHeight();
}

QRect Configuration::getScreenRect(const QPoint& tileIndex) const
{
    assert(tileIndex.x() < _totalScreenCountX);
    assert(tileIndex.y() < _totalScreenCountY);

    const int xPos = tileIndex.x() * (_screenWidth + _mullionWidth);
    const int yPos = tileIndex.y() * (_screenHeight + _mullionHeight);

    return QRect(xPos, yPos, _screenWidth, _screenHeight);
}

bool Configuration::getFullscreen() const
{
    return _fullscreen;
}

bool Configuration::getDouble(const QXmlQuery& query, double& value) const
{
    bool ok = false;
    QString queryResult;
    double tmp = 0.0;
    if (query.evaluateTo(&queryResult))
        tmp = queryResult.toDouble(&ok);
    if (ok)
        value = tmp;
    return ok;
}

bool Configuration::getInt(const QXmlQuery& query, int& value) const
{
    bool ok = false;
    QString queryResult;
    int tmp = 0;
    if (query.evaluateTo(&queryResult))
        tmp = queryResult.toInt(&ok);
    if (ok)
        value = tmp;
    return ok;
}

bool Configuration::getString(const QXmlQuery& query, QString& value) const
{
    QString queryResult;
    if (!query.evaluateTo(&queryResult))
        return false;

    value = queryResult.remove(QRegExp(TRIM_REGEX));
    return true;
}

bool Configuration::getBool(const QXmlQuery& query, bool& value) const
{
    QString result;
    if (!getString(query, result))
        return false;

    if (result == "true")
    {
        value = true;
        return true;
    }
    if (result == "false")
    {
        value = false;
        return true;
    }
    return false;
}

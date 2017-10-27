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
    , _displaysPerScreenX(1)
    , _displaysPerScreenY(1)
    , _totalScreenCountX(1)
    , _totalScreenCountY(1)
    , _displayWidth(0)
    , _displayHeight(0)
    , _bezelWidth(0)
    , _bezelHeight(0)
    , _fullscreen(false)
    , _swapSync(SwapSync::software)
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

    query.setQuery("string(/configuration/dimensions/@numScreensX)");
    getInt(query, _totalScreenCountX);

    query.setQuery("string(/configuration/dimensions/@numScreensY)");
    getInt(query, _totalScreenCountY);

    query.setQuery("string(/configuration/dimensions/@displayWidth)");
    getInt(query, _displayWidth);

    query.setQuery("string(/configuration/dimensions/@displayHeight)");
    getInt(query, _displayHeight);

    query.setQuery("string(/configuration/dimensions/@bezelWidth)");
    getInt(query, _bezelWidth);

    query.setQuery("string(/configuration/dimensions/@bezelHeight)");
    getInt(query, _bezelHeight);

    query.setQuery("string(/configuration/dimensions/@displaysPerScreenX)");
    getInt(query, _displaysPerScreenX);

    query.setQuery("string(/configuration/dimensions/@displaysPerScreenY)");
    getInt(query, _displaysPerScreenY);

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

    QString swapsync;
    query.setQuery("string(/configuration/setup/@swapsync)");
    if (getString(query, swapsync) && swapsync == "hardware")
        _swapSync = SwapSync::hardware;

    _validateSettings();
}

int Configuration::getTotalScreenCountX() const
{
    return _totalScreenCountX;
}

int Configuration::getTotalScreenCountY() const
{
    return _totalScreenCountY;
}

int Configuration::getDisplayWidth() const
{
    return _displayWidth;
}

int Configuration::getDisplayHeight() const
{
    return _displayHeight;
}

int Configuration::getScreenWidth() const
{
    return _displayWidth * _displaysPerScreenX +
           ((_displaysPerScreenX - 1) * _bezelWidth);
}

int Configuration::getScreenHeight() const
{
    return _displayHeight * _displaysPerScreenY +
           ((_displaysPerScreenY - 1) * _bezelHeight);
}

int Configuration::getBezelWidth() const
{
    return _bezelWidth;
}

int Configuration::getBezelHeight() const
{
    return _bezelHeight;
}

int Configuration::getTotalWidth() const
{
    return _totalScreenCountX * getScreenWidth() +
           (_totalScreenCountX - 1) * getBezelWidth();
}

int Configuration::getTotalHeight() const
{
    return _totalScreenCountY * getScreenHeight() +
           (_totalScreenCountY - 1) * getBezelHeight();
}

QSize Configuration::getTotalSize() const
{
    return QSize(getTotalWidth(), getTotalHeight());
}

double Configuration::getAspectRatio() const
{
    if (getTotalHeight() == 0)
        return 0;
    return double(getTotalWidth()) / getTotalHeight();
}

int Configuration::getDisplaysPerScreenX() const
{
    return _displaysPerScreenX;
}

int Configuration::getDisplaysPerScreenY() const
{
    return _displaysPerScreenY;
}

QRect Configuration::getScreenRect(const QPoint& tileIndex) const
{
    if (tileIndex.x() < 0 || tileIndex.x() >= _totalScreenCountX ||
        tileIndex.y() < 0 || tileIndex.y() >= _totalScreenCountY)
    {
        throw std::invalid_argument("tile index does not exist");
    }

    const int xPos = tileIndex.x() * (getScreenWidth() + _bezelWidth);
    const int yPos = tileIndex.y() * (getScreenHeight() + _bezelHeight);

    return QRect(xPos, yPos, getScreenWidth(), getScreenHeight());
}

bool Configuration::getFullscreen() const
{
    return _fullscreen;
}

SwapSync Configuration::getSwapSync() const
{
    return _swapSync;
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

bool Configuration::getUShort(const QXmlQuery& query, ushort& value) const
{
    bool ok = false;
    QString queryResult;
    ushort tmp = 0;
    if (query.evaluateTo(&queryResult))
        tmp = queryResult.toUShort(&ok);
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

void Configuration::_validateSettings()
{
    if (_displaysPerScreenY == 0 || _displaysPerScreenX == 0)
        throw std::invalid_argument("displayPerScreenX/Y cannot be set to 0");
    if (_totalScreenCountY == 0 || _totalScreenCountX == 0)
        throw std::invalid_argument("numScreenX/Y cannot be set to 0");
    if (_displayWidth == 0 || _displayHeight == 0)
        throw std::invalid_argument("displayWidth/Height cannot be set to 0");
}

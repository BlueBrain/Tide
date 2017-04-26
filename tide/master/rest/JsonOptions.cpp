/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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

#include "JsonOptions.h"

#include "json.h"
#include "jsonschema.h"
#include "scene/Options.h"

QJsonObject _makeJsonObject(const Options& options)
{
    return QJsonObject{
        {"alphaBlending", options.isAlphaBlendingEnabled()},
        {"autoFocusStreamers", options.getAutoFocusPixelStreams()},
        {"backgroundColor", options.getBackgroundColor().name()},
        {"background", options.getBackgroundUri()},
        {"clock", options.getShowClock()},
        {"contentTiles", options.getShowContentTiles()},
        {"controlArea", options.getShowControlArea()},
        {"statistics", options.getShowStatistics()},
        {"testPattern", options.getShowTestPattern()},
        {"touchPoints", options.getShowTouchPoints()},
        {"windowBorders", options.getShowWindowBorders()},
        {"windowTitles", options.getShowWindowTitles()},
        {"zoomContext", options.getShowZoomContext()}};
}

JsonOptions::JsonOptions(OptionsPtr options)
    : _options(options)
{
}

std::string JsonOptions::getTypeName() const
{
    return "tide/options";
}

std::string JsonOptions::getSchema() const
{
    return jsonschema::create("Options", _makeJsonObject(*_options),
                              "Options of the Tide application");
}

std::string JsonOptions::_toJSON() const
{
    if (!_options)
        return std::string();

    return json::toString(_makeJsonObject(*_options));
}

bool JsonOptions::_fromJSON(const std::string& string)
{
    if (!_options)
        return false;

    const auto obj = json::toObject(string);
    if (obj.isEmpty())
        return false;

    QJsonValue value;
    value = obj["alphaBlending"];
    if (value.isBool())
        _options->enableAlphaBlending(value.toBool());

    value = obj["autoFocusStreamers"];
    if (value.isBool())
        _options->setAutoFocusPixelStreams(value.toBool());

    value = obj["backgroundColor"];
    if (value.isString())
        _options->setBackgroundColor(QColor(value.toString()));

    value = obj["background"];
    if (value.isString())
        _options->setBackgroundUri(value.toString());

    value = obj["clock"];
    if (value.isBool())
        _options->setShowClock(value.toBool());

    value = obj["contentTiles"];
    if (value.isBool())
        _options->setShowContentTiles(value.toBool());

    value = obj["controlArea"];
    if (value.isBool())
        _options->setShowControlArea(value.toBool());

    value = obj["clock"];
    if (value.isBool())
        _options->setShowClock(value.toBool());

    value = obj["statistics"];
    if (value.isBool())
        _options->setShowStatistics(value.toBool());

    value = obj["testPattern"];
    if (value.isBool())
        _options->setShowTestPattern(value.toBool());

    value = obj["touchPoints"];
    if (value.isBool())
        _options->setShowTouchPoints(value.toBool());

    value = obj["windowBorders"];
    if (value.isBool())
        _options->setShowWindowBorders(value.toBool());

    value = obj["windowTitles"];
    if (value.isBool())
        _options->setShowWindowTitles(value.toBool());

    value = obj["zoomContext"];
    if (value.isBool())
        _options->setShowZoomContext(value.toBool());

    return true;
}

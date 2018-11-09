/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "serialization.h"

#include "configuration/Configuration.h"
#include "scene/Background.h"
#include "scene/Options.h"

#include "json/templates.h"

namespace json
{
QString url_encode(const QUuid& uuid)
{
    return uuid.toString().replace(QRegExp("\\{|\\}"), "");
}

QUuid url_decode(const QString& uuid)
{
    return QUuid{QString("{%1}").arg(uuid)};
}

QJsonValue serialize(const SwapSync swapsync)
{
    switch (swapsync)
    {
    case SwapSync::software:
        return "software";
    case SwapSync::hardware:
        return "hardware";
    default:
        throw std::logic_error("unsupported SwapSync type");
    }
}

QJsonValue serialize(const deflect::View view)
{
    switch (view)
    {
    case deflect::View::mono:
        return "mono";
    case deflect::View::left_eye:
        return "left";
    case deflect::View::right_eye:
        return "right";
    default:
        throw std::logic_error("unsupported deflect::View type");
    }
}

void deserialize(const QJsonValue& value, bool& result)
{
    result = value.toBool(result);
}

void deserialize(const QJsonValue& value, int& result)
{
    result = value.toInt(result);
}

void deserialize(const QJsonValue& value, uint& result)
{
    if (value.isDouble())
    {
        const auto tmp = value.toInt((int)result);
        if (tmp >= 0)
            result = static_cast<uint>(tmp);
    }
}

void deserialize(const QJsonValue& value, ushort& result)
{
    if (value.isDouble())
    {
        const auto tmp = value.toInt((int)result);
        if (tmp >= 0 && tmp <= std::numeric_limits<ushort>::max())
            result = static_cast<ushort>(tmp);
    }
}

void deserialize(const QJsonValue& value, double& result)
{
    result = value.toDouble(result);
}

void deserialize(const QJsonValue& value, QString& result)
{
    if (value.isString())
        result = value.toString();
}

void deserialize(const QJsonValue& value, SwapSync& result)
{
    if (value.isString())
    {
        const auto sync = value.toString();
        if (sync == "software")
            result = SwapSync::software;
        else if (sync == "hardware")
            result = SwapSync::hardware;
    }
}

void deserialize(const QJsonValue& value, deflect::View& result)
{
    if (value.isString())
    {
        const auto view = value.toString();
        if (view == "mono")
            result = deflect::View::mono;
        else if (view == "left")
            result = deflect::View::left_eye;
        else if (view == "right")
            result = deflect::View::right_eye;
    }
}

QJsonArray serialize(const QSize& size)
{
    return QJsonArray{{size.width(), size.height()}};
}

QJsonArray serialize(const QSizeF& size)
{
    return QJsonArray{{size.width(), size.height()}};
}

QJsonObject serializeAsObject(const QSize& size)
{
    return QJsonObject{{"width", size.width()}, {"height", size.height()}};
}

QJsonObject serialize(const Background& background)
{
    return QJsonObject{{"color", background.getColor().name()},
                       {"text", background.getText()},
                       {"uri", background.getUri()}};
}

QJsonObject serialize(const Options& options)
{
    return QJsonObject{
        {"alphaBlending", options.isAlphaBlendingEnabled()},
        {"autoFocusStreamers", options.getAutoFocusPixelStreams()},
        {"clock", options.getShowClock()},
        {"contentTiles", options.getShowContentTiles()},
        {"controlArea", options.getShowControlArea()},
        {"filePaths", options.getShowFilePaths()},
        {"statistics", options.getShowStatistics()},
        {"testPattern", options.getShowTestPattern()},
        {"touchPoints", options.getShowTouchPoints()},
        {"windowBorders", options.getShowWindowBorders()},
        {"windowTitles", options.getShowWindowTitles()},
        {"zoomContext", options.getShowZoomContext()}};
}

QJsonObject serialize(const Configuration& config)
{
    return QJsonObject{
        {"surfaces", serialize(config.surfaces)},
        {"processes", serialize(config.processes)},
        {"folders", QJsonObject{{"contents", config.folders.contents},
                                {"sessions", config.folders.sessions},
                                {"tmp", config.folders.tmp},
                                {"upload", config.folders.upload}}},
        {"global",
         QJsonObject{{"swapsync", serialize(config.global.swapsync)}}},
        {"launcher",
         QJsonObject{{"display", config.launcher.display},
                     {"demoServiceUrl", config.launcher.demoServiceUrl}}},
        {"master",
         QJsonObject{{"host", config.master.hostname},
                     {"display", config.master.display},
                     {"headless", config.master.headless},
                     {"webservicePort", config.master.webservicePort},
                     {"planarSerialPort", config.master.planarSerialPort}}},
        {"settings",
         QJsonObject{{"infoName", config.settings.infoName},
                     {"touchpointsToWakeup",
                      static_cast<int>(config.settings.touchpointsToWakeup)},
                     {"inactivityTimeout",
                      static_cast<int>(config.settings.inactivityTimeout)},
                     {"contentMaxScale", config.settings.contentMaxScale},
                     {"contentMaxScaleVectorial",
                      config.settings.contentMaxScaleVectorial}}},
        {"webbrowser", QJsonObject{{"defaultUrl", config.webbrowser.defaultUrl},
                                   {"defaultSize",
                                    serialize(config.webbrowser.defaultSize)}}},
        {"whiteboard",
         QJsonObject{{"saveDir", config.whiteboard.saveDir},
                     {"defaultSize",
                      serialize(config.whiteboard.defaultSize)}}}};
}

QJsonObject serialize(const Process& process)
{
    return QJsonObject{{"host", process.host},
                       {"screens", serialize(process.screens)}};
}

QJsonObject serialize(const Screen& screen)
{
    return QJsonObject{{"surface", static_cast<int>(screen.surfaceIndex)},
                       {"display", screen.display},
                       {"x", screen.position.x()},
                       {"y", screen.position.y()},
                       {"i", screen.globalIndex.x()},
                       {"j", screen.globalIndex.y()},
                       {"stereo", serialize(screen.stereoMode)},
                       {"fullscreen", screen.fullscreen}};
}

QJsonObject serialize(const SurfaceConfig& surface)
{
    return QJsonObject{
        {"displayWidth", static_cast<int>(surface.displayWidth)},
        {"displayHeight", static_cast<int>(surface.displayHeight)},
        {"displaysPerScreenX", static_cast<int>(surface.displaysPerScreenX)},
        {"displaysPerScreenY", static_cast<int>(surface.displaysPerScreenY)},
        {"screenCountX", static_cast<int>(surface.screenCountX)},
        {"screenCountY", static_cast<int>(surface.screenCountY)},
        {"bezelWidth", surface.bezelWidth},
        {"bezelHeight", surface.bezelHeight},
        {"background", serialize(*surface.background)},
        {"dimensions", serialize(surface.dimensions)}};
}

bool deserialize(const QJsonValue& value, QSize& size)
{
    if (value.isArray())
        return deserialize(value.toArray(), size);

    if (value.isObject())
        return deserialize(value.toObject(), size);

    return false;
}

bool deserialize(const QJsonValue& value, QSizeF& size)
{
    if (value.isArray())
        return deserialize(value.toArray(), size);

    if (value.isObject())
        return deserialize(value.toObject(), size);

    return false;
}

bool deserialize(const QJsonArray& array, QSize& size)
{
    if (array.size() == 2 && array[0].isDouble() && array[1].isDouble())
    {
        size = QSize{array[0].toInt(), array[1].toInt()};
        return true;
    }
    return false;
}

bool deserialize(const QJsonArray& array, QSizeF& size)
{
    if (array.size() == 2 && array[0].isDouble() && array[1].isDouble())
    {
        size = QSizeF{array[0].toDouble(), array[1].toDouble()};
        return true;
    }
    return false;
}

bool deserialize(const QJsonObject& object, QSize& size)
{
    if (!object["width"].isDouble() || !object["height"].isDouble())
        return false;

    size = QSize{object["width"].toInt(), object["height"].toInt()};
    return true;
}

bool deserialize(const QJsonObject& object, QSizeF& size)
{
    if (!object["width"].isDouble() || !object["height"].isDouble())
        return false;

    size = QSizeF{object["width"].toDouble(), object["height"].toDouble()};
    return true;
}

bool deserialize(const QJsonObject& object, Background& background)
{
    if (!object.contains("color") && !object.contains("uri"))
        return false;

    QJsonValue value;
    value = object["color"];
    if (value.isString())
    {
        const auto color = QColor{value.toString()};
        if (!color.isValid())
            return false;
        background.setColor(color);
    }

    value = object["text"];
    if (value.isString())
        background.setText(value.toString());

    value = object["uri"];
    if (value.isString())
        background.setUri(value.toString());

    return true;
}

bool deserialize(const QJsonObject& object, Options& options)
{
    if (object.isEmpty())
        return false;

    QJsonValue value;
    value = object["alphaBlending"];
    if (value.isBool())
        options.enableAlphaBlending(value.toBool());

    value = object["autoFocusStreamers"];
    if (value.isBool())
        options.setAutoFocusPixelStreams(value.toBool());

    value = object["clock"];
    if (value.isBool())
        options.setShowClock(value.toBool());

    value = object["contentTiles"];
    if (value.isBool())
        options.setShowContentTiles(value.toBool());

    value = object["controlArea"];
    if (value.isBool())
        options.setShowControlArea(value.toBool());

    value = object["filePaths"];
    if (value.isBool())
        options.setShowFilePaths(value.toBool());

    value = object["statistics"];
    if (value.isBool())
        options.setShowStatistics(value.toBool());

    value = object["testPattern"];
    if (value.isBool())
        options.setShowTestPattern(value.toBool());

    value = object["touchPoints"];
    if (value.isBool())
        options.setShowTouchPoints(value.toBool());

    value = object["windowBorders"];
    if (value.isBool())
        options.setShowWindowBorders(value.toBool());

    value = object["windowTitles"];
    if (value.isBool())
        options.setShowWindowTitles(value.toBool());

    value = object["zoomContext"];
    if (value.isBool())
        options.setShowZoomContext(value.toBool());

    return true;
}

bool deserialize(const QJsonObject& object, Configuration& config)
{
    if (object.isEmpty())
        return false;

    deserialize(object["surfaces"], config.surfaces);
    deserialize(object["processes"], config.processes);

    const auto foldersObj = object["folders"].toObject();
    deserialize(foldersObj["contents"], config.folders.contents);
    deserialize(foldersObj["sessions"], config.folders.sessions);
    deserialize(foldersObj["tmp"], config.folders.tmp);
    deserialize(foldersObj["upload"], config.folders.upload);

    const auto globalObj = object["global"].toObject();
    deserialize(globalObj["swapsync"], config.global.swapsync);

    const auto launcherObj = object["launcher"].toObject();
    deserialize(launcherObj["display"], config.launcher.display);
    deserialize(launcherObj["demoServiceUrl"], config.launcher.demoServiceUrl);

    const auto masterObj = object["master"].toObject();
    deserialize(masterObj["host"], config.master.hostname);
    deserialize(masterObj["display"], config.master.display);
    deserialize(masterObj["headless"], config.master.headless);
    deserialize(masterObj["webservicePort"], config.master.webservicePort);
    deserialize(masterObj["planarSerialPort"], config.master.planarSerialPort);

    const auto settingsObj = object["settings"].toObject();
    deserialize(settingsObj["infoName"], config.settings.infoName);
    deserialize(settingsObj["touchpointsToWakeup"],
                config.settings.touchpointsToWakeup);
    deserialize(settingsObj["inactivityTimeout"],
                config.settings.inactivityTimeout);
    deserialize(settingsObj["contentMaxScale"],
                config.settings.contentMaxScale);
    deserialize(settingsObj["contentMaxScaleVectorial"],
                config.settings.contentMaxScaleVectorial);

    const auto webbrowserObj = object["webbrowser"].toObject();
    deserialize(webbrowserObj["defaultUrl"], config.webbrowser.defaultUrl);
    deserialize(webbrowserObj["defaultSize"], config.webbrowser.defaultSize);

    const auto whiteboardObj = object["whiteboard"].toObject();
    deserialize(whiteboardObj["saveDir"], config.whiteboard.saveDir);
    deserialize(whiteboardObj["defaultSize"], config.whiteboard.defaultSize);

    return true;
}

bool deserialize(const QJsonObject& object, Process& process)
{
    if (!object["host"].isString() || !object["screens"].isArray())
        return false;

    deserialize(object["host"], process.host);
    deserialize(object["screens"], process.screens);

    return true;
}

bool deserialize(const QJsonObject& object, Screen& screen)
{
    if (object.isEmpty())
        return false;

    deserialize(object["surface"], screen.surfaceIndex);
    deserialize(object["display"], screen.display);
    deserialize(object["x"], screen.position.rx());
    deserialize(object["y"], screen.position.ry());
    deserialize(object["i"], screen.globalIndex.rx());
    deserialize(object["j"], screen.globalIndex.ry());
    deserialize(object["stereo"], screen.stereoMode);
    deserialize(object["fullscreen"], screen.fullscreen);

    return true;
}

bool deserialize(const QJsonObject& object, SurfaceConfig& surface)
{
    if (object.isEmpty())
        return false;

    deserialize(object["background"].toObject(), *surface.background);
    deserialize(object["bezelWidth"], surface.bezelWidth);
    deserialize(object["bezelHeight"], surface.bezelHeight);
    deserialize(object["dimensions"], surface.dimensions);
    deserialize(object["displayWidth"], surface.displayWidth);
    deserialize(object["displayHeight"], surface.displayHeight);
    deserialize(object["displaysPerScreenX"], surface.displaysPerScreenX);
    deserialize(object["displaysPerScreenY"], surface.displaysPerScreenY);
    deserialize(object["screenCountX"], surface.screenCountX);
    deserialize(object["screenCountY"], surface.screenCountY);

    return true;
}
}

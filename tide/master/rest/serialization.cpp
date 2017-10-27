/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#include "serialization.h"

#include "LoggingUtility.h"
#include "MasterConfiguration.h"
#include "control/ContentWindowController.h"
#include "json.h"
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
#include "scene/Options.h"

#include <tide/master/version.h>

#include <QDateTime>
#include <QHostInfo>

namespace
{
const QString _applicationStartTime = QDateTime::currentDateTime().toString();

QString to_qstring(const ScreenState state)
{
    switch (state)
    {
    case ScreenState::ON:
        return "ON";
    case ScreenState::OFF:
        return "OFF";
    default:
        return "UNDEF";
    }
}
}

QJsonObject to_json_object(ContentWindowPtr window, const DisplayGroup& group)
{
    const ContentWindowController controller(*window, group);
    return QJsonObject{
        {"aspectRatio", window->getContent()->getAspectRatio()},
        {"minWidth", controller.getMinSizeAspectRatioCorrect().width()},
        {"minHeight", controller.getMinSizeAspectRatioCorrect().height()},
        {"width", window->getDisplayCoordinates().width()},
        {"height", window->getDisplayCoordinates().height()},
        {"x", window->getDisplayCoordinates().x()},
        {"y", window->getDisplayCoordinates().y()},
        {"z", group.getZindex(window)},
        {"title", window->getContent()->getTitle()},
        {"mode", window->getMode()},
        {"selected", window->isSelected()},
        {"fullscreen", window->isFullscreen()},
        {"focus", window->isFocused()},
        {"uri", window->getContent()->getURI()},
        {"visible", window->getState() == ContentWindow::HIDDEN ? false : true},
        {"uuid", url_encode(window->getID())}};
}

QJsonObject to_json_object(const DisplayGroup& group)
{
    QJsonArray windows;
    for (const auto& window : group.getContentWindows())
    {
        if (window->getContent()->getURI() == "Launcher")
            continue;

        windows.append(to_json_object(window, group));
    }
    return QJsonObject{{"windows", windows}};
}

QJsonObject to_json_object(const LoggingUtility& logger)
{
    const QJsonObject event{{"last_event", logger.getLastInteractionName()},
                            {"last_event_date",
                             logger.getLastInteractionTime()},
                            {"count", logger.getInteractionCount()}};
    const QJsonObject window{{"count", int(logger.getWindowCount())},
                             {"date_set",
                              logger.getWindowCountModificationTime()},
                             {"accumulated_count",
                              int(logger.getAccumulatedWindowCount())}};
    const QJsonObject screens{{"state", to_qstring(logger.getScreenState())},
                              {"last_change",
                               logger.getScreenStateModificationTime()}};
    return QJsonObject{{"event", event},
                       {"window", window},
                       {"screens", screens}};
}

QJsonObject to_json_object(const MasterConfiguration& config)
{
    QJsonObject configObject{
        {"hostname", QHostInfo::localHostName()},
        {"version", QString::fromStdString(tide::Version::getString())},
        {"revision", QString::number(tide::Version::getRevision(), 16)},
        {"startTime", _applicationStartTime},
        {"wallSize", to_json_object(config.getTotalSize())},
        {"dimensions",
         QJsonObject{{"screenCountX", config.getTotalScreenCountX()},
                     {"screenCountY", config.getTotalScreenCountY()},
                     {"bezelHeight", config.getBezelHeight()},
                     {"bezelWidth", config.getBezelWidth()},
                     {"displayWidth", config.getDisplayWidth()},
                     {"displayHeight", config.getDisplayHeight()},
                     {"screenWidth", config.getScreenWidth()},
                     {"screenHeight", config.getScreenHeight()},
                     {"displaysPerScreenX", config.getDisplaysPerScreenX()},
                     {"displaysPerScreenY", config.getDisplaysPerScreenY()}}},
        {"backgroundColor", config.getBackgroundColor().name()},
        {"contentDir", config.getContentDir()},
        {"sessionDir", config.getSessionsDir()},
        {"name", config.getInfoName()},
        {"filters", QJsonArray::fromStringList(
                        ContentFactory::getSupportedFilesFilter())}};
    return QJsonObject{{"config", configObject}};
}

QJsonObject to_json_object(const Options& options)
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

QJsonObject to_json_object(const QSize& size)
{
    return QJsonObject{{"width", size.width()}, {"height", size.height()}};
}

bool from_json_object(Options& options, const QJsonObject& object)
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

    value = object["backgroundColor"];
    if (value.isString())
        options.setBackgroundColor(QColor(value.toString()));

    value = object["background"];
    if (value.isString())
        options.setBackgroundUri(value.toString());

    value = object["clock"];
    if (value.isBool())
        options.setShowClock(value.toBool());

    value = object["contentTiles"];
    if (value.isBool())
        options.setShowContentTiles(value.toBool());

    value = object["controlArea"];
    if (value.isBool())
        options.setShowControlArea(value.toBool());

    value = object["clock"];
    if (value.isBool())
        options.setShowClock(value.toBool());

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

QString url_encode(const QUuid& uuid)
{
    return uuid.toString().replace(QRegExp("\\{|\\}"), "");
}

QUuid url_decode(const QString& uuid)
{
    return QUuid{QString("{%1}").arg(uuid)};
}

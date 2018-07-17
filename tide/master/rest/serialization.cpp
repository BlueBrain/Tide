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

#include "configuration/Configuration.h"
#include "control/WindowController.h"
#include "localstreamer/PixelStreamerLauncher.h"
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
#include "scene/Scene.h"
#include "tools/ActivityLogger.h"
#include "json/json.h"
#include "json/serialization.h"
#include "json/templates.h"

#include <tide/master/version.h>

#include <QDateTime>
#include <QHostInfo>

namespace
{
const QString _applicationStartTime = QDateTime::currentDateTime().toString();

QSizeF _getMinSize(const Window& window, const DisplayGroup& group)
{
    return WindowController(const_cast<Window&>(window), group)
        .getMinSizeAspectRatioCorrect();
}
}

namespace
{
QString to_qstring(const ScreenState state)
{
    switch (state)
    {
    case ScreenState::on:
        return "ON";
    case ScreenState::off:
        return "OFF";
    default:
        return "UNDEF";
    }
}
}

namespace json
{
QJsonObject serialize(const Window& window, const DisplayGroup& group)
{
    const auto minSize = _getMinSize(window, group);
    const auto coordinates = window.getDisplayCoordinates();

    return QJsonObject{{"aspectRatio", window.getContent().getAspectRatio()},
                       {"minWidth", minSize.width()},
                       {"minHeight", minSize.height()},
                       {"width", coordinates.width()},
                       {"height", coordinates.height()},
                       {"x", coordinates.x()},
                       {"y", coordinates.y()},
                       {"title", window.getContent().getTitle()},
                       {"mode", window.getMode()},
                       {"selected", window.isSelected()},
                       {"fullscreen", window.isFullscreen()},
                       {"focus", window.isFocused()},
                       {"uri", window.getContent().getUri()},
                       {"visible", !window.isHidden()},
                       {"uuid", json::url_encode(window.getID())}};
}

QJsonObject serialize(const DisplayGroup& group)
{
    QJsonArray windows;
    for (const auto& window : group.getWindows())
    {
        if (window->getContent().getUri() == PixelStreamerLauncher::launcherUri)
            continue;

        windows.append(serialize(*window, group));
    }
    return QJsonObject{{"windows", windows}};
}

QJsonObject serialize(const Surface& surface)
{
    return serialize(surface.getGroup());
}

QJsonArray serialize(const Scene& scene)
{
    return serialize(scene.getSurfaces());
}

QJsonObject serializeForRest(const Configuration& config)
{
    std::vector<QSize> surfaceSizes;
    for (const auto& surface : config.surfaces)
        surfaceSizes.emplace_back(surface.getTotalSize());

    QJsonObject configObject{
        {"hostname", QHostInfo::localHostName()},
        {"version", QString::fromStdString(tide::Version::getString())},
        {"revision", QString::number(tide::Version::getRevision(), 16)},
        {"startTime", _applicationStartTime},
        {"surfaceSizes", serialize(surfaceSizes)},
        {"surfaces", serialize(config.surfaces)},
        {"contentDir", config.folders.contents},
        {"sessionDir", config.folders.sessions},
        {"name", config.settings.infoName},
        {"filters", QJsonArray::fromStringList(
                        ContentFactory::getSupportedFilesFilter())}};
    return QJsonObject{{"config", configObject}};
}

QJsonObject serialize(const ActivityLogger& logger)
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
}

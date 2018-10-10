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

#include "SessionSaver.h"

#include "Session.h"
#include "SessionPreview.h"
#include "SessionPreviewGenerator.h"
#include "control/DisplayGroupController.h"
#include "scene/ContentFactory.h"
#include "scene/ErrorContent.h"
#include "scene/Scene.h"
#include "serialization/utils.h"
#include "utils/log.h"

#include <QFileInfo>

namespace
{
const QString SESSION_FILE_EXTENSION(".dcx");

bool _canBeRestored(const ContentType type)
{
    // PixelStreams are external applications and can't be restored.
    if (type == ContentType::pixel_stream)
        return false;

    return true;
}

bool _isErrorContent(const Content& content)
{
    return !!dynamic_cast<const ErrorContent*>(&content);
}

bool _canBeSaved(const Content& content)
{
    return _canBeRestored(content.getType()) && !_isErrorContent(content);
}

void _relocateContent(Window& window, const QString& tmpDir,
                      const QString& dstDir)
{
    const auto& uri = window.getContent().getUri();
    if (!uri.startsWith(tmpDir))
        return;

    const auto newUri = SessionSaver::findAvailableFilePath(uri, dstDir);
    if (!QDir().rename(uri, newUri))
    {
        print_log(LOG_WARN, LOG_CONTENT, "Failed to move %s to : %s",
                  uri.toLocal8Bit().constData(),
                  newUri.toLocal8Bit().constData());
        return;
    }
    window.setContent(ContentFactory::getContent(newUri));
}

WindowPtrs _findWindowsToRelocate(const DisplayGroup& group,
                                  const QString& tmpDir)
{
    WindowPtrs windowsToRelocate;
    for (const auto& window : group.getWindows())
    {
        const auto& uri = window->getContent().getUri();
        if (QFileInfo{uri}.absolutePath() == tmpDir)
            windowsToRelocate.push_back(window);
    }
    return windowsToRelocate;
}

void _relocateTempContents(DisplayGroup& group, const QString& tmpDir,
                           const QString& dstDir)
{
    if (QDir{dstDir}.exists())
    {
        print_log(LOG_WARN, LOG_CONTENT,
                  "Moving content to existing session folder: '%s'",
                  dstDir.toLocal8Bit().constData());
    }
    else if (!QDir().mkpath(dstDir))
    {
        print_log(LOG_WARN, LOG_CONTENT,
                  "Cannot create a new session folder: '%s'",
                  dstDir.toLocal8Bit().constData());
        return;
    }

    for (const auto& window : _findWindowsToRelocate(group, tmpDir))
    {
        _relocateContent(*window, tmpDir, dstDir);
        // Remove the window and add back a copy of it to ensure that the wall
        // processes use the new URI to access the file.
        // Note: the content must be relocated before removing the window,
        // otherwise the MasterApplication destroys the temporary file.
        group.remove(window);
        group.add(serialization::xmlCopy(window));
    }
}

void _relocateTempContents(Scene& scene, const QString& tmpDir,
                           const QString& dstDir)
{
    for (auto&& surface : scene.getSurfaces())
        _relocateTempContents(surface.getGroup(), tmpDir, dstDir);
}
}

SessionSaver::SessionSaver(ScenePtr scene, const QString& tmpDir,
                           const QString& uploadDir)
    : _scene{std::move(scene)}
    , _tmpDir{tmpDir}
    , _uploadDir{uploadDir}
{
}

void _generatePreview(const Scene& scene, const QString& filepath)
{
    auto image = SessionPreviewGenerator().generateImage(scene.getGroup(0));
    SessionPreview{filepath}.saveToFile(image);
}

void _filterContents(DisplayGroup& group)
{
    const auto& windows = group.getWindows();

    WindowPtrs filteredWindows;
    filteredWindows.reserve(windows.size());

    std::copy_if(windows.begin(), windows.end(),
                 std::back_inserter(filteredWindows),
                 [](const WindowPtr& window) {
                     return _canBeSaved(window->getContent());
                 });
    group.replaceWindows(filteredWindows);
}

void _filterContents(Scene& scene)
{
    for (auto&& surface : scene.getSurfaces())
        _filterContents(surface.getGroup());
}

QFuture<bool> SessionSaver::save(QString filepath, const bool generatePreview)
{
    if (!filepath.endsWith(SESSION_FILE_EXTENSION))
    {
        filepath.append(SESSION_FILE_EXTENSION);
        print_log(LOG_VERBOSE, LOG_CONTENT, "appended %s extension",
                  SESSION_FILE_EXTENSION.toLocal8Bit().constData());
    }

    print_log(LOG_INFO, LOG_CONTENT, "Saving session: '%s'",
              filepath.toStdString().c_str());

    if (!_uploadDir.isEmpty())
    {
        const auto sessionName = QFileInfo{filepath}.baseName();
        _relocateTempContents(*_scene, _tmpDir, _uploadDir + "/" + sessionName);
    }

    // Important: use xml archive not binary as they use different code paths
    auto scene = serialization::xmlCopy(_scene);
    return QtConcurrent::run([scene, filepath, generatePreview]() {
        _filterContents(*scene);

        // Create preview before session so that thumbnail shows in file browser
        if (generatePreview)
            _generatePreview(*scene, filepath);

        if (!serialization::toXmlFile(Session{scene}, filepath.toStdString()))
            return false;

        return true;
    });
}

QString SessionSaver::findAvailableFilePath(const QString& filepath,
                                            const QString& dstDir)
{
    const auto file = QFileInfo(filepath);
    const auto dir = QDir(dstDir).absolutePath();

    auto newUri = dir + "/" + file.fileName();
    auto nameSuffix = 0;
    while (QFile(newUri).exists())
    {
        newUri = QString("%1/%2_%3.%4")
                     .arg(dir, file.baseName(), QString::number(++nameSuffix),
                          file.suffix());
    }
    return newUri;
}

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

#include "StateSerializationHelper.h"

#include "State.h"
#include "StatePreview.h"
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

    const auto newUri =
        StateSerializationHelper::findAvailableFilePath(uri, dstDir);
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

bool _validateContent(const WindowPtr& window)
{
    auto content = window->getContentPtr();
    if (!content)
    {
        print_log(LOG_WARN, LOG_CONTENT, "Window '%s' does not have a Content!",
                  window->getID().toString().toLocal8Bit().constData());
        return false;
    }

    if (!_canBeRestored(content->getType()))
        return false;

    // Some regular textures were saved as DynamicTexture type before the
    // migration to qml2 rendering
    if (content->getType() == ContentType::dynamic_texture)
    {
        const auto& uri = content->getUri();
        const auto type = ContentFactory::getContentTypeForFile(uri);
        if (type == ContentType::image)
        {
            print_log(LOG_DEBUG, LOG_CONTENT,
                      "Try restoring legacy DynamicTexture as "
                      "a regular image: '%s'",
                      content->getUri().toLocal8Bit().constData());

            auto newContent = ContentFactory::getContent(uri);
            newContent->moveToThread(window->thread());
            window->setContent(std::move(newContent));
            content = window->getContentPtr();
        }
        else
        {
            print_log(LOG_INFO, LOG_CONTENT,
                      "DynamicTexture are no longer supported. Please"
                      "convert the source image to a tiff pyramid: "
                      "'%s'",
                      content->getUri().toLocal8Bit().constData());
        }
    }

    // Refresh content information, files can have been modified or removed
    // since the state was saved.
    if (window->getContent().readMetadata())
    {
        print_log(LOG_DEBUG, LOG_CONTENT, "Restoring content: '%s'",
                  content->getUri().toLocal8Bit().constData());
    }
    else
    {
        print_log(LOG_WARN, LOG_CONTENT, "'%s' could not be restored!",
                  content->getUri().toLocal8Bit().constData());
        auto errorContent = ContentFactory::getErrorContent(*content);
        errorContent->moveToThread(window->thread());
        window->setContent(std::move(errorContent));
    }
    return true;
}

void _validateContents(DisplayGroup& group)
{
    using Windows = QVector<WindowPtr>;
    auto windows = Windows::fromStdVector(group.getWindows());

    QtConcurrent::blockingFilter(windows, _validateContent);

    group.replaceWindows(windows.toStdVector());
}

void _validateContents(Scene& scene)
{
    for (auto&& surface : scene.getSurfaces())
        _validateContents(surface.getGroup());
}

void _adjust(DisplayGroup& group, const DisplayGroup& referenceGroup)
{
    // Reshape the new DisplayGroup only if it doesn't fit (legacy behaviour).
    // If the saved group was smaller, resize it but don't modify its windows.
    if (!referenceGroup.getCoordinates().contains(group.getCoordinates()))
        DisplayGroupController{group}.reshape(referenceGroup.size());
    else
    {
        group.setWidth(referenceGroup.width());
        group.setHeight(referenceGroup.height());
    }
}

void _adjust(Scene& scene, const Scene& currentScene)
{
    const auto max =
        std::min(scene.getSurfaceCount(), currentScene.getSurfaceCount());

    for (auto i = 0u; i < max; ++i)
        _adjust(scene.getGroup(i), currentScene.getGroup(i));
}
}

StateSerializationHelper::StateSerializationHelper(ScenePtr scene)
    : _scene(scene)
{
}

ScenePtr _load(const QString& filename, SceneConstPtr referenceScene)
{
    State state;
    // For backward compatibility, try to load the file as a legacy xml first
    if (!state.legacyLoadXML(filename) &&
        !serialization::fromXmlFile(state, filename.toStdString()))
    {
        return ScenePtr();
    }

    auto scene = state.getScene();
    _validateContents(*scene);

    auto& group = scene->getGroup(0);
    const auto& referenceGroup = referenceScene->getGroup(0);

    DisplayGroupController controller(group);
    controller.updateFocusedWindowsCoordinates();

    if (state.getVersion() < FIRST_PIXEL_COORDINATES_FILE_VERSION)
        controller.denormalize(referenceGroup.size());
    else if (state.getVersion() == FIRST_PIXEL_COORDINATES_FILE_VERSION)
    {
        // Approximation; only applies to FIRST_PIXEL_COORDINATES_FILE_VERSION
        // which did not serialize the size of the DisplayGroup
        assert(group.getCoordinates().isEmpty());
        group.setCoordinates(controller.estimateSurface());
    }

    _adjust(*scene, *referenceScene);

    scene->moveToThread(QCoreApplication::instance()->thread());
    return scene;
}

QFuture<ScenePtr> StateSerializationHelper::load(const QString& filename) const
{
    print_log(LOG_INFO, LOG_CONTENT, "Restoring session: '%s'",
              filename.toStdString().c_str());

    return QtConcurrent::run([ referenceScene = _scene, filename ]() {
        return _load(filename, referenceScene);
    });
}

void _generatePreview(const Scene& scene, const QString& filename)
{
    const auto& group = scene.getGroup(0);
    const auto& windows = group.getWindows();
    const auto size = group.size().toSize();

    StatePreview filePreview(filename);
    filePreview.generateImage(size, windows);
    filePreview.saveToFile();
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

QFuture<bool> StateSerializationHelper::save(QString filename,
                                             const QString& tmpDir,
                                             const QString& uploadDir,
                                             const bool generatePreview)
{
    if (!filename.endsWith(SESSION_FILE_EXTENSION))
    {
        filename.append(SESSION_FILE_EXTENSION);
        print_log(LOG_VERBOSE, LOG_CONTENT, "appended %s filename extension",
                  SESSION_FILE_EXTENSION.toLocal8Bit().constData());
    }

    print_log(LOG_INFO, LOG_CONTENT, "Saving session: '%s'",
              filename.toStdString().c_str());

    if (!uploadDir.isEmpty())
    {
        const auto sessionName = QFileInfo{filename}.baseName();
        _relocateTempContents(*_scene, tmpDir, uploadDir + "/" + sessionName);
    }

    // Important: use xml archive not binary as they use different code paths
    auto scene = serialization::xmlCopy(_scene);
    return QtConcurrent::run([scene, filename, generatePreview]() {
        _filterContents(*scene);

        // Create preview before session so that thumbnail shows in file browser
        if (generatePreview)
            _generatePreview(*scene, filename);

        if (!serialization::toXmlFile(State{scene}, filename.toStdString()))
            return false;

        return true;
    });
}

QString StateSerializationHelper::findAvailableFilePath(const QString& filename,
                                                        const QString& dstDir)
{
    const auto file = QFileInfo(filename);
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

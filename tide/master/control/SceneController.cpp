/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#include "SceneController.h"

#include "ContentLoader.h"
#include "StateSerializationHelper.h"
#include "config.h"
#include "control/DisplayGroupController.h"
#include "scene/Scene.h"

#if TIDE_ENABLE_WEBBROWSER_SUPPORT
#include "scene/WebbrowserContent.h"
#endif

SceneController::SceneController(Scene& scene_,
                                 const Configuration::Folders& folders)
    : _scene{scene_}
    , _folders(folders)
{
    for (const auto& surface : _scene.getSurfaces())
    {
        connect(&surface.getGroup(), &DisplayGroup::windowRemoved, this,
                &SceneController::_deleteTempContentFile);
    }

    connect(&_loadSessionOp, &QFutureWatcher<ScenePtr>::finished, [this]() {
        auto scene = _loadSessionOp.result();
        if (scene)
            apply(scene);

        if (_loadSessionCallback)
            _loadSessionCallback(scene != nullptr);
        _loadSessionCallback = nullptr;
    });

    connect(&_saveSessionOp, &QFutureWatcher<bool>::finished, [this]() {
        if (_saveSessionCallback)
            _saveSessionCallback(_saveSessionOp.result());
        _saveSessionCallback = nullptr;
    });
}

void SceneController::open(const uint surfaceIndex, const QString& uri,
                           const QPointF& coords, BoolCallback callback)
{
    auto success = false;
    try
    {
        auto loader = ContentLoader{_scene.getGroup(surfaceIndex)};
        success = loader.loadOrMoveToFront(uri, coords);
    }
    catch (const invalid_surface_index_error& e)
    {
        put_log(LOG_DEBUG, "%s: %d", e.what(), surfaceIndex);
    }
    if (callback)
        callback(success);
}

void SceneController::load(const QString& sessionFile, BoolCallback callback)
{
    _loadSessionOp.waitForFinished();
    _loadSessionCallback = callback;
    StateSerializationHelper helper(_scene.shared_from_this());
    _loadSessionOp.setFuture(helper.load(sessionFile));
}

void SceneController::save(const QString& sessionFile, BoolCallback callback)
{
    _saveSessionOp.waitForFinished();
    _saveSessionCallback = callback;

    StateSerializationHelper helper(_scene.shared_from_this());
    _saveSessionOp.setFuture(
        helper.save(sessionFile, _folders.tmp, _folders.upload));
}

void SceneController::hideLauncher()
{
    DisplayGroupController{_scene.getGroup(0)}.hidePanels();
}

std::unique_ptr<WindowController> SceneController::getController(
    const QUuid& winId)
{
    auto res = _scene.findWindowAndGroup(winId);
    return std::make_unique<WindowController>(res.first, res.second);
}

void SceneController::apply(SceneConstPtr scene)
{
    const auto max =
        std::min(scene->getSurfaceCount(), _scene.getSurfaceCount());

    for (auto i = 0u; i < max; ++i)
    {
        const auto& sourceGroup = scene->getGroup(i);
        _scene.getGroup(i).replaceWindows(sourceGroup.getWindows());
    }

#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    _restoreWebbrowsers(_scene);
#endif
}

void SceneController::_restoreWebbrowsers(const Scene& scene)
{
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    using WebContent = const WebbrowserContent*;
    for (const auto& window : scene.getWindows())
    {
        if (auto browser = dynamic_cast<WebContent>(window->getContentPtr()))
            emit startWebbrowser(*browser);
    }
#else
    Q_UNUSED(scene);
#endif
}

void SceneController::_deleteTempContentFile(WindowPtr window)
{
    const auto isFile = contentTypeIsFile(window->getContent().getType());
    const auto& filename = window->getContent().getURI();
    if (isFile && QFileInfo(filename).absolutePath() == _folders.tmp)
    {
        QDir().remove(filename);
        print_log(LOG_INFO, LOG_REST, "Deleted temporary file: %s",
                  filename.toLocal8Bit().constData());
    }
}

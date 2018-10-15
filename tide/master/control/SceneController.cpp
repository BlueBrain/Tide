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
#include "control/DisplayGroupController.h"
#include "scene/Scene.h"

#include <QDir>
#include <QFileInfo>

namespace
{
const auto surface0 = 0u;
}

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
}

void SceneController::openAll(const QStringList& uris)
{
    for (const auto& uri : uris)
        open(surface0, uri, QPointF(), BoolCallback());
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

void SceneController::clear(const uint surfaceIndex)
{
    try
    {
        _scene.getGroup(surfaceIndex).clear();
    }
    catch (const invalid_surface_index_error& e)
    {
        put_log(LOG_DEBUG, "%s: %d", e.what(), surfaceIndex);
    }
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

void SceneController::_deleteTempContentFile(WindowPtr window)
{
    const auto isFile = contentTypeIsFile(window->getContent().getType());
    const auto& filename = window->getContent().getUri();
    if (isFile && QFileInfo(filename).absolutePath() == _folders.tmp)
    {
        QDir().remove(filename);
        print_log(LOG_INFO, LOG_REST, "Deleted temporary file: %s",
                  filename.toLocal8Bit().constData());
    }
}

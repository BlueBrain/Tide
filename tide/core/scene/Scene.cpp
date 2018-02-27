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

#include "Scene.h"

#include "configuration/SurfaceConfig.h"

ScenePtr Scene::create(const QSize& size)
{
    return ScenePtr{new Scene{{DisplayGroup::create(size)}}};
}

ScenePtr Scene::create(const std::vector<SurfaceConfig>& surfaces)
{
    return ScenePtr{new Scene{surfaces}};
}

ScenePtr Scene::create(const std::vector<DisplayGroupPtr>& groups)
{
    return ScenePtr{new Scene{groups}};
}

ScenePtr Scene::create(DisplayGroupPtr group)
{
    return ScenePtr{new Scene{{std::move(group)}}};
}

Scene::Scene(const std::vector<SurfaceConfig>& surfaces)
{
    for (const auto& surface : surfaces)
    {
        _surfaces.emplace_back(DisplayGroup::create(surface.getTotalSize()),
                               surface.background);
    }
    _forwardSceneModifiedSignals();
}

Scene::Scene(const std::vector<DisplayGroupPtr>& groups)
{
    for (const auto& group : groups)
        _surfaces.emplace_back(group);

    _forwardSceneModifiedSignals();
}

Scene::~Scene()
{
    for (auto&& surface : _surfaces)
        surface.getGroup().setParent(nullptr); // avoid double deletion
}

size_t Scene::getSurfaceCount() const
{
    return _surfaces.size();
}

const std::vector<Surface>& Scene::getSurfaces() const
{
    return _surfaces;
}

DisplayGroup& Scene::getGroup(const size_t surfaceIndex)
{
    if (surfaceIndex >= _surfaces.size())
        throw invalid_surface_index_error("no group for this surface");

    return _surfaces[surfaceIndex].getGroup();
}

const DisplayGroup& Scene::getGroup(const size_t surfaceIndex) const
{
    if (surfaceIndex >= _surfaces.size())
        throw invalid_surface_index_error("no group for this surface");

    return _surfaces[surfaceIndex].getGroup();
}

ContentWindowPtrs Scene::getContentWindows() const
{
    ContentWindowPtrs windows;
    for (const auto& surface : _surfaces)
    {
        const auto& list = surface.getGroup().getContentWindows();
        windows.insert(windows.end(), list.begin(), list.end());
    }
    return windows;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
void Scene::moveToThread(QThread* thread)
{
    QObject::moveToThread(thread);
    for (auto&& surface : _surfaces)
    {
        surface.getGroup().moveToThread(thread);
        surface.getBackground().moveToThread(thread);
    }
}
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

ContentWindowPtr Scene::findWindow(const QUuid& id) const
{
    for (const auto& surface : _surfaces)
    {
        if (auto window = surface.getGroup().getContentWindow(id))
            return window;
    }
    return ContentWindowPtr();
}

std::pair<ContentWindow&, DisplayGroup&> Scene::findWindowAndGroup(
    const QUuid& id)
{
    for (auto&& surface : _surfaces)
    {
        if (auto window = surface.getGroup().getContentWindow(id))
            return {*window, surface.getGroup()};
    }
    throw window_not_found_error("window not found");
}

std::pair<ContentWindowPtr, DisplayGroup&> Scene::findWindowPtrAndGroup(
    const QUuid& id)
{
    for (auto&& surface : _surfaces)
    {
        if (auto window = surface.getGroup().getContentWindow(id))
            return {window, surface.getGroup()};
    }
    throw window_not_found_error("window not found");
}

void Scene::_forwardSceneModifiedSignals()
{
    for (auto&& surface : _surfaces)
    {
        connect(&surface.getGroup(), &DisplayGroup::modified, this,
                &Scene::_sendScene);
        connect(&surface.getBackground(), &Background::updated, this,
                &Scene::_sendScene);
    }
}

void Scene::_sendScene()
{
    emit modified(shared_from_this());
}

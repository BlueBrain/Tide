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

#include "Scene.h"

#include "configuration/SurfaceConfig.h"
#include "utils/compilerMacros.h"

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
    size_t index = 0;
    for (const auto& surface : surfaces)
    {
        _surfaces.emplace_back(std::make_shared<Surface>(
            index++, DisplayGroup::create(surface.getTotalSize()),
            surface.background));
    }
    _forwardSignals();
}

Scene::Scene(const std::vector<DisplayGroupPtr>& groups)
{
    size_t index = 0;
    for (const auto& group : groups)
        _surfaces.emplace_back(std::make_shared<Surface>(index++, group));
    _forwardSignals();
}

Scene::~Scene()
{
    for (auto&& surface : _surfaces)
        surface->getGroup().setParent(nullptr); // avoid double deletion
}

void Scene::assign(const Scene& other)
{
    const auto max = std::min(getSurfaceCount(), other.getSurfaceCount());
    for (auto i = 0u; i < max; ++i)
        getGroup(i).replaceWindows(other.getGroup(i).getWindows());
}

void Scene::clear()
{
    for (auto& surface : getSurfaces())
        surface.getGroup().clear();
}

bool Scene::isEmpty() const
{
    for (const auto& surface : getSurfaces())
        if (!surface.getGroup().isEmpty())
            return false;
    return true;
}

size_t Scene::getSurfaceCount() const
{
    return _surfaces.size();
}

Surface& Scene::getSurface(size_t surfaceIndex)
{
    if (surfaceIndex >= _surfaces.size())
        throw invalid_surface_index_error("no surface with this index");

    return *_surfaces.at(surfaceIndex);
}

const Surface& Scene::getSurface(const size_t surfaceIndex) const
{
    if (surfaceIndex >= _surfaces.size())
        throw invalid_surface_index_error("no surface with this index");

    return *_surfaces.at(surfaceIndex);
}

SurfacePtr Scene::getSurfacePtr(size_t surfaceIndex) const
{
    if (surfaceIndex >= _surfaces.size())
        throw invalid_surface_index_error("no surface with this index");

    return _surfaces.at(surfaceIndex);
}

DisplayGroup& Scene::getGroup(const size_t surfaceIndex)
{
    if (surfaceIndex >= _surfaces.size())
        throw invalid_surface_index_error("no group for this surface");

    return _surfaces[surfaceIndex]->getGroup();
}

const DisplayGroup& Scene::getGroup(const size_t surfaceIndex) const
{
    if (surfaceIndex >= _surfaces.size())
        throw invalid_surface_index_error("no group for this surface");

    return _surfaces[surfaceIndex]->getGroup();
}

WindowPtrs Scene::getWindows() const
{
    WindowPtrs windows;
    for (const auto& surface : getSurfaces())
    {
        const auto& list = surface.getGroup().getWindows();
        windows.insert(windows.end(), list.begin(), list.end());
    }
    return windows;
}

TIDE_DISABLE_WARNING_SHADOW
void Scene::moveToThread(QThread* thread)
{
    QObject::moveToThread(thread);
    for (auto&& surface : getSurfaces())
        surface.moveToThread(thread);
}
TIDE_DISABLE_WARNING_SHADOW_END

WindowPtr Scene::findWindow(const QUuid& id) const
{
    for (const auto& surface : getSurfaces())
    {
        if (auto window = surface.getGroup().getWindow(id))
            return window;
    }
    return WindowPtr();
}

std::pair<Window&, DisplayGroup&> Scene::findWindowAndGroup(const QUuid& id)
{
    for (auto&& surface : getSurfaces())
    {
        if (auto window = surface.getGroup().getWindow(id))
            return {*window, surface.getGroup()};
    }
    throw window_not_found_error("window not found");
}

std::pair<WindowPtr, DisplayGroup&> Scene::findWindowPtrAndGroup(
    const QUuid& id)
{
    for (auto&& surface : getSurfaces())
    {
        if (auto window = surface.getGroup().getWindow(id))
            return {window, surface.getGroup()};
    }
    throw window_not_found_error("window not found");
}

void Scene::_forwardSignals()
{
    _forwardSceneModifiedSignals();
    _forwardIsEmptyChangedSignals();
}

void Scene::_forwardSceneModifiedSignals()
{
    for (auto&& surface : getSurfaces())
        connect(&surface, &Surface::modified, this, &Scene::_sendScene);
}

void Scene::_forwardIsEmptyChangedSignals()
{
    for (auto&& surface : getSurfaces())
    {
        connect(&surface.getGroup(), &DisplayGroup::isEmptyChanged, this,
                &Scene::isEmptyChanged);
    }
}

void Scene::_sendScene()
{
    emit modified(shared_from_this());
}

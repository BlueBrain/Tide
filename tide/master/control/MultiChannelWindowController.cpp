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

#include "MultiChannelWindowController.h"

#include "control/DisplayGroupController.h"
#include "scene/MultiChannelContent.h"
#include "scene/Scene.h"

MultiChannelWindowController::MultiChannelWindowController(Scene& scene)
    : _scene(scene)
{
    for (const auto& surface : scene.getSurfaces())
        _monitor(surface.getGroup());
}

void MultiChannelWindowController::closeAllWindows(const QString& uri)
{
    auto it = _contentToWindowsMap.find(uri);
    if (it != _contentToWindowsMap.end())
        _closeAll(it->second);
}

void MultiChannelWindowController::closeSingleWindow(const QString& uri,
                                                     const QUuid& windowId)
{
    // dereference window before _onWindowRemoved() is called
    auto it = _contentToWindowsMap.find(uri);
    if (it != _contentToWindowsMap.end() && it->second.erase(windowId))
        _close(windowId);
}

void MultiChannelWindowController::_monitor(const DisplayGroup& group)
{
    connect(&group, &DisplayGroup::windowAdded, this,
            &MultiChannelWindowController::_onWindowAdded);

    connect(&group, &DisplayGroup::windowRemoved, this,
            &MultiChannelWindowController::_onWindowRemoved);
}

void MultiChannelWindowController::_onWindowAdded(WindowPtr window)
{
    try
    {
        const auto& content =
            dynamic_cast<const MultiChannelContent&>(window->getContent());

        _contentToWindowsMap[content.getUri()].insert(window->getID());

        connect(window.get(), &Window::modeChanged,
                [ this, windowWeakPtr = std::weak_ptr<Window>(window) ] {
                    if (auto win = windowWeakPtr.lock())
                    {
                        _changeFullscreenModeForAllWindows(
                            win->getContent().getUri(), win->isFullscreen());
                    }
                });
    }
    catch (const std::bad_cast&)
    {
    }
}

void MultiChannelWindowController::_onWindowRemoved(WindowPtr window)
{
    try
    {
        const auto& content =
            dynamic_cast<const MultiChannelContent&>(window->getContent());

        auto& windows = _contentToWindowsMap[content.getUri()];

        // Close all windows if a window is closed by the user on any surface
        if (windows.erase(window->getID()))
            _closeAll(windows);

        if (windows.empty())
            _contentToWindowsMap.erase(content.getUri());
    }
    catch (const std::bad_cast&)
    {
    }
}

void MultiChannelWindowController::_changeFullscreenModeForAllWindows(
    const QString& uri, const bool fullscreenMode)
{
    for (const auto& windowId : _contentToWindowsMap.at(uri))
        _changeFullscreenMode(windowId, fullscreenMode);
}

void MultiChannelWindowController::_changeFullscreenMode(
    const QUuid& windowId, const bool fullscreenMode)
{
    const auto windowAndGroup = _scene.findWindowAndGroup(windowId);
    const auto windowIsFullscreen = windowAndGroup.first.isFullscreen();

    DisplayGroupController controller{windowAndGroup.second};

    if (fullscreenMode && !windowIsFullscreen)
        controller.showFullscreen(windowAndGroup.first.getID());
    else if (!fullscreenMode && windowIsFullscreen)
        controller.exitFullscreen();
}

void MultiChannelWindowController::_closeAll(const std::set<QUuid> windows)
{
    for (const auto& windowId : windows)
        _close(windowId);
}

void MultiChannelWindowController::_close(const QUuid& windowId)
{
    try
    {
        auto wg = _scene.findWindowAndGroup(windowId);
        DisplayGroupController{wg.second}.remove(wg.first.getID());
    }
    catch (const window_not_found_error&)
    {
    }
}

/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
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

#include "SceneRemoteController.h"

#include "control/WindowController.h"
#include "json/json.h"
#include "json/serialization.h"

using namespace rockets;

namespace
{
const jsonrpc::Response noWindow{
    jsonrpc::Response::Error{"window does not exist", 2}};
const jsonrpc::Response ok{"\"OK\""};
}

template <typename Obj>
bool from_json(Obj& object, const std::string& json)
{
    return object.fromJson(json::parse(json));
}

struct SurfaceIndex
{
    uint surfaceIndex = 0;

    bool fromJson(const QJsonObject& object)
    {
        json::deserialize(object["surfaceIndex"], surfaceIndex);
        return true;
    }
};

struct WindowId
{
    QUuid id;

    bool fromJson(const QJsonObject& obj)
    {
        id = obj["id"].toString();
        return !id.isNull();
    }
};

struct WindowPos
{
    QUuid id;
    QPointF pos;

    bool fromJson(const QJsonObject& obj)
    {
        if (!obj["id"].isString() || !obj["x"].isDouble() ||
            !obj["y"].isDouble())
        {
            return false;
        }
        id = obj["id"].toString();
        pos = QPointF(obj["x"].toDouble(), obj["y"].toDouble());
        return !id.isNull();
    }
};

struct WindowSize
{
    QUuid id;
    QSizeF size;
    WindowPoint fixedPoint;

    bool fromJson(const QJsonObject& obj)
    {
        if (!obj["id"].isString() || !obj["w"].isDouble() ||
            !obj["h"].isDouble() || !obj["centered"].isBool())
        {
            return false;
        }
        id = obj["id"].toString();
        size = QSizeF(obj["w"].toDouble(), obj["h"].toDouble());
        fixedPoint = obj["centered"].toBool() ? WindowPoint::CENTER
                                              : WindowPoint::TOP_LEFT;
        return !id.isNull();
    }
};

SceneRemoteController::SceneRemoteController(Scene& scene)
    : _scene{scene}
{
    connect<SurfaceIndex>("deselect-windows", [this](const auto params) {
        try
        {
            auto& group = _scene.getGroup(params.surfaceIndex);
            DisplayGroupController{group}.deselectAll();
        }
        catch (const invalid_surface_index_error&)
        {
        }
    });

    connect<SurfaceIndex>("exit-fullscreen", [this](const auto params) {
        try
        {
            auto& group = _scene.getGroup(params.surfaceIndex);
            DisplayGroupController{group}.exitFullscreen();
        }
        catch (const invalid_surface_index_error&)
        {
        }
    });

    connect<SurfaceIndex>("focus-windows", [this](const auto params) {
        try
        {
            auto& group = _scene.getGroup(params.surfaceIndex);
            DisplayGroupController{group}.focusSelected();
        }
        catch (const invalid_surface_index_error&)
        {
        }
    });

    connect<SurfaceIndex>("focus-all-windows", [this](const auto params) {
        try
        {
            auto& group = _scene.getGroup(params.surfaceIndex);
            DisplayGroupController{group}.focusAll();
        }
        catch (const invalid_surface_index_error&)
        {
        }
    });

    connect<SurfaceIndex>("unfocus-windows", [this](const auto params) {
        try
        {
            auto& group = _scene.getGroup(params.surfaceIndex);
            DisplayGroupController{group}.unfocusAll();
        }
        catch (const invalid_surface_index_error&)
        {
        }
    });

    connect<SurfaceIndex>("clear", [this](const auto params) {
        try
        {
            _scene.getGroup(params.surfaceIndex).clear();
        }
        catch (const invalid_surface_index_error&)
        {
        }
    });

    connect("clear-all", [this]() { _scene.clear(); });

    bind<WindowId>("close-window", [this](const auto params) {
        try
        {
            auto windowAndGroup = _scene.findWindowPtrAndGroup(params.id);
            if (windowAndGroup.first->isFocused())
            {
                DisplayGroupController controller{windowAndGroup.second};
                controller.unfocus(params.id);
            }
            windowAndGroup.second.remove(windowAndGroup.first);
            return ok;
        }
        catch (const window_not_found_error&)
        {
            return noWindow;
        }
    });

    bind<WindowPos>("move-window", [this](const auto params) {
        try
        {
            auto windowAndGroup = _scene.findWindowAndGroup(params.id);
            auto& window = windowAndGroup.first;
            auto& group = windowAndGroup.second;

            WindowController(window, group)
                .moveTo(params.pos, WindowPoint::TOP_LEFT);
            DisplayGroupController{group}.moveWindowToFront(params.id);
            return ok;
        }
        catch (const window_not_found_error&)
        {
            return noWindow;
        }
    });

    bind<WindowId>("move-window-to-front", [this](const auto params) {
        try
        {
            auto windowAndGroup = _scene.findWindowAndGroup(params.id);
            DisplayGroupController controller{windowAndGroup.second};
            return controller.moveWindowToFront(params.id) ? ok : noWindow;
        }
        catch (const window_not_found_error&)
        {
            return noWindow;
        }
    });

    bind<WindowId>("move-window-to-fullscreen", [this](const auto params) {
        try
        {
            auto windowAndGroup = _scene.findWindowAndGroup(params.id);
            DisplayGroupController controller{windowAndGroup.second};
            return controller.showFullscreen(params.id) ? ok : noWindow;
        }
        catch (const window_not_found_error&)
        {
            return noWindow;
        }
    });

    bind<WindowSize>("resize-window", [this](const auto params) {
        try
        {
            auto windowAndGroup = _scene.findWindowAndGroup(params.id);
            auto& window = windowAndGroup.first;
            auto& group = windowAndGroup.second;

            WindowController(window, group)
                .resize(params.size, params.fixedPoint);
            DisplayGroupController{group}.moveWindowToFront(params.id);
            return ok;
        }
        catch (const window_not_found_error&)
        {
            return noWindow;
        }
    });

    bind<WindowId>("toggle-select-window", [this](const auto params) {
        try
        {
            auto window = _scene.findWindow(params.id);
            window->setSelected(!window->isSelected());
            return ok;
        }
        catch (const window_not_found_error&)
        {
            return noWindow;
        }
    });

    bind<WindowId>("unfocus-window", [this](const auto params) {
        try
        {
            auto windowAndGroup = _scene.findWindowAndGroup(params.id);
            DisplayGroupController controller{windowAndGroup.second};
            return controller.unfocus(params.id) ? ok : noWindow;
        }
        catch (const window_not_found_error&)
        {
            return noWindow;
        }
    });
}

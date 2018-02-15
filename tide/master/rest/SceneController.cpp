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

#include "control/ContentWindowController.h"
#include "json/json.h"

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

SceneController::SceneController(DisplayGroup& group)
    : _group{group}
    , _controller{group}
{
    connect("deselect-windows", [this] { _controller.deselectAll(); });
    connect("exit-fullscreen", [this] { _controller.exitFullscreen(); });
    connect("focus-windows", [this] { _controller.focusSelected(); });
    connect("unfocus-windows", [this] { _controller.unfocusAll(); });
    connect("clear", [this] { _group.clear(); });

    bind<WindowId>("close-window", [this](const WindowId params) {
        if (auto window = _group.getContentWindow(params.id))
        {
            _group.removeContentWindow(window);
            return ok;
        }
        return noWindow;
    });

    bind<WindowPos>("move-window", [this](const WindowPos params) {
        if (auto window = _group.getContentWindow(params.id))
        {
            ContentWindowController(*window, _group)
                .moveTo(params.pos, WindowPoint::TOP_LEFT);
            _controller.moveWindowToFront(params.id);
            return ok;
        }
        return noWindow;
    });

    bind<WindowId>("move-window-to-front", [this](const WindowId params) {
        return _controller.moveWindowToFront(params.id) ? ok : noWindow;
    });

    bind<WindowId>("move-window-to-fullscreen", [this](const WindowId params) {
        return _controller.showFullscreen(params.id) ? ok : noWindow;
    });

    bind<WindowSize>("resize-window", [this](const WindowSize params) {
        if (auto window = _group.getContentWindow(params.id))
        {
            ContentWindowController(*window, _group)
                .resize(params.size, params.fixedPoint);
            return ok;
        }
        return noWindow;
    });

    bind<WindowId>("toggle-select-window", [this](const WindowId params) {
        if (auto window = _group.getContentWindow(params.id))
        {
            window->setSelected(!window->isSelected());
            return ok;
        }
        return noWindow;
    });

    bind<WindowId>("unfocus-window", [this](const WindowId params) {
        return _controller.unfocus(params.id) ? ok : noWindow;
    });
}

/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#include "WindowResizeHandlesController.h"

#include "WindowController.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"

WindowResizeHandlesController::WindowResizeHandlesController(
    Window& window, const DisplayGroup& group)
    : _window{window}
    , _displayGroup{group}
{
}

void WindowResizeHandlesController::startResizing(
    const Window::ResizeHandle handle)
{
    _window.setActiveHandle(handle);
    _window.setState(Window::WindowState::RESIZING);
}

void WindowResizeHandlesController::toggleResizeMode()
{
    if (_window.getContent().hasFixedAspectRatio())
        _window.setResizePolicy(Window::ResizePolicy::ADJUST_CONTENT);
    else
        _window.setResizePolicy(Window::ResizePolicy::KEEP_ASPECT_RATIO);
}

void WindowResizeHandlesController::stopResizing()
{
    _window.setState(Window::WindowState::NONE);
    _window.setActiveHandle(Window::ResizeHandle::NOHANDLE);

    if (_window.getContent().hasFixedAspectRatio())
        _window.setResizePolicy(Window::ResizePolicy::KEEP_ASPECT_RATIO);
    else
        _window.setResizePolicy(Window::ResizePolicy::ADJUST_CONTENT);
}

void WindowResizeHandlesController::resizeRelative(const QPointF& delta)
{
    const auto& coord = _window.getDisplayCoordinates();

    auto fixedPoint = QPointF();
    auto newSize = coord.size();

    switch (_window.getActiveHandle())
    {
    case Window::TOP:
        fixedPoint =
            QPointF(coord.left() + coord.width() * 0.5, coord.bottom());
        newSize += QSizeF(0, -delta.y());
        break;
    case Window::RIGHT:
        fixedPoint = QPointF(coord.left(), coord.top() + coord.height() * 0.5);
        newSize += QSizeF(delta.x(), 0);
        break;
    case Window::BOTTOM:
        fixedPoint = QPointF(coord.left() + coord.width() * 0.5, coord.top());
        newSize += QSizeF(0, delta.y());
        break;
    case Window::LEFT:
        fixedPoint = QPointF(coord.right(), coord.top() + coord.height() * 0.5);
        newSize += QSizeF(-delta.x(), 0);
        break;
    case Window::TOP_LEFT:
        fixedPoint = coord.bottomRight();
        newSize += QSizeF(-delta.x(), -delta.y());
        break;
    case Window::BOTTOM_LEFT:
        fixedPoint = coord.topRight();
        newSize += QSizeF(-delta.x(), delta.y());
        break;
    case Window::TOP_RIGHT:
        fixedPoint = coord.bottomLeft();
        newSize += QSizeF(delta.x(), -delta.y());
        break;
    case Window::BOTTOM_RIGHT:
        fixedPoint = coord.topLeft();
        newSize += QSizeF(delta.x(), delta.y());
        break;
    case Window::NOHANDLE:
        return;
    }

    auto controller = WindowController{_window, _displayGroup};
    controller.resize(fixedPoint, newSize, _window.getResizePolicy());
}

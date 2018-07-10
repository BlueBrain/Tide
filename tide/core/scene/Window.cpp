/*********************************************************************/
/* Copyright (c) 2011-2012, The University of Texas at Austin.       */
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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

#include "Window.h"

IMPLEMENT_SERIALIZE_FOR_XML(Window)

Window::Window(ContentPtr content, const WindowType type)
    : _type{type}
{
    setContent(std::move(content));
    _coordinates.setSize(_content->getDimensions());
    connect(this, &Window::modified, [this] { ++_version; });
}

Window::Window()
{
    connect(this, &Window::modified, [this] { ++_version; });
}

Window::Window(ContentPtr content, const QUuid& uuid)
    : _uuid{uuid}
{
    setContent(std::move(content));
}

Window::~Window()
{
    if (_content)
        _content->setParent(nullptr); // avoid double deletion
}

const QUuid& Window::getID() const
{
    return _uuid;
}

bool Window::isPanel() const
{
    return _type == WindowType::PANEL;
}

Content& Window::getContent()
{
    return *_content;
}

const Content& Window::getContent() const
{
    return *_content;
}

Content* Window::getContentPtr() const
{
    return _content.get();
}

void Window::setContent(ContentPtr content)
{
    if (!content)
        throw std::invalid_argument("Window content cannot be null");

    content->setParent(this);
    _content = std::move(content);

    setResizePolicy(_content->hasFixedAspectRatio() ? KEEP_ASPECT_RATIO
                                                    : ADJUST_CONTENT);
    _initContentConnections();
}

void Window::setCoordinates(const QRectF& coordinates)
{
    if (coordinates == _coordinates)
        return;

    setX(coordinates.x());
    setY(coordinates.y());
    setWidth(coordinates.width());
    setHeight(coordinates.height());

    emit coordinatesChanged();

    emit modified();
}

Window::ResizeHandle Window::getActiveHandle() const
{
    return _activeHandle;
}

void Window::setActiveHandle(const Window::ResizeHandle handle)
{
    if (_activeHandle == handle)
        return;

    _activeHandle = handle;
    emit activeHandleChanged();
    emit modified();
}

Window::ResizePolicy Window::getResizePolicy() const
{
    return _resizePolicy;
}

bool Window::setResizePolicy(const Window::ResizePolicy policy)
{
    if (policy == _resizePolicy)
        return true;

    if (policy == ADJUST_CONTENT && _content->hasFixedAspectRatio() &&
        !_content->canBeZoomed())
    {
        return false;
    }

    _resizePolicy = policy;
    emit resizePolicyChanged();
    emit modified();
    return true;
}

Window::WindowState Window::getState() const
{
    return _state;
}

Window::WindowMode Window::getMode() const
{
    return _mode;
}

void Window::setMode(const Window::WindowMode mode)
{
    if (mode == _mode)
        return;

    _mode = mode;
    emit modeChanged();
    emit modified();
}

bool Window::isFocused() const
{
    return _mode == WindowMode::FOCUSED;
}

bool Window::isFullscreen() const
{
    return _mode == WindowMode::FULLSCREEN;
}

const QRectF& Window::getFocusedCoordinates() const
{
    return _focusedCoordinates;
}

void Window::setFocusedCoordinates(const QRectF& coordinates)
{
    if (coordinates == _focusedCoordinates)
        return;

    _focusedCoordinates = coordinates;
    emit focusedCoordinatesChanged();
    emit modified();
}

const QRectF& Window::getFullscreenCoordinates() const
{
    return _fullscreenCoordinates;
}

void Window::setFullscreenCoordinates(const QRectF& coordinates)
{
    if (coordinates == _fullscreenCoordinates)
        return;

    _fullscreenCoordinates = coordinates;
    emit fullscreenCoordinatesChanged();
    emit modified();
}

const QRectF& Window::getDisplayCoordinates() const
{
    switch (getMode())
    {
    case WindowMode::FULLSCREEN:
        return getFullscreenCoordinates();
    case WindowMode::FOCUSED:
        return getFocusedCoordinates();
    case WindowMode::STANDARD:
    default:
        return getCoordinates();
    }
}

void Window::setDisplayCoordinates(const QRectF& coordinates)
{
    switch (getMode())
    {
    case WindowMode::FULLSCREEN:
        setFullscreenCoordinates(coordinates);
        break;
    case WindowMode::FOCUSED:
        setFocusedCoordinates(coordinates);
        break;
    case WindowMode::STANDARD:
    default:
        setCoordinates(coordinates);
        break;
    }
}

bool Window::setState(const Window::WindowState state)
{
    if (_state == state)
        return false;

    const auto prevState = _state;
    _state = state;

    if (prevState == Window::HIDDEN)
        emit hiddenChanged(false);
    else if (state == Window::HIDDEN)
        emit hiddenChanged(true);

    emit stateChanged();
    emit modified();
    return true;
}

bool Window::isIdle() const
{
    return _state == NONE;
}

bool Window::isMoving() const
{
    return _state == MOVING;
}

bool Window::isResizing() const
{
    return _state == RESIZING;
}

bool Window::isHidden() const
{
    return _state == HIDDEN;
}

bool Window::isSelected() const
{
    return _selected;
}

size_t Window::getVersion() const
{
    return _version;
}

void Window::backupModeAndZoom()
{
    _backupMode = getMode();
    _backupZoom = _content->getZoomRect();
}

void Window::restoreModeAndZoom()
{
    setMode(_backupMode);
    _content->setZoomRect(_backupZoom);
}

void Window::setSelected(const bool value)
{
    if (value == _selected)
        return;

    _selected = value;
    emit selectedChanged();
    emit modified();
}

void Window::_initContentConnections()
{
    connect(_content.get(), &Content::modified, [this] { ++_version; });
    connect(_content.get(), &Content::modified, this, &Window::contentModified);
}

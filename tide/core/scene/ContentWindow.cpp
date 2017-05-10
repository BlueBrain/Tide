/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "ContentWindow.h"

IMPLEMENT_SERIALIZE_FOR_XML(ContentWindow)

ContentWindow::ContentWindow(ContentPtr content, const WindowType type)
    : _uuid(QUuid::createUuid())
    , _type(type)
    , _content(content)
    , _activeHandle(NOHANDLE)
    , _resizePolicy(KEEP_ASPECT_RATIO)
    , _mode(WindowMode::STANDARD)
    , _windowState(NONE)
    , _selected(false)
{
    assert(content);
    _init();
    _coordinates.setSize(content->getDimensions());
}

ContentWindow::ContentWindow()
    : _uuid(QUuid::createUuid())
    , _type(WindowType::DEFAULT)
    , _activeHandle(NOHANDLE)
    , _resizePolicy(KEEP_ASPECT_RATIO)
    , _mode(WindowMode::STANDARD)
    , _windowState(NONE)
    , _selected(false)
{
}

ContentWindow::~ContentWindow()
{
}

const QUuid& ContentWindow::getID() const
{
    return _uuid;
}

bool ContentWindow::isPanel() const
{
    return _type == WindowType::PANEL;
}

Content* ContentWindow::getContentPtr() const
{
    return _content.get();
}

ContentPtr ContentWindow::getContent() const
{
    return _content;
}

void ContentWindow::setContent(ContentPtr content)
{
    assert(content);

    if (_content)
        _content->disconnect(this, SIGNAL(contentModified()));

    content->moveToThread(thread());
    _content = content;
    _init();
}

void ContentWindow::setCoordinates(const QRectF& coordinates)
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

ContentWindow::ResizeHandle ContentWindow::getActiveHandle() const
{
    return _activeHandle;
}

void ContentWindow::setActiveHandle(const ContentWindow::ResizeHandle handle)
{
    if (_activeHandle == handle)
        return;

    _activeHandle = handle;
    emit activeHandleChanged();
    emit modified();
}

ContentWindow::ResizePolicy ContentWindow::getResizePolicy() const
{
    return _resizePolicy;
}

bool ContentWindow::setResizePolicy(const ContentWindow::ResizePolicy policy)
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

ContentWindow::WindowState ContentWindow::getState() const
{
    return _windowState;
}

ContentWindow::WindowMode ContentWindow::getMode() const
{
    return _mode;
}

void ContentWindow::setMode(const ContentWindow::WindowMode mode)
{
    if (mode == _mode)
        return;

    _mode = mode;
    emit modeChanged();
    emit modified();
}

bool ContentWindow::isFocused() const
{
    return _mode == WindowMode::FOCUSED;
}

bool ContentWindow::isFullscreen() const
{
    return _mode == WindowMode::FULLSCREEN;
}

const QRectF& ContentWindow::getFocusedCoordinates() const
{
    return _focusedCoordinates;
}

void ContentWindow::setFocusedCoordinates(const QRectF& coordinates)
{
    if (coordinates == _focusedCoordinates)
        return;

    _focusedCoordinates = coordinates;
    emit focusedCoordinatesChanged();
    emit modified();
}

const QRectF& ContentWindow::getFullscreenCoordinates() const
{
    return _fullscreenCoordinates;
}

void ContentWindow::setFullscreenCoordinates(const QRectF& coordinates)
{
    if (coordinates == _fullscreenCoordinates)
        return;

    _fullscreenCoordinates = coordinates;
    emit fullscreenCoordinatesChanged();
    emit modified();
}

const QRectF& ContentWindow::getDisplayCoordinates() const
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

void ContentWindow::setDisplayCoordinates(const QRectF& coordinates)
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

bool ContentWindow::setState(const ContentWindow::WindowState state)
{
    if (_windowState == state)
        return false;

    const ContentWindow::WindowState prevState = _windowState;

    _windowState = state;

    if (prevState == ContentWindow::HIDDEN)
        emit hiddenChanged(false);
    else if (state == ContentWindow::HIDDEN)
        emit hiddenChanged(true);

    emit stateChanged();
    emit modified();
    return true;
}

bool ContentWindow::isMoving() const
{
    return _windowState == MOVING;
}

bool ContentWindow::isResizing() const
{
    return _windowState == RESIZING;
}

bool ContentWindow::isHidden() const
{
    return _windowState == HIDDEN;
}

bool ContentWindow::isSelected() const
{
    return _selected;
}

void ContentWindow::setSelected(const bool value)
{
    if (value == _selected)
        return;

    _selected = value;
    emit selectedChanged();
    emit modified();
}

void ContentWindow::_init()
{
    setResizePolicy(_content->hasFixedAspectRatio() ? KEEP_ASPECT_RATIO
                                                    : ADJUST_CONTENT);
    connect(_content.get(), &Content::modified, this,
            &ContentWindow::contentModified);
}

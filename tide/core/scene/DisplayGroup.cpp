/*********************************************************************/
/* Copyright (c) 2011-2012, The University of Texas at Austin.       */
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Raphael.Dumusc@epfl.ch                   */
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

#include "DisplayGroup.h"

#include "Window.h"
#include "utils/compilerMacros.h"
#include "utils/log.h"

IMPLEMENT_SERIALIZE_FOR_XML(DisplayGroup)

DisplayGroupPtr DisplayGroup::create(const QSizeF& size)
{
    return DisplayGroupPtr{new DisplayGroup{size}};
}

DisplayGroup::DisplayGroup(const QSizeF& size_)
{
    _coordinates.setSize(size_);
}

DisplayGroup::~DisplayGroup()
{
}

void DisplayGroup::add(WindowPtr window)
{
    if (getWindow(window->getID()))
    {
        print_log(LOG_WARN, LOG_GENERAL,
                  "A window with the same id already exists!");
        return;
    }

    const auto empty = isEmpty();

    _windows.push_back(window);
    _watchChanges(*window);

    if (window->isPanel())
    {
        _panels.insert(window);
        emit hasVisiblePanelsChanged();
    }

    if (window->isFocused())
    {
        _focusedWindows.insert(window);
        if (_focusedWindows.size() == 1)
            emit hasFocusedWindowsChanged();
    }

    if (empty && !isEmpty())
        emit isEmptyChanged();
    emit windowAdded(window);
    _sendDisplayGroup();
}

void DisplayGroup::remove(WindowPtr window)
{
    auto it = find(_windows.begin(), _windows.end(), window);
    if (it == _windows.end())
        return;

    if (*it == _fullscreenWindow)
        setFullscreenWindow(WindowPtr());

    if (window->isPanel())
    {
        _panels.erase(window);
        emit hasVisiblePanelsChanged();
    }

    removeFocusedWindow(*it);
    _windows.erase(it);

    // disconnect any existing connections with the window
    disconnect(window.get(), 0, this, 0);

    if (isEmpty())
        emit isEmptyChanged();

    emit windowRemoved(window);

    if (window->isSelected())
        emit selectedUrisChanged();

    _sendDisplayGroup();
}

void DisplayGroup::moveToFront(WindowPtr window)
{
    if (!window || window == _windows.back())
        return;

    auto it = find(_windows.begin(), _windows.end(), window);
    if (it == _windows.end())
        return;

    // move it to end of the list (last item rendered is on top)
    _windows.erase(it);
    _windows.push_back(window);

    emit(windowMovedToFront(window));
    _sendDisplayGroup();
}

bool DisplayGroup::isEmpty() const
{
    return _windows.empty() || _windows.size() == _panels.size();
}

const WindowPtrs& DisplayGroup::getWindows() const
{
    return _windows;
}

WindowPtr DisplayGroup::getWindow(const QUuid& id) const
{
    for (const auto& window : _windows)
    {
        if (window->getID() == id)
            return window;
    }
    return WindowPtr();
}

WindowPtr DisplayGroup::findWindow(const QString& filename) const
{
    for (const auto& window : _windows)
    {
        if (window->getContent().getUri() == filename)
            return window;
    }
    return WindowPtr();
}

void DisplayGroup::replaceWindows(WindowPtrs windows)
{
    clear();

    for (const auto& window : windows)
        add(window);
}

void DisplayGroup::clear()
{
    if (_windows.empty())
        return;

    // Close regular windows but hide panels (instead of removing them)
    WindowPtrs removeSet;
    for (auto window : _windows)
    {
        if (window->isPanel())
            window->setState(Window::HIDDEN);
        else
            removeSet.push_back(window);
    }

    // Do this before remove because removeFocusedWindow() resets
    // the state of the focused windows which interfers with xml session loading
    if (!_focusedWindows.empty())
    {
        _focusedWindows.clear();
        emit hasFocusedWindowsChanged();
    }

    for (auto window : removeSet)
        remove(window);

    emit cleared(removeSet.size());
}

int DisplayGroup::getZindex(const QUuid& id) const
{
    const auto it = std::find_if(_windows.begin(), _windows.end(),
                                 [&id](const auto& window) {
                                     return window->getID() == id;
                                 });
    return it == _windows.end() ? -1 : it - _windows.begin();
}

bool DisplayGroup::hasFocusedWindows() const
{
    return !_focusedWindows.empty();
}

bool DisplayGroup::hasFullscreenWindows() const
{
    return static_cast<bool>(_fullscreenWindow);
}

bool DisplayGroup::hasVisiblePanels() const
{
    for (const auto& window : _panels)
        if (!window->isHidden())
            return true;
    return false;
}

const WindowSet& DisplayGroup::getFocusedWindows() const
{
    return _focusedWindows;
}

void DisplayGroup::addFocusedWindow(WindowPtr window)
{
    if (!_focusedWindows.insert(window).second)
        return;

    window->setMode(Window::WindowMode::FOCUSED);

    if (_focusedWindows.size() == 1)
        emit hasFocusedWindowsChanged();

    _sendDisplayGroup();
}

void DisplayGroup::removeFocusedWindow(WindowPtr window)
{
    if (!_focusedWindows.erase(window))
        return;

    window->setMode(Window::WindowMode::STANDARD);

    if (_focusedWindows.empty())
        emit hasFocusedWindowsChanged();

    _sendDisplayGroup();
}

const WindowSet& DisplayGroup::getPanels() const
{
    return _panels;
}

WindowSet DisplayGroup::getSelectedWindows() const
{
    auto selectedWindows = WindowSet{};

    for (const auto& window : _windows)
        if (window->isSelected() && !window->isPanel())
            selectedWindows.insert(window);

    return selectedWindows;
}

WindowSet DisplayGroup::getFocusableWindows() const
{
    auto focusableWindows = WindowSet{};
    std::copy_if(getWindows().begin(), getWindows().end(),
                 std::inserter(focusableWindows, focusableWindows.end()),
                 [](const WindowPtr& window) { return !window->isPanel(); });
    return focusableWindows;
}

QStringList DisplayGroup::getSelectedUris() const
{
    auto uris = QStringList{};

    for (const auto& window : _windows)
        if (window->isSelected() && !window->isPanel())
            uris.push_back(window->getContent().getUri());

    return uris;
}

TIDE_DISABLE_WARNING_SHADOW
void DisplayGroup::moveToThread(QThread* thread)
{
    QObject::moveToThread(thread);
    for (auto& window : _windows)
        window->moveToThread(thread);
}
TIDE_DISABLE_WARNING_SHADOW_END

Window* DisplayGroup::getFullscreenWindow() const
{
    return _fullscreenWindow.get();
}

void DisplayGroup::setFullscreenWindow(WindowPtr window)
{
    if (_fullscreenWindow == window)
        return;

    _fullscreenWindow = window;
    emit hasFullscreenWindowsChanged();
    _sendDisplayGroup();
}

void DisplayGroup::_sendDisplayGroup()
{
    emit modified(shared_from_this());
}

void DisplayGroup::_watchChanges(Window& window)
{
    connect(&window, &Window::modified, this, &DisplayGroup::_sendDisplayGroup);

    connect(&window, &Window::contentModified, this,
            &DisplayGroup::_sendDisplayGroup);

    if (window.isPanel())
        connect(&window, &Window::hiddenChanged, this,
                &DisplayGroup::hasVisiblePanelsChanged);
    else
        connect(&window, &Window::selectedChanged, this,
                &DisplayGroup::selectedUrisChanged);
}

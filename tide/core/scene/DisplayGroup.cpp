/*********************************************************************/
/* Copyright (c) 2011-2012, The University of Texas at Austin.       */
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
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

#include "ContentWindow.h"
#include "log.h"

IMPLEMENT_SERIALIZE_FOR_XML(DisplayGroup)

DisplayGroup::DisplayGroup()
{
}

DisplayGroup::DisplayGroup(const QSizeF& size_)
{
    _coordinates.setSize(size_);
}

DisplayGroup::~DisplayGroup()
{
}

void DisplayGroup::addContentWindow(ContentWindowPtr window)
{
    for (ContentWindowPtr existingWindow : _contentWindows)
    {
        if (window->getID() == existingWindow->getID())
        {
            print_log(LOG_WARN, LOG_GENERAL,
                      "A window with the same id already exists!");
            return;
        }
    }

    _contentWindows.push_back(window);
    _watchChanges(*window);

    if (window->isPanel())
    {
        _panels.insert(window);
        emit hasVisiblePanelsChanged();
    }

    emit(contentWindowAdded(window));
    _sendDisplayGroup();
}

void DisplayGroup::removeContentWindow(ContentWindowPtr window)
{
    auto it = find(_contentWindows.begin(), _contentWindows.end(), window);
    if (it == _contentWindows.end())
        return;

    if (*it == _fullscreenWindow)
        setFullscreenWindow(ContentWindowPtr());

    if (window->isPanel())
    {
        _panels.erase(window);
        emit hasVisiblePanelsChanged();
    }

    removeFocusedWindow(*it);
    _contentWindows.erase(it);

    // disconnect any existing connections with the window
    disconnect(window.get(), 0, this, 0);

    emit(contentWindowRemoved(window));
    _sendDisplayGroup();
}

void DisplayGroup::moveToFront(ContentWindowPtr window)
{
    if (!window || window == _contentWindows.back())
        return;

    auto it = find(_contentWindows.begin(), _contentWindows.end(), window);
    if (it == _contentWindows.end())
        return;

    // move it to end of the list (last item rendered is on top)
    _contentWindows.erase(it);
    _contentWindows.push_back(window);

    emit(contentWindowMovedToFront(window));
    _sendDisplayGroup();
}

bool DisplayGroup::isEmpty() const
{
    return _contentWindows.empty();
}

const ContentWindowPtrs& DisplayGroup::getContentWindows() const
{
    return _contentWindows;
}

ContentWindowPtr DisplayGroup::getContentWindow(const QUuid& id) const
{
    for (ContentWindowPtr window : _contentWindows)
    {
        if (window->getID() == id)
            return window;
    }
    return ContentWindowPtr();
}

void DisplayGroup::setContentWindows(ContentWindowPtrs windows)
{
    clear();

    for (const auto& window : windows)
    {
        addContentWindow(window);
        if (window->isFocused())
            _focusedWindows.insert(window);
        if (window->isPanel())
            _panels.insert(window);
    }
    if (!_focusedWindows.empty())
        emit hasFocusedWindowsChanged();
    if (!_panels.empty())
        emit hasVisiblePanelsChanged();

    _sendDisplayGroup();
}

void DisplayGroup::clear()
{
    if (_contentWindows.empty())
        return;

    // Close regular windows but hide panels (instead of removing them)
    ContentWindowPtrs removeSet;
    for (auto window : _contentWindows)
    {
        if (window->isPanel())
            window->setState(ContentWindow::HIDDEN);
        else
            removeSet.push_back(window);
    }

    print_log(LOG_INFO, LOG_CONTENT, "removing %i windows", removeSet.size());

    // Do this before removeContentWindow because removeFocusedWindow() resets
    // the state of the focused windows which interfers with xml session loading
    if (!_focusedWindows.empty())
    {
        _focusedWindows.clear();
        emit hasFocusedWindowsChanged();
    }

    for (auto window : removeSet)
        removeContentWindow(window);
}

int DisplayGroup::getZindex(ContentWindowPtr window) const
{
    const auto it =
        std::find(_contentWindows.begin(), _contentWindows.end(), window);
    return it == _contentWindows.end() ? -1 : it - _contentWindows.begin();
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

const ContentWindowSet& DisplayGroup::getFocusedWindows() const
{
    return _focusedWindows;
}

void DisplayGroup::addFocusedWindow(ContentWindowPtr window)
{
    if (!_focusedWindows.insert(window).second)
        return;

    window->setMode(ContentWindow::WindowMode::FOCUSED);

    if (_focusedWindows.size() == 1)
        emit hasFocusedWindowsChanged();

    _sendDisplayGroup();
}

void DisplayGroup::removeFocusedWindow(ContentWindowPtr window)
{
    if (!_focusedWindows.erase(window))
        return;

    window->setMode(ContentWindow::WindowMode::STANDARD);

    if (_focusedWindows.empty())
        emit hasFocusedWindowsChanged();

    _sendDisplayGroup();
}

const ContentWindowSet& DisplayGroup::getPanels() const
{
    return _panels;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
void DisplayGroup::moveToThread(QThread* thread)
{
    QObject::moveToThread(thread);
    for (auto& window : _contentWindows)
        window->moveToThread(thread);
}
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

ContentWindow* DisplayGroup::getFullscreenWindow() const
{
    return _fullscreenWindow.get();
}

void DisplayGroup::setFullscreenWindow(ContentWindowPtr window)
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

void DisplayGroup::_watchChanges(ContentWindow& window)
{
    connect(&window, &ContentWindow::modified, this,
            &DisplayGroup::_sendDisplayGroup);

    connect(&window, &ContentWindow::contentModified, this,
            &DisplayGroup::_sendDisplayGroup);

    if (window.isPanel())
        connect(&window, &ContentWindow::hiddenChanged, this,
                &DisplayGroup::hasVisiblePanelsChanged);
}

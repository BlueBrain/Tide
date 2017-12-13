/*********************************************************************/
/* Copyright (c) 2014-2017, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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

#include "DisplayGroupRenderer.h"

#include "DataProvider.h"
#include "VisibilityHelper.h"
#include "geometry.h"
#include "qmlUtils.h"
#include "scene/DisplayGroup.h"

namespace
{
const QUrl QML_DISPLAYGROUP_URL("qrc:/qml/core/DisplayGroup.qml");
}

DisplayGroupRenderer::DisplayGroupRenderer(const WallRenderContext& context,
                                           QQuickItem& parentItem)
    : _context{context}
    , _qmlContext{*_context.engine.rootContext()}
    , _displayGroup{new DisplayGroup(QSize())}
{
    _qmlContext.setContextProperty("displaygroup", _displayGroup.get());
    _createDisplayGroupQmlItem(parentItem);
}

void DisplayGroupRenderer::setDisplayGroup(DisplayGroupPtr displayGroup)
{
    // Update the scene with the new information
    _qmlContext.setContextProperty("displaygroup", displayGroup.get());

    // Update windows, creating new ones if needed
    QSet<QUuid> updatedWindows;
    const QQuickItem* parentItem = nullptr;
    const VisibilityHelper helper(*displayGroup, _context.screenRect);
    for (const auto& window : displayGroup->getContentWindows())
    {
        const auto& id = window->getID();

        updatedWindows.insert(id);

        if (!_windowItems.contains(id))
            _createWindowQmlItem(window);

        _windowItems[id]->update(window, helper.getVisibleArea(*window));

        // Update stacking order
        auto quickItem = _windowItems[id]->getQuickItem();
        if (parentItem)
            quickItem->stackAfter(parentItem);
        parentItem = quickItem;
    }

    // Remove old windows
    auto it = _windowItems.begin();
    while (it != _windowItems.end())
    {
        if (updatedWindows.contains(it.key()))
            ++it;
        else
            it = _windowItems.erase(it);
    }

    // Retain the new DisplayGroup
    _displayGroup = displayGroup;

    // Work around a bug in animation in Qt, where the opacity property
    // of the focus context may not always be restored to its original value.
    // See JIRA issue: DISCL-305
    if (!displayGroup->hasFocusedWindows() &&
        !displayGroup->hasFullscreenWindows() &&
        !displayGroup->hasVisiblePanels())
    {
        for (auto child : _displayGroupItem->childItems())
        {
            if (child->objectName() == "focuscontext")
                child->setProperty("opacity", 0.0);
        }
    }
}

void DisplayGroupRenderer::_createDisplayGroupQmlItem(QQuickItem& parentItem)
{
    _displayGroupItem = qml::makeItem(_context.engine, QML_DISPLAYGROUP_URL);
    _displayGroupItem->setParentItem(&parentItem);
}

void DisplayGroupRenderer::_createWindowQmlItem(ContentWindowPtr window)
{
    const auto& id = window->getID();
    auto sync = _context.provider.createSynchronizer(*window, _context.view);
    _windowItems[id].reset(new QmlWindowRenderer(std::move(sync),
                                                 std::move(window),
                                                 *_displayGroupItem,
                                                 _context.engine.rootContext()));
}

/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#include "MasterDisplayGroupRenderer.h"

#include "control/ContentController.h"
#include "control/DisplayGroupController.h"
#include "control/WindowResizeHandlesController.h"
#include "control/WindowTouchController.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"
#include "utils/qml.h"

#include <QQmlContext>
#include <QQmlEngine>

namespace
{
const QUrl QML_DISPLAYGROUP_URL("qrc:/qml/core/DisplayGroup.qml");
const QUrl QML_WINDOW_URL("qrc:/qml/master/MasterWindow.qml");
}

MasterDisplayGroupRenderer::MasterDisplayGroupRenderer(DisplayGroupPtr group,
                                                       QQmlEngine& engine,
                                                       QQuickItem& parentItem)
    : _displayGroup{group}
    , _groupController{new DisplayGroupController{*_displayGroup}}
    , _engine{engine}
    , _qmlContext{new QQmlContext(engine.rootContext())}
{
    _setContextProperties();
    _createQmlItem(parentItem);
    _addWindows();
    _watchDisplayGroupUpdates();
}

MasterDisplayGroupRenderer::~MasterDisplayGroupRenderer()
{
    _displayGroup->disconnect(this); // prevent race conditions
}

void MasterDisplayGroupRenderer::_setContextProperties()
{
    _qmlContext->setContextProperty("displaygroup", _displayGroup.get());
    _qmlContext->setContextProperty("groupcontroller", _groupController.get());
}

void MasterDisplayGroupRenderer::_createQmlItem(QQuickItem& parentItem)
{
    _displayGroupItem =
        qml::makeItem(_engine, QML_DISPLAYGROUP_URL, _qmlContext.get());
    _displayGroupItem->setParentItem(&parentItem);
}

void MasterDisplayGroupRenderer::_addWindows()
{
    for (const auto& window : _displayGroup->getWindows())
        _add(window);
}

void MasterDisplayGroupRenderer::_watchDisplayGroupUpdates()
{
    connect(_displayGroup.get(), &DisplayGroup::windowAdded, this,
            &MasterDisplayGroupRenderer::_add);
    connect(_displayGroup.get(), &DisplayGroup::windowRemoved, this,
            &MasterDisplayGroupRenderer::_remove);
    connect(_displayGroup.get(), &DisplayGroup::windowMovedToFront, this,
            &MasterDisplayGroupRenderer::_moveToFront);
}

void MasterDisplayGroupRenderer::_add(WindowPtr window)
{
    // New Context for the window, ownership retained by the windowItem
    auto windowContext = new QQmlContext(_qmlContext.get());
    windowContext->setContextProperty("window", window.get());

    auto controller = new WindowResizeHandlesController(*window, *_displayGroup);
    controller->setParent(windowContext);
    windowContext->setContextProperty("resizehandlescontroller", controller);

    auto touchController = new WindowTouchController(*window, *_displayGroup);
    touchController->setParent(windowContext);
    windowContext->setContextProperty("touchcontroller", touchController);

    auto contentController = ContentController::create(*window).release();
    contentController->setParent(windowContext);
    windowContext->setContextProperty("contentcontroller", contentController);

    auto windowItem = qml::makeItem(_engine, QML_WINDOW_URL, windowContext);
    windowContext->setParent(windowItem.get());

    windowItem->setParentItem(_displayGroupItem.get());

    const auto& id = window->getID();
    _uuidToWindowMap.emplace(id, std::move(windowItem));
}

void MasterDisplayGroupRenderer::_remove(WindowPtr window)
{
    _uuidToWindowMap.erase(window->getID());
}

void MasterDisplayGroupRenderer::_moveToFront(WindowPtr window)
{
    const auto& id = window->getID();
    auto itemToRaise = _uuidToWindowMap.find(id);
    if (itemToRaise == _uuidToWindowMap.end())
        return;

    itemToRaise->second->stackAfter(_displayGroupItem->childItems().last());
}

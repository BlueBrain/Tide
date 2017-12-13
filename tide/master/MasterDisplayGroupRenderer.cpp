/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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
#include "control/ContentWindowController.h"
#include "control/DisplayGroupController.h"
#include "qmlUtils.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"

#include <QQmlContext>
#include <QQmlEngine>

namespace
{
const QUrl QML_CONTENTWINDOW_URL("qrc:/qml/master/MasterContentWindow.qml");
const QUrl QML_DISPLAYGROUP_URL("qrc:/qml/master/MasterDisplayGroup.qml");
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
    _displayGroup->disconnect(this);

    if (_displayGroupItem)
    {
        _displayGroupItem->setParentItem(nullptr);
        delete _displayGroupItem;
        _displayGroupItem = nullptr;
    }
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
    for (const auto& window : _displayGroup->getContentWindows())
        _add(window);
}

void MasterDisplayGroupRenderer::_watchDisplayGroupUpdates()
{
    connect(_displayGroup.get(), &DisplayGroup::contentWindowAdded, this,
            &MasterDisplayGroupRenderer::_add);
    connect(_displayGroup.get(), &DisplayGroup::contentWindowRemoved, this,
            &MasterDisplayGroupRenderer::_remove);
    connect(_displayGroup.get(), &DisplayGroup::contentWindowMovedToFront, this,
            &MasterDisplayGroupRenderer::_moveToFront);
}

void MasterDisplayGroupRenderer::_add(ContentWindowPtr window)
{
    // New Context for the window, ownership retained by the windowItem
    auto windowContext = new QQmlContext(_qmlContext.get());
    windowContext->setContextProperty("contentwindow", window.get());

    auto controller = new ContentWindowController(*window, *_displayGroup);
    controller->setParent(windowContext);
    windowContext->setContextProperty("controller", controller);

    auto contentController = ContentController::create(*window).release();
    contentController->setParent(windowContext);
    windowContext->setContextProperty("contentcontroller", contentController);

    auto windowItem =
        qml::makeItem(_engine, QML_CONTENTWINDOW_URL, windowContext);
    windowContext->setParent(windowItem);

    // Store a reference to the window and add it to the scene
    const auto& id = window->getID();
    _uuidToWindowMap[id] = qobject_cast<QQuickItem*>(windowItem);
    _uuidToWindowMap[id]->setParentItem(_displayGroupItem);
}

void MasterDisplayGroupRenderer::_remove(ContentWindowPtr contentWindow)
{
    const auto& id = contentWindow->getID();
    if (!_uuidToWindowMap.contains(id))
        return;

    QQuickItem* itemToRemove = _uuidToWindowMap[id];
    _uuidToWindowMap.remove(id);
    delete itemToRemove;
}

void MasterDisplayGroupRenderer::_moveToFront(ContentWindowPtr contentWindow)
{
    const auto& id = contentWindow->getID();
    if (!_uuidToWindowMap.contains(id))
        return;

    QQuickItem* itemToRaise = _uuidToWindowMap[id];
    itemToRaise->stackAfter(_displayGroupItem->childItems().last());
}

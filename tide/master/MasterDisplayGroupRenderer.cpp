/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>

namespace
{
const QUrl QML_CONTENTWINDOW_URL("qrc:/qml/master/MasterContentWindow.qml");
const QUrl QML_DISPLAYGROUP_URL("qrc:/qml/master/MasterDisplayGroup.qml");
}

MasterDisplayGroupRenderer::MasterDisplayGroupRenderer(DisplayGroupPtr group,
                                                       QQmlEngine* engine,
                                                       QQuickItem* parentItem)
    : _displayGroup{group}
    , _engine{engine}
    , _parentItem{parentItem}
{
    auto rootContext = _engine->rootContext();
    rootContext->setContextProperty("displaygroup", _displayGroup.get());

    auto controller = make_unique<DisplayGroupController>(*_displayGroup);
    rootContext->setContextProperty("groupcontroller", controller.get());

    QQmlComponent component(_engine, QML_DISPLAYGROUP_URL);
    qmlCheckOrThrow(component);
    _displayGroupItem = qobject_cast<QQuickItem*>(component.create());
    _displayGroupItem->setParentItem(_parentItem);

    // Transfer ownership of the controller to qml item
    controller->setParent(_displayGroupItem);
    controller.release();

    connect(_displayGroupItem, SIGNAL(openLauncher()), this,
            SIGNAL(openLauncher()));

    const auto contentWindows = _displayGroup->getContentWindows();
    for (const auto& contentWindow : contentWindows)
        _add(contentWindow);

    connect(_displayGroup.get(), &DisplayGroup::contentWindowAdded, this,
            &MasterDisplayGroupRenderer::_add);
    connect(_displayGroup.get(), &DisplayGroup::contentWindowRemoved, this,
            &MasterDisplayGroupRenderer::_remove);
    connect(_displayGroup.get(), &DisplayGroup::contentWindowMovedToFront, this,
            &MasterDisplayGroupRenderer::_moveToFront);
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

void MasterDisplayGroupRenderer::_add(ContentWindowPtr window)
{
    // New Context for the window, ownership retained by the windowItem
    QQmlContext* windowContext = new QQmlContext(_engine->rootContext());
    windowContext->setContextProperty("contentwindow", window.get());

    auto controller = new ContentWindowController(*window, *_displayGroup);
    controller->setParent(windowContext);
    windowContext->setContextProperty("controller", controller);

    auto contentController = ContentController::create(*window).release();
    contentController->setParent(windowContext);
    windowContext->setContextProperty("contentcontroller", contentController);

    QQmlComponent component(_engine, QML_CONTENTWINDOW_URL);
    qmlCheckOrThrow(component);
    QObject* windowItem = component.create(windowContext);
    windowContext->setParent(windowItem);

    // Store a reference to the window and add it to the scene
    const QUuid& id = window->getID();
    _uuidToWindowMap[id] = qobject_cast<QQuickItem*>(windowItem);
    _uuidToWindowMap[id]->setParentItem(_displayGroupItem);
}

void MasterDisplayGroupRenderer::_remove(ContentWindowPtr contentWindow)
{
    const QUuid& id = contentWindow->getID();
    if (!_uuidToWindowMap.contains(id))
        return;

    QQuickItem* itemToRemove = _uuidToWindowMap[id];
    _uuidToWindowMap.remove(id);
    delete itemToRemove;
}

void MasterDisplayGroupRenderer::_moveToFront(ContentWindowPtr contentWindow)
{
    const QUuid& id = contentWindow->getID();
    if (!_uuidToWindowMap.contains(id))
        return;

    QQuickItem* itemToRaise = _uuidToWindowMap[id];
    itemToRaise->stackAfter(_displayGroupItem->childItems().last());
}

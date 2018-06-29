/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#include "MasterSurfaceRenderer.h"

#include "MasterDisplayGroupRenderer.h"
#include "control/DisplayGroupController.h"
#include "qmlUtils.h"
#include "scene/Scene.h"

#include <QQmlContext>
#include <QQmlEngine>

namespace
{
const QUrl QML_CONTROL_SURFACE_URL("qrc:/qml/master/MasterControlSurface.qml");
const QUrl QML_BASIC_SURFACE_URL("qrc:/qml/core/BasicSurface.qml");
}

MasterSurfaceRenderer::MasterSurfaceRenderer(Surface& surface,
                                             QQmlEngine& engine,
                                             QQuickItem& parentItem)
    : _surface{surface}
    , _group{surface.getGroupPtr()}
    , _groupController{new DisplayGroupController{*_group}}
{
    _setContextProperties(*engine.rootContext());

    if (surface.getIndex() == 0)
    {
        _setControlSurfaceContextProperties(*engine.rootContext());
        _createControlSurfaceItem(engine);
    }
    else
        _createBasicSurfaceItem(engine);

    _surfaceItem->setParentItem(&parentItem);
    _createGroupRenderer(engine);
}

MasterSurfaceRenderer::~MasterSurfaceRenderer()
{
}

void MasterSurfaceRenderer::_setContextProperties(QQmlContext& context)
{
    context.setContextProperty("contextmenu", &_surface.getContextMenu());
}

void MasterSurfaceRenderer::_setControlSurfaceContextProperties(
    QQmlContext& context)
{
    // SideControl buttons need a displaygroup and groupcontroller
    context.setContextProperty("displaygroup", _group.get());
    context.setContextProperty("groupcontroller", _groupController.get());
}

void MasterSurfaceRenderer::_createControlSurfaceItem(QQmlEngine& engine)
{
    _surfaceItem = qml::makeItem(engine, QML_CONTROL_SURFACE_URL);
    connect(_surfaceItem.get(), SIGNAL(openLauncher()), this,
            SIGNAL(openLauncher()));
}

void MasterSurfaceRenderer::_createBasicSurfaceItem(QQmlEngine& engine)
{
    _surfaceItem = qml::makeItem(engine, QML_BASIC_SURFACE_URL);
}

void MasterSurfaceRenderer::_createGroupRenderer(QQmlEngine& engine)
{
    _displayGroupRenderer =
        std::make_unique<MasterDisplayGroupRenderer>(_group, engine,
                                                     *_surfaceItem);
}

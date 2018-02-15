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

#include "MasterQuickView.h"

#include "configuration/Configuration.h"
#include "scene/Options.h"
#include "scene/ScreenLock.h"

#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>

namespace
{
const QUrl QML_ROOT_COMPONENT("qrc:/qml/master/Root.qml");
const QString WALL_OBJECT_NAME("Wall");
}

MasterQuickView::MasterQuickView(OptionsPtr options, ScreenLockPtr lock,
                                 const Configuration& config)
{
    setResizeMode(QQuickView::SizeRootObjectToView);

    rootContext()->setContextProperty("options", options.get());
    rootContext()->setContextProperty("lock", lock.get());
    rootContext()->setContextProperty("view", this);

    setSource(QML_ROOT_COMPONENT);

    const auto& surface = config.surfaces[0];

    _wallItem = rootObject()->findChild<QQuickItem*>(WALL_OBJECT_NAME);
    _wallItem->setProperty("numberOfTilesX", surface.screenCountX);
    _wallItem->setProperty("numberOfTilesY", surface.screenCountY);
    _wallItem->setProperty("bezelWidth", surface.bezelWidth);
    _wallItem->setProperty("bezelHeight", surface.bezelHeight);
    _wallItem->setProperty("screenWidth", surface.getScreenWidth());
    _wallItem->setProperty("screenHeight", surface.getScreenHeight());
    _wallItem->setProperty("wallWidth", surface.getTotalWidth());
    _wallItem->setProperty("wallHeight", surface.getTotalHeight());
}

MasterQuickView::~MasterQuickView()
{
}

QQuickItem* MasterQuickView::wallItem()
{
    return _wallItem;
}

QPointF MasterQuickView::mapToWallPos(const QPointF& normalizedPos) const
{
    const auto scale = QQmlProperty::read(_wallItem, "scale").toFloat();
    const auto offsetX = QQmlProperty::read(_wallItem, "offsetX").toFloat();
    const auto offsetY = QQmlProperty::read(_wallItem, "offsetY").toFloat();

    const auto screenPosX =
        normalizedPos.x() * _wallItem->width() * scale + offsetX;
    const auto screenPosY =
        normalizedPos.y() * _wallItem->height() * scale + offsetY;
    return {screenPosX, screenPosY};
}

bool MasterQuickView::event(QEvent* evt)
{
    switch (evt->type())
    {
    case QEvent::KeyPress:
    {
        QKeyEvent* k = static_cast<QKeyEvent*>(evt);

        // Override default behaviour to process TAB key events
        QQuickView::keyPressEvent(k);

        if (k->key() == Qt::Key_Backtab || k->key() == Qt::Key_Tab ||
            (k->key() == Qt::Key_Tab && (k->modifiers() & Qt::ShiftModifier)))
        {
            evt->accept();
        }
        return true;
    }
    case QEvent::MouseButtonPress:
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        if (e->button() == Qt::LeftButton)
            emit mousePressed(_wallItem->mapFromScene(e->localPos()));
        break;
    }
    case QEvent::MouseMove:
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        if (e->buttons() & Qt::LeftButton)
            emit mouseMoved(_wallItem->mapFromScene(e->localPos()));
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        if (e->button() == Qt::LeftButton)
            emit mouseReleased(_wallItem->mapFromScene(e->localPos()));
        break;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
    {
        _mapTouchEvent(static_cast<QTouchEvent*>(evt));
    }
    default:
        break;
    }
    return QQuickView::event(evt);
}

void MasterQuickView::_mapTouchEvent(QTouchEvent* event)
{
    for (const auto& point : event->touchPoints())
    {
        auto& p = const_cast<QTouchEvent::TouchPoint&>(point);
        p.setPos(mapToWallPos(point.normalizedPos()));
    }
}

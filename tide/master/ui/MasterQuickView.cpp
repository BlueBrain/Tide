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

#include "MasterQuickView.h"

#include "MasterConfiguration.h"
#include "ScreenLock.h"
#include "scene/Options.h"

#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>

namespace
{
const QUrl QML_ROOT_COMPONENT("qrc:/qml/master/Root.qml");
const QString WALL_OBJECT_NAME("Wall");
}

MasterQuickView::MasterQuickView(OptionsPtr options, ScreenLockPtr lock,
                                 const MasterConfiguration& config)
{
    setResizeMode(QQuickView::SizeRootObjectToView);

    rootContext()->setContextProperty("options", options.get());
    rootContext()->setContextProperty("view", this);
    rootContext()->setContextProperty("lock", lock.get());

    setSource(QML_ROOT_COMPONENT);

    _wallItem = rootObject()->findChild<QQuickItem*>(WALL_OBJECT_NAME);
    _wallItem->setProperty("numberOfTilesX", config.getTotalScreenCountX());
    _wallItem->setProperty("numberOfTilesY", config.getTotalScreenCountY());
    _wallItem->setProperty("mullionWidth", config.getMullionWidth());
    _wallItem->setProperty("mullionHeight", config.getMullionHeight());
    _wallItem->setProperty("screenWidth", config.getScreenWidth());
    _wallItem->setProperty("screenHeight", config.getScreenHeight());
    _wallItem->setProperty("wallWidth", config.getTotalWidth());
    _wallItem->setProperty("wallHeight", config.getTotalHeight());
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
    default:
        break;
    }
    return QQuickView::event(evt);
}

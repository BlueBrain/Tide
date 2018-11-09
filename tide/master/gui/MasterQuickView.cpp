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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "MasterQuickView.h"

#include "configuration/SurfaceConfig.h"

#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>

namespace
{
const QUrl QML_ROOT_COMPONENT("qrc:/qml/master/GUIRoot.qml");
const QString SURFACE_OBJECT_NAME("Surface");
}

MasterQuickView::MasterQuickView(const SurfaceConfig& surfaceConfig)
{
    setResizeMode(QQuickView::SizeRootObjectToView);

    rootContext()->setContextProperty("view", this);

    setSource(QML_ROOT_COMPONENT);

    _surfaceItem = rootObject()->findChild<QQuickItem*>(SURFACE_OBJECT_NAME);
    _surfaceItem->setProperty("numberOfTilesX", surfaceConfig.screenCountX);
    _surfaceItem->setProperty("numberOfTilesY", surfaceConfig.screenCountY);
    _surfaceItem->setProperty("bezelWidth", surfaceConfig.bezelWidth);
    _surfaceItem->setProperty("bezelHeight", surfaceConfig.bezelHeight);
    _surfaceItem->setProperty("screenWidth", surfaceConfig.getScreenWidth());
    _surfaceItem->setProperty("screenHeight", surfaceConfig.getScreenHeight());
    _surfaceItem->setProperty("surfaceWidth", surfaceConfig.getTotalWidth());
    _surfaceItem->setProperty("surfaceHeight", surfaceConfig.getTotalHeight());
}

MasterQuickView::~MasterQuickView()
{
}

QQuickItem& MasterQuickView::getSurfaceItem()
{
    return *_surfaceItem;
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
            emit mousePressed(_surfaceItem->mapFromScene(e->localPos()));
        break;
    }
    case QEvent::MouseMove:
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        if (e->buttons() & Qt::LeftButton)
            emit mouseMoved(_surfaceItem->mapFromScene(e->localPos()));
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        if (e->button() == Qt::LeftButton)
            emit mouseReleased(_surfaceItem->mapFromScene(e->localPos()));
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
        p.setPos(_mapToQmlSurfacePos(point.normalizedPos()));
    }
}

QPointF MasterQuickView::_mapToQmlSurfacePos(const QPointF& normalizedPos) const
{
    const auto scale = QQmlProperty::read(_surfaceItem, "scale").toFloat();
    const auto offsetX = QQmlProperty::read(_surfaceItem, "offsetX").toFloat();
    const auto offsetY = QQmlProperty::read(_surfaceItem, "offsetY").toFloat();

    const auto screenPosX =
        normalizedPos.x() * _surfaceItem->width() * scale + offsetX;
    const auto screenPosY =
        normalizedPos.y() * _surfaceItem->height() * scale + offsetY;
    return {screenPosX, screenPosY};
}

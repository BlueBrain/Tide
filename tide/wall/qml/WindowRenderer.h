/*********************************************************************/
/* Copyright (c) 2015-2018, EPFL/Blue Brain Project                  */
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

#ifndef WINDOWRENDERER_H
#define WINDOWRENDERER_H

#include "types.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

/**
 * Provide a Qml representation of a Window on the Wall.
 */
class WindowRenderer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowRenderer)

public:
    /** Constructor. */
    WindowRenderer(std::unique_ptr<ContentSynchronizer> synchronizer,
                   WindowPtr window, QQuickItem& parentItem,
                   QQmlContext* parentContext, bool isBackground = false);
    /** Destructor. */
    ~WindowRenderer();

    /** Update the qml object with a new data model. */
    void update(WindowPtr window, const QRectF& visibleArea);

    /** Get the QML item. */
    QQuickItem* getQuickItem();

private:
    ContentSynchronizerSharedPtr _synchronizer;
    WindowPtr _window;

    std::unique_ptr<QQmlContext> _windowContext;
    std::unique_ptr<QQuickItem> _windowItem;

    std::map<uint, TilePtr> _tiles;
    TilePtr _zoomContextTile;

    void _addTile(TilePtr tile, uint lod);
    QQuickItem* _getZoomContextParentItem() const;
    void _updateZoomContextTile(bool visible);
    void _addZoomContextTile();
    void _removeZoomContextTile();
    void _removeTile(uint tileIndex);
    void _updateTile(uint tileIndex, const QRect& coordinates);
};

#endif

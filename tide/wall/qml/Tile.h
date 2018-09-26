/*********************************************************************/
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project                  */
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

#ifndef TILE_H
#define TILE_H

#include "types.h"

#include "TextureBorderSwitcher.h"

#include <QQuickItem> // parent
#include <memory>     // std::enable_shared_from_this

/**
 * Qml item to render an image tile with texture double-buffering.
 */
class Tile : public QQuickItem, public std::enable_shared_from_this<Tile>
{
    Q_OBJECT
    Q_DISABLE_COPY(Tile)

    Q_PROPERTY(uint id READ getId CONSTANT)
    Q_PROPERTY(bool showBorder READ getShowBorder WRITE setShowBorder NOTIFY
                   showBorderChanged)

public:
    enum SizePolicy
    {
        AdjustToTexture,
        FillParent
    };

    /**
     * Create a shared Tile.
     * @param id the unique identifier for the tile
     * @param rect the nominal coordinates of the tile
     * @param type the type of texture (static/dynamic)
     */
    static TilePtr create(uint id, const QRect& rect,
                          TextureType type = TextureType::static_);

    /** @return the unique identifier for this tile. */
    uint getId() const;

    /** @return true if this tile displays its borders. */
    bool getShowBorder() const;

    /**
     * Request an update of the back texture, resing it if necessary.
     * @param rect the new size for the back texture.
     */
    void update(const QRect& rect);

    /**
     * Set the size policy.
     * @param policy defines how the tile should resize and position itself
     */
    void setSizePolicy(SizePolicy policy);

public slots:
    /**
     * Upload the given image to the back texture.
     *
     * The *self* parameter is here to extend the lifetime of the Tile and
     * prevent an infrequent race condition. This function is called through
     * QMetaObject::invokeMethod with a Qt::QueuedConnection from DataProvider.
     * It is therefore possible that the shared_ptr holding this Tile is
     * destroyed after the function is called by Qt (valid QObject) but before
     * reaching the end of the function.
     *
     * The observed symptom was a bad_weak_ptr exception at
     * emit readyToSwap(shared_from_this());
     */
    void updateBackTexture(ImagePtr image, TilePtr self);

    /** Show a border around the tile (for debugging purposes). */
    void setShowBorder(bool set);

    /** Swap the front and back texture. */
    void swapImage();

signals:
    /** Notifier for the showBorder property. */
    void showBorderChanged();

    /**
     * Request a texture update from the DataProvider.
     *
     * Emitted when the Tile is first added and after each call to update() for
     * dynamic textures.
     */
    void requestNextFrame(TilePtr tile);

    /** Notify that the back texture has been updated and it can be swapped. */
    void readyToSwap(TilePtr tile);

private:
    const uint _tileId = 0;
    TextureType _type = TextureType::static_;
    SizePolicy _policy = AdjustToTexture;

    bool _firstImageUploaded = false;
    QRect _nextCoord;
    TextureBorderSwitcher _textureSwitcher;

    Tile(uint id, const QRect& rect, TextureType type);

    /** Called on the render thread to update the scene graph. */
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) final;
    void _onParentChanged(QQuickItem* newParent);

    QMetaObject::Connection _widthConn;
    QMetaObject::Connection _heightConn;
};

#endif

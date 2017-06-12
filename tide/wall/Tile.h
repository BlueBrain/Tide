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

#include <QQuickItem>
#include <memory> // std::enable_shared_from_this

class QuadLineNode;

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

    enum class TextureType
    {
        Static,
        Dynamic
    };

    /**
     * Constructor
     * @param id the unique identifier for this tile
     * @param rect the nominal size of the tile's texture
     * @param format the texture format to use for rendering
     * @param type the type of texture
     */
    Tile(uint id, const QRect& rect, TextureFormat format = TextureFormat::rgba,
         TextureType type = TextureType::Static);

    /** @return the unique identifier for this tile. */
    uint getId() const;

    /** @return the texture format for this tile. */
    TextureFormat getFormat() const;

    /** @return true if this tile displays its borders. */
    bool getShowBorder() const;

    /**
     * Request an update of the back texture, resing it if necessary.
     * @param rect the new size for the back texture.
     * @param format the new format for the back texture. Changing between RGBA
     *        and YUV texture formats is not allowed.
     */
    void update(const QRect& rect, TextureFormat format);

    /**
     * Set the size policy.
     * @param policy defines how the tile should resize and position itself
     */
    void setSizePolicy(SizePolicy policy);

public slots:
    /** Upload the given image to the back texture. */
    void updateBackTexture(ImagePtr image);

    /** Show a border around the tile (for debugging purposes). */
    void setShowBorder(bool set);

    /** Swap the front and back texture. */
    void swapImage();

signals:
    /** Notifier for the showBorder property. */
    void showBorderChanged();

    /**
     * Notifies that the texture is ready to be updated.
     * It is emitted after the texture has been created or after a call to
     * update() for dynamic textures.
     */
    void requestNextFrame(TilePtr tile);

    /** Notify that the back texture has been updated and it can be swapped. */
    void readyToSwap(TilePtr tile);

protected:
    /** Called on the render thread to update the scene graph. */
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private:
    const uint _tileId;
    TextureFormat _format;
    TextureType _type;
    SizePolicy _policy;

    bool _swapRequested = false;
    QRect _nextCoord;
    ImagePtr _image;
    bool _firstImageUploaded = false;

    bool _showBorder = false;
    QuadLineNode* _border = nullptr; // Child QObject

    template <class NodeT>
    QSGNode* _updateTextureNode(QSGNode* oldNode);

    template <class NodeT>
    void _updateBorderNode(NodeT* parentNode);

    void _onParentChanged(QQuickItem* newParent);

    QMetaObject::Connection _widthConn;
    QMetaObject::Connection _heightConn;
};

#endif

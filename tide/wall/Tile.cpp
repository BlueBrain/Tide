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

#include "Tile.h"

#include "TextureNodeFactory.h"
#include "log.h"

#include <QSGNode>

TilePtr Tile::create(const uint id, const QRect& rect, const TextureType type)
{
    return TilePtr{new Tile{id, rect, type}};
}

// false-positive on qt signals for Q_PROPERTY notifiers
// cppcheck-suppress uninitMemberVar
Tile::Tile(const uint id, const QRect& rect, const TextureType type)
    : _tileId(id)
    , _type(type)
    , _nextCoord(rect)
{
    setFlag(ItemHasContents, true);
    setVisible(false);

    connect(this, &QQuickItem::parentChanged, this, &Tile::_onParentChanged);
}

uint Tile::getId() const
{
    return _tileId;
}

bool Tile::getShowBorder() const
{
    return _textureSwitcher.showBorder;
}

void Tile::setShowBorder(const bool set)
{
    if (_textureSwitcher.showBorder == set)
        return;

    _textureSwitcher.showBorder = set;
    emit showBorderChanged();
    QQuickItem::update();
}

void Tile::update(const QRect& rect)
{
    _nextCoord = rect;

    if (_type == TextureType::Dynamic)
        emit requestNextFrame(shared_from_this());

    QQuickItem::update();
}

void Tile::updateBackTexture(ImagePtr image)
{
    if (!image)
    {
        print_log(LOG_WARN, LOG_GENERAL, "Invalid image");
        return;
    }

    if (_type == TextureType::Static && _firstImageUploaded)
        throw std::logic_error("Static tiles can't be updated");

    _textureSwitcher.setNextImage(image);
    _firstImageUploaded = true;

    // Note: readToSwap() must happen immediately and not in _updateTextureNode,
    // otherwise tiles don't update faster than 30 fps. The reason for this
    // could not be established. However, it was verified (using glFenceSync)
    // that textures always upload faster than they are generated in practice.
    // There is no need to check for upload completion in _updateTextureNode.
    emit readyToSwap(shared_from_this());

    QQuickItem::update();
}

void Tile::setSizePolicy(const SizePolicy policy)
{
    _policy = policy;
}

void Tile::swapImage()
{
    _textureSwitcher.requestSwap();

    if (!isVisible())
        setVisible(true);

    if (_policy == SizePolicy::AdjustToTexture)
    {
        setPosition(_nextCoord.topLeft());
        setSize(_nextCoord.size());
    }

    QQuickItem::update();
}

QSGNode* Tile::updatePaintNode(QSGNode* node, QQuickItem::UpdatePaintNodeData*)
{
    auto textureNode =
        std::unique_ptr<TextureNode>(dynamic_cast<TextureNode*>(node));

    TextureNodeFactoryImpl factory{*window(), _type};
    _textureSwitcher.update(textureNode, factory);
    if (!textureNode)
        return nullptr;

    textureNode->setCoord(boundingRect());

    _textureSwitcher.updateBorderNode(*textureNode);

    return dynamic_cast<QSGNode*>(textureNode.release());
}

void Tile::_onParentChanged(QQuickItem* newParent)
{
    if (!newParent)
    {
        disconnect(_widthConn);
        disconnect(_heightConn);
        return;
    }

    if (_policy != FillParent)
        return;

    setWidth(newParent->width());
    setHeight(newParent->height());

    _widthConn = connect(newParent, &QQuickItem::widthChanged,
                         [this]() { setWidth(parentItem()->width()); });
    _heightConn = connect(newParent, &QQuickItem::heightChanged,
                          [this]() { setHeight(parentItem()->height()); });
}

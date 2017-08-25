/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#include "TextureSwitcher.h"

#include "data/Image.h"

void TextureSwitcher::setNextImage(ImagePtr image)
{
    if (!image)
        throw std::invalid_argument("empty image");

    _image = image;
}

void TextureSwitcher::requestSwap()
{
    _swapRequested = true;
}

void TextureSwitcher::update(std::unique_ptr<TextureNode>& node,
                             TextureNodeFactory& factory)
{
    if (!node) // initial call
    {
        if (_image)
            node = factory.create(_image->getFormat());
        else
            return;
    }

    if (_image)
    {
        if (_needToChangeNextNode(factory))
            _createNextNode(factory);

        _uploadImage(_nextNode ? *_nextNode : *node);
    }

    if (_canSwap())
        _swap(node);
}

bool TextureSwitcher::_canSwap() const
{
    return _swapRequested && _swapPossible;
}

void TextureSwitcher::_swap(std::unique_ptr<TextureNode>& node)
{
    if (_nextNode)
    {
        aboutToSwitch(*node, *_nextNode);
        node = std::move(_nextNode);
    }

    node->swap();
    _swapRequested = false;
    _swapPossible = false;
}

bool TextureSwitcher::_needToChangeNextNode(const TextureNodeFactory& factory)
{
    return factory.needToChangeNodeType(_format, _image->getFormat());
}

void TextureSwitcher::_createNextNode(TextureNodeFactory& factory)
{
    _nextNode = factory.create(_image->getFormat());
}

void TextureSwitcher::_uploadImage(TextureNode& node)
{
    node.updateBackTexture(*_image);
    _format = _image->getFormat();
    _image.reset();
    _swapPossible = true;
}

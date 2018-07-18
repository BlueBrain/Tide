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

#include "TextureBorderSwitcher.h"

#include "QuadLineNode.h"

#include <QColor>
#include <QSGNode>

namespace
{
const qreal borderWidth = 10.0;
const QColor borderColor("lightgreen");

QuadLineNode* _makeBorder(const QRectF& coordinates)
{
    return new QuadLineNode(coordinates, borderWidth, borderColor);
}
}

void TextureBorderSwitcher::updateBorderNode(TextureNode& textureNode)
{
    if (showBorder)
    {
        if (_border)
            _border->setRect(textureNode.getCoord());
        else
        {
            _border = _makeBorder(textureNode.getCoord());
            _addBorder(textureNode);
        }
    }
    else if (_border)
    {
        _removeBorder(textureNode);
        delete _border;
        _border = nullptr;
    }
}

void TextureBorderSwitcher::aboutToSwitch(TextureNode& oldNode,
                                          TextureNode& newNode)
{
    if (!_border)
        return;

    _removeBorder(oldNode);
    _addBorder(newNode);
}

void TextureBorderSwitcher::_addBorder(TextureNode& node)
{
    dynamic_cast<QSGNode&>(node).appendChildNode(_border);
}

void TextureBorderSwitcher::_removeBorder(TextureNode& node)
{
    dynamic_cast<QSGNode&>(node).removeChildNode(_border);
}

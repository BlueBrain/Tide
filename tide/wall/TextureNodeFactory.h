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

#ifndef TEXTURENODEFACTORY_H
#define TEXTURENODEFACTORY_H

#include "TextureNode.h"

class QQuickWindow;

/**
 * Abstract TextureNode factory for all supported texture formats.
 */
class TextureNodeFactory
{
public:
    virtual ~TextureNodeFactory() = default;

    /** @return new TextureNode for the given format. */
    virtual std::unique_ptr<TextureNode> create(TextureFormat format) = 0;

    /**
     * @return true if the TextureNode must be recreated when switching from
     *         format a -> b.
     */
    virtual bool needToChangeNodeType(TextureFormat a,
                                      TextureFormat b) const = 0;
};

/**
 * Concrete factory for [RGBA|YUV] QSG texture nodes.
 */
class TextureNodeFactoryImpl : public TextureNodeFactory
{
public:
    TextureNodeFactoryImpl(QQuickWindow& _window, TextureType _type);
    std::unique_ptr<TextureNode> create(TextureFormat format) final;
    bool needToChangeNodeType(TextureFormat a, TextureFormat b) const final;

private:
    QQuickWindow& _window;
    TextureType _type = TextureType::Static;
};

#endif

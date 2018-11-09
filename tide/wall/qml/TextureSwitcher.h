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

#ifndef TEXTURESWITCHER_H
#define TEXTURESWITCHER_H

#include "TextureNodeFactory.h"

/**
 * Swap double-buffered textures of various types during rendering.
 */
class TextureSwitcher
{
public:
    /** Set the image used to update the back texture in the next update(). */
    void setNextImage(ImagePtr image);

    /** Request a swap of the front/back textures in the next update(). */
    void requestSwap();

    /**
     * Update and/or swap a texture node, (re-)creating it if needed.
     * @param node to update with the given image, swapping its front/back
     *        textures if requested with requestSwap(). The node is reset if the
     *        image format has changed.
     * @param factory to create appropriate nodes for the image type.
     */
    void update(std::unique_ptr<TextureNode>& node,
                TextureNodeFactory& factory);

    virtual ~TextureSwitcher() = default;

private:
    bool _swapRequested = false;
    bool _swapPossible = false;
    ImagePtr _image;
    TextureFormat _format = TextureFormat::rgba;
    std::unique_ptr<TextureNode> _nextNode;

    virtual void aboutToSwitch(TextureNode&, TextureNode&) {}
    bool _canSwap() const;
    void _swap(std::unique_ptr<TextureNode>& node);
    bool _needToChangeNextNode(const TextureNodeFactory& factory);
    void _createNextNode(TextureNodeFactory& factory);
    void _uploadImage(TextureNode& node);
};

#endif

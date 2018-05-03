/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#ifndef STREAMIMAGE_H
#define STREAMIMAGE_H

#include "data/YUVImage.h"

/**
 * Image wrapper for a pixel stream image.
 */
class StreamImage : public YUVImage
{
public:
    /** Constructor, stores the given deflect frame. */
    StreamImage(deflect::server::FramePtr frame, uint tileIndex);

    /** @copydoc Image::getWidth */
    int getWidth() const final;

    /** @copydoc Image::getHeight */
    int getHeight() const final;

    /** @copydoc Image::getData */
    const uint8_t* getData(uint texture) const final;

    /** @copydoc Image::getRowOrder */
    deflect::RowOrder getRowOrder() const final;

    /** @copydoc Image::getFormat */
    TextureFormat getFormat() const final;

    /** @copydoc Image::getColorSpace */
    ColorSpace getColorSpace() const final;

    /** @return the position of the image in the stream. */
    QPoint getPosition() const;

    /** Copy another image of the same format at the given position. */
    void copy(const StreamImage& source, const QPoint& position);

private:
    const deflect::server::FramePtr _frame;
    const uint _tileIndex;

    void _copy(const StreamImage& image, uint texture, const QPoint& position);
    uint8_t* _getData(const uint texture);
};

#endif

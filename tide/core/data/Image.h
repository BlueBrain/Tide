/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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

#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"

/**
 * An interface to provide necessary image information for the texture upload.
 *
 * Valid image formats are:
 * - RGBA: 1 texture plane, 32 bits per pixel (in any GL-compatible arrangement)
 * - YUV: 3 texture planes, 8 bits per pixel
 *
 * Derived classes must comply with this requirement.
 */
class Image
{
public:
    virtual ~Image() = default;

    /** @return the width of the image. */
    virtual int getWidth() const = 0;

    /** @return the height of the image. */
    virtual int getHeight() const = 0;

    /** @return the view port for displaying part of the image. */
    virtual QRect getViewPort() const
    {
        return QRect(QPoint(0, 0), QSize(getWidth(), getHeight()));
    }

    /** @return the dimensions of the given texture plane. */
    virtual QSize getTextureSize(uint texture = 0) const
    {
        return texture == 0 ? QSize(getWidth(), getHeight()) : QSize();
    }

    /** @return the pointer to the pixels of the given texture plane. */
    virtual const uint8_t* getData(uint texture = 0) const = 0;

    /** @return the size of the pixel data buffer of the given texture plane. */
    virtual size_t getDataSize(const uint texture = 0) const
    {
        const auto tex = getTextureSize(texture);
        const auto bpp = getFormat() == TextureFormat::rgba ? 4 : 1;
        return tex.width() * tex.height() * bpp;
    }

    /** @return the row order of the image data. */
    virtual deflect::RowOrder getRowOrder() const
    {
        return deflect::RowOrder::top_down;
    }

    /** @return the format of the image. */
    virtual TextureFormat getFormat() const = 0;

    /** @return the color space of the image. */
    virtual ColorSpace getColorSpace() const { return ColorSpace::undefined; }
    /** @return the OpenGL pixel format of the image data. */
    virtual uint getGLPixelFormat() const = 0;

    /** @return true if generateGpuImage must be called from render thread. */
    virtual bool isGpuImage() const { return false; }
    /**
     * Generate the GPU image.
     * This method will be called on a thread with an active OpenGL context if
     * isGpuImage() is true;
     */
    virtual bool generateGpuImage() const { return false; }
};

#endif

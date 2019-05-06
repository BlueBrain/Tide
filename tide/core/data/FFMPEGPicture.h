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

#ifndef FFMPEGPICTURE_H
#define FFMPEGPICTURE_H

#include "YUVImage.h"

#include <QByteArray>
#include <QImage>

#include <array>
#include <memory>

class FFMPEGFrame;

enum class StereoView
{
    NONE,
    LEFT,
    RIGHT
};

/**
 * A decoded frame of the movie stream in RGBA or YUV format.
 */
class FFMPEGPicture : public YUVImage
{
public:
    /** Allocate a new picture. */
    FFMPEGPicture(std::shared_ptr<FFMPEGFrame> frame);

    /** @copydoc Image::getWidth */
    int getWidth() const final;

    /** @copydoc Image::getHeight */
    int getHeight() const final;

    /** @copydoct Image::getViewPort */
    QRect getViewPort() const final;

    /** @copydoc Image::getData */
    const uint8_t* getData(uint texture = 0) const final;

    /** @copydoc Image::getFormat */
    TextureFormat getFormat() const final;

    /** @copydoc Image::getColorSpace */
    ColorSpace getColorSpace() const final;

    /** @return data size of a given image texture plane. */
    size_t getDataSize(uint texture) const;

    /** @return the picture as a QImage */
    QImage toQImage() const;

    /** Set stereo view for the picture */
    void setStereoView(const StereoView view);

private:
    std::shared_ptr<FFMPEGFrame> _frame;
    std::array<size_t, 3> _dataSize{{0, 0, 0}};
    size_t _width{0};
    size_t _height{0};
    StereoView _stereoView{StereoView::NONE};
};

#endif

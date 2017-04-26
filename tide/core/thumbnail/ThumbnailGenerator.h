/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#ifndef THUMBNAILGENERATOR_H
#define THUMBNAILGENERATOR_H

#include <QColor>
#include <QImage>
#include <QSize>
#include <QString>

/**
 * Abstract class for generating thumbnails for various content types.
 */
class ThumbnailGenerator
{
public:
    /**
     * Constructor.
     *
     * @param size target size for the generated thumbnails.
     */
    ThumbnailGenerator(const QSize& size);

    /** Virtual destructor. */
    virtual ~ThumbnailGenerator() {}
    /**
     * Generate a thumbnail for a given file.
     *
     * Derived classes are expected to always return an image of the correct
     * size. The image can be a placeholder in case of error, but it should not
     * be null.
     *
     * @param filename the content to generate the image for.
     * @return thumbnail the desired thumbnail, or a placeholder image.
     */
    virtual QImage generate(const QString& filename) const = 0;

protected:
    /** Target size for the thumbnails. */
    const QSize _size;

    /** Aspect ratio policy for the thumbnails. */
    const Qt::AspectRatioMode _aspectRatioMode;

    /** Create a generic placeholder image indicating that an error occured. */
    QImage createErrorImage(const QString& message) const;

    /** Create a gradient image, can be used as background for a placeholder. */
    QImage createGradientImage(const QColor& bgcolor1,
                               const QColor& bgcolor2) const;

    /** Paint text over an image, for example on top of a placeholder image. */
    void paintText(QImage& img, const QString& text) const;
};

#endif

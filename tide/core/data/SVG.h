/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#ifndef SVG_H
#define SVG_H

#include "types.h"

#include <QImage>

/**
 * An SVG document which can be rendered as a QImage.
 *
 * This class is a facade to different SVG readers/rendering backends available.
 * It currently uses (in order of preference):
 * - RSVG with Cairo
 * - QSvgRenderer (with GPU antialiasing)
 */
class SVG
{
public:
    /**
     * Open an svg document.
     * @param uri the file to open.
     * @throw std::runtime_error if the file could not be opened.
     */
    explicit SVG(const QString& uri);

    /**
     * Create an svg document from existing data.
     * @param svgData the svg data, typically read from an svg file
     * @throw std::runtime_error if the data is invalid
     */
    explicit SVG(const QByteArray& svgData);

    /** Close the document. */
    ~SVG();

    /** @return the filename of the document passed to the constructor. */
    const QString& getFilename() const;

    /** @return the dimensions of the document in pixels. */
    QSize getSize() const;

    /** @return the raw svg data. */
    const QByteArray& getData() const;

    /**
     * Render the document to an image.
     * @param imageSize the desired size for the image
     * @param region the target area of the svg to render, in normalized coord.
     * @return the rendered image region, or an empty QImage on failure.
     * @note when using the SVGQtGpuBackend, this function must be called from a
     *       thread with an active OpenGL context
     */
    QImage renderToImage(const QSize& imageSize,
                         const QRectF& region = UNIT_RECTF) const;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif

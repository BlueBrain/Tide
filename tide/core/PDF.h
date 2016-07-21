/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#ifndef PDF_H
#define PDF_H

#include "types.h"

#include <QImage>

/**
 * A PDF document which can be rendered as a QImage.
 *
 * This class is a facade to different PDF readers/rendering backends available.
 * It currently uses (in order of preference):
 * - Poppler-glib with Cairo (faster)
 * - Poppler-qt with Splash
 */
class PDF
{
public:
    /**
     * Open a document.
     * @param uri the file to open.
     */
    PDF( const QString& uri );

    /** Close the document. */
    ~PDF();

    /** @return the filename of the document passed to the constructor. */
    const QString& getFilename() const;

    /** @return true if the document is valid. */
    bool isValid() const;

    /** @return the dimensions of the document in pixels. */
    QSize getSize() const;

    /** @return the current page number. */
    int getPage() const;

    /**
     * Go to a given page number.
     * @param pageNumber the page to open. If invalid, the page is not changed.
     */
    void setPage( int pageNumber );

    /** @return the number of pages in the document. */
    int getPageCount() const;

    /**
     * Render the document to an image.
     * @param imageSize the desired size for the image
     * @param region the target area of the page to render, in normalized coord.
     * @return the rendered image region, or an empty QImage on failure.
     */
    QImage renderToImage( const QSize& imageSize,
                          const QRectF& region = UNIT_RECTF ) const;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif

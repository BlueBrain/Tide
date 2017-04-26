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

#include "PDFThumbnailGenerator.h"

#include "data/PDF.h"
#include "log.h"

#include <QFileInfo>

namespace
{
const size_t sizeOfMegabyte = 1024 * 1024;
// empirical value used to minimize thumbnail generation time
const qint64 maxPdfPageSize = 2 * sizeOfMegabyte;
}

PDFThumbnailGenerator::PDFThumbnailGenerator(const QSize& size)
    : ThumbnailGenerator(size)
{
}

QImage PDFThumbnailGenerator::generate(const QString& filename) const
{
    const PDF pdf(filename);

    if (!pdf.isValid())
    {
        put_flog(LOG_ERROR, "could not open pdf file: '%s'",
                 filename.toLatin1().constData());
        return createErrorImage("pdf");
    }

    if (QFileInfo(filename).size() > maxPdfPageSize * pdf.getPageCount())
        return _createLargePdfPlaceholder();

    const QSize imageSize = pdf.getSize().scaled(_size, _aspectRatioMode);
    const QImage image = pdf.renderToImage(imageSize);
    if (image.isNull())
    {
        put_flog(LOG_ERROR, "could not render pdf file: '%s'",
                 filename.toLatin1().constData());
        return createErrorImage("pdf");
    }

    return image;
}

QImage PDFThumbnailGenerator::_createLargePdfPlaceholder() const
{
    QImage img = createGradientImage(Qt::darkBlue, Qt::white);
    paintText(img, "LARGE\nPDF");
    return img;
}

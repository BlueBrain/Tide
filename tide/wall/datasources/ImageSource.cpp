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

#include "ImageSource.h"

#include "data/ImageReader.h"

ImageSource::ImageSource(const QString& uri)
    : _reader{std::make_unique<ImageReader>(uri)}
{
}

ImageSource::~ImageSource()
{
}

QRect ImageSource::getTileRect(const uint tileIndex) const
{
    Q_UNUSED(tileIndex);

    return QRect(QPoint(0, 0), _reader->getSize());
}

QSize ImageSource::getTilesArea(const uint lod, const uint channel) const
{
    Q_UNUSED(lod);
    Q_UNUSED(channel);

    return _reader->getSize();
}

QImage ImageSource::getCachableTileImage(const uint tileIndex,
                                         const deflect::View view) const
{
    Q_UNUSED(tileIndex);

    // temporary reader required for thread safety
    return ImageReader{_reader->getUri()}.getImage(view);
}

bool ImageSource::isStereo() const
{
    return _reader->isStereo();
}

Indices ImageSource::computeVisibleSet(const QRectF& visibleTilesArea,
                                       const uint lod, const uint channel) const
{
    Q_UNUSED(lod);
    Q_UNUSED(channel);

    if (!_reader->isValid() || visibleTilesArea.isEmpty())
        return Indices();

    return {0};
}

uint ImageSource::getMaxLod() const
{
    return 0;
}

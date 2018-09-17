/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#include "ImageReader.h"

#include "data/QtImage.h"
#include "utils/stereoimage.h"

#include <QFileInfo>

namespace
{
bool _isStereoImage(const QString& uri)
{
    return QFileInfo{uri}.suffix().toLower() == "jps";
}

QImage _ensureGlCompatibleFormat(const QImage& image)
{
    return QtImage::is32Bits(image)
               ? image
               : image.convertToFormat(QImage::Format_ARGB32);
}
}

ImageReader::ImageReader(const QString& uri)
    : _reader{std::make_unique<QImageReader>(uri)}
    , _stereo{_isStereoImage(uri)}
{
}

QString ImageReader::getUri() const
{
    return _reader->fileName();
}

bool ImageReader::isValid() const
{
    return _reader->canRead();
}

bool ImageReader::isStereo() const
{
    return _stereo;
}

QSize ImageReader::getSize() const
{
    return isStereo() ? stereoimage::getMonoSize(_reader->size())
                      : _reader->size();
}

QImage ImageReader::getImage(const deflect::View view) const
{
    return _ensureGlCompatibleFormat(_readImage(view));
}

QStringList ImageReader::getSupportedImageFormats()
{
    QStringList extensions;

    for (const auto& entry : QImageReader::supportedImageFormats())
        extensions << entry;
    extensions << "jps";

    return extensions;
}

QImage ImageReader::_readImage(const deflect::View view) const
{
    return isStereo() ? stereoimage::extract(_reader->read(), view)
                      : _reader->read();
}

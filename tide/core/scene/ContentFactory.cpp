/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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

#include "ContentFactory.h"

#include "config.h"

#include "Content.h"
#include "ErrorContent.h"
#include "ImageContent.h"
#include "PixelStreamContent.h"
#include "SVGContent.h"
#include "data/ImageReader.h"
#if TIDE_USE_TIFF
#include "ImagePyramidContent.h"
#include "data/TiffPyramidReader.h"
#endif
#if TIDE_ENABLE_MOVIE_SUPPORT
#include "MovieContent.h"
#endif
#if TIDE_ENABLE_PDF_SUPPORT
#include "PDFContent.h"
#endif
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
#include "WebbrowserContent.h"
#endif

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace
{
const QSize maxTextureSize(16384, 16384);

ContentType _getContentTypeForFile(const QString& uri)
{
    const auto extension = QFileInfo(uri).suffix().toLower();

    // SVGs must be processed first because they can also be read as an image
    if (SVGContent::getSupportedExtensions().contains(extension))
        return ContentType::svg;

#if TIDE_ENABLE_MOVIE_SUPPORT
    if (MovieContent::getSupportedExtensions().contains(extension))
        return ContentType::movie;
#endif

#if TIDE_ENABLE_PDF_SUPPORT
    if (PDFContent::getSupportedExtensions().contains(extension))
        return ContentType::pdf;
#endif

#if TIDE_USE_TIFF
    if (ImagePyramidContent::getSupportedExtensions().contains(extension))
    {
        try
        {
            TiffPyramidReader tif{uri};
            return ContentType::image_pyramid;
        }
        catch (...)
        { /* not a pyramid file, pass */
        }
    }
#endif

    const auto imageReader = ImageReader(uri);
    if (imageReader.isValid())
    {
        const auto size = imageReader.getSize();

        if (size.width() <= maxTextureSize.width() &&
            size.height() <= maxTextureSize.height())
            return ContentType::image;

        throw load_error(
            "Image is too big to open. Try converting it to a TIFF image "
            "pyramid using Tide's 'pyramidify' tool.");
    }

    throw load_error("Unsupported content type.");
}

ContentPtr _makeContent(const QString& uri)
{
    switch (_getContentTypeForFile(uri))
    {
    case ContentType::svg:
        return std::make_unique<SVGContent>(uri);
#if TIDE_USE_TIFF
    case ContentType::image_pyramid:
        return std::make_unique<ImagePyramidContent>(uri);
#endif
#if TIDE_ENABLE_MOVIE_SUPPORT
    case ContentType::movie:
        return std::make_unique<MovieContent>(uri);
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    case ContentType::pdf:
        return std::make_unique<PDFContent>(uri);
#endif
    case ContentType::image:
        return std::make_unique<ImageContent>(uri);
    default:
        throw load_error("Unsupported content type.");
    }
}
}

ContentPtr ContentFactory::createContent(const QString& uri)
{
    auto content = _makeContent(uri);
    if (!content->readMetadata())
        throw load_error("Could not read content metadata.");
    return content;
}

ContentPtr ContentFactory::createPixelStreamContent(const QString& uri,
                                                    const QSize& size,
                                                    const StreamType stream)
{
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    if (stream == StreamType::WEBBROWSER)
        return std::make_unique<WebbrowserContent>(uri, size);
#endif
    const auto keyboard = stream == StreamType::EXTERNAL;
    return std::make_unique<PixelStreamContent>(uri, size, keyboard);
}

ContentPtr ContentFactory::createErrorContent(const Content& content)
{
    const auto& uri = content.getUri();
    const auto& size = content.getDimensions();

    return std::make_unique<ErrorContent>(uri, size);
}

bool ContentFactory::isValidImageFile(const QString& uri)
{
    const auto imageReader = ImageReader{uri};
    if (!imageReader.isValid())
        return false;

    const auto size = imageReader.getSize();
    return size.width() <= maxTextureSize.width() &&
           size.height() <= maxTextureSize.height();
}

ContentPtr ContentFactory::createErrorContent(const QString& uri)
{
    return std::make_unique<ErrorContent>(uri, QSize());
}

const QStringList& ContentFactory::getSupportedExtensions()
{
    static QStringList extensions;

    if (extensions.empty())
    {
#if TIDE_ENABLE_PDF_SUPPORT
        extensions.append(PDFContent::getSupportedExtensions());
#endif
        extensions.append(SVGContent::getSupportedExtensions());
        extensions.append(ImageContent::getSupportedExtensions());
#if TIDE_ENABLE_MOVIE_SUPPORT
        extensions.append(MovieContent::getSupportedExtensions());
#endif
        extensions.removeDuplicates();
    }

    return extensions;
}

const QStringList& ContentFactory::getSupportedFilesFilter()
{
    static QStringList filters;

    if (filters.empty())
    {
        for (const auto& ext : getSupportedExtensions())
            filters.append("*." + ext);
    }

    return filters;
}

QString ContentFactory::getSupportedFilesFilterAsString()
{
    QString string;
    QTextStream out(&string);

    out << "Content files (";
    for (const auto& filter : getSupportedFilesFilter())
        out << filter << " ";
    out << ")";

    return string;
}

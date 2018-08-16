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

#include "DataSourceFactory.h"

#include "config.h"
#include "scene/Content.h"

#include "datasources/ImageSource.h"
#include "datasources/PixelStreamUpdater.h"
#include "datasources/SVGTiler.h"

#if TIDE_ENABLE_MOVIE_SUPPORT
#include "datasources/MovieUpdater.h"
#endif

#if TIDE_USE_TIFF
#include "datasources/ImagePyramidDataSource.h"
#endif

#if TIDE_ENABLE_PDF_SUPPORT
#include "datasources/PDFTiler.h"
#endif

std::unique_ptr<DataSource> DataSourceFactory::create(const Content& content)
{
    switch (content.getType())
    {
#if TIDE_ENABLE_MOVIE_SUPPORT
    case ContentType::movie:
        return std::make_unique<MovieUpdater>(content.getUri());
#endif

    case ContentType::pixel_stream:
    case ContentType::webbrowser:
        return std::make_unique<PixelStreamUpdater>(content.getUri());
    case ContentType::svg:
        return std::make_unique<SVGTiler>(content.getUri());
    case ContentType::image:
        return std::make_unique<ImageSource>(content.getUri());

#if TIDE_ENABLE_PDF_SUPPORT
    case ContentType::pdf:
        return std::make_unique<PDFTiler>(content.getUri());
#endif

#if TIDE_USE_TIFF
    case ContentType::image_pyramid:
        return std::make_unique<ImagePyramidDataSource>(content.getUri());
#endif
    default:
        throw std::logic_error("No data source for this content type");
    }
}

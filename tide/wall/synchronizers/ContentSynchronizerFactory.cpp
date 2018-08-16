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

#include "ContentSynchronizerFactory.h"

#include "config.h"
#include "scene/Content.h"
#include "scene/MultiChannelContent.h"

#include "datasources/PixelStreamUpdater.h"
#include "synchronizers/BasicSynchronizer.h"
#include "synchronizers/LodSynchronizer.h"
#include "synchronizers/PixelStreamSynchronizer.h"

#if TIDE_ENABLE_MOVIE_SUPPORT
#include "datasources/MovieUpdater.h"
#include "synchronizers/MovieSynchronizer.h"
#endif

#if TIDE_ENABLE_PDF_SUPPORT
#include "datasources/PDFTiler.h"
#include "synchronizers/PDFSynchronizer.h"
#endif

std::unique_ptr<ContentSynchronizer> ContentSynchronizerFactory::create(
    const Content& content, const deflect::View view,
    std::shared_ptr<DataSource> source)
{
    switch (content.getType())
    {
#if TIDE_ENABLE_MOVIE_SUPPORT
    case ContentType::movie:
    {
        return std::make_unique<MovieSynchronizer>(
            std::dynamic_pointer_cast<MovieUpdater>(source), view);
    }
#endif
    case ContentType::pixel_stream:
    case ContentType::webbrowser:
    {
        const auto channel =
            static_cast<const MultiChannelContent&>(content).getChannel();
        return std::make_unique<PixelStreamSynchronizer>(
            std::dynamic_pointer_cast<PixelStreamUpdater>(source), view,
            channel);
    }
#if TIDE_USE_TIFF
    case ContentType::image_pyramid:
    {
        return std::make_unique<LodSynchronizer>(source);
    }
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    case ContentType::pdf:
    {
        return std::make_unique<PDFSynchronizer>(
            std::dynamic_pointer_cast<PDFTiler>(source));
    }
#endif
    case ContentType::svg:
    {
        return std::make_unique<LodSynchronizer>(source);
    }
    case ContentType::image:
    {
        return std::make_unique<BasicSynchronizer>(source, view);
    }
    default:
        throw std::runtime_error("No ContentSynchronizer for ContentType");
    }
}

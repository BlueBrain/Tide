/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#include "ContentSynchronizer.h"

#include "config.h"
#include "Content.h"

#include "BasicSynchronizer.h"
#include "DynamicTextureSynchronizer.h"
#include "ImageSynchronizer.h"
#include "MovieSynchronizer.h"
#include "PixelStreamSynchronizer.h"
#if TIDE_ENABLE_PDF_SUPPORT
#include "PDFSynchronizer.h"
#endif
#include "SVGSynchronizer.h"

ContentSynchronizer::~ContentSynchronizer() {}

ContentSynchronizerPtr ContentSynchronizer::create( ContentPtr content )
{
    const QString& uri = content->getURI();
    switch( content->getType( ))
    {
    case CONTENT_TYPE_DYNAMIC_TEXTURE:
        return make_unique<DynamicTextureSynchronizer>( uri );
    case CONTENT_TYPE_MOVIE:
        return make_unique<MovieSynchronizer>( uri );
    case CONTENT_TYPE_PIXEL_STREAM:
    case CONTENT_TYPE_WEBBROWSER:
        return make_unique<PixelStreamSynchronizer>();
#if TIDE_ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
        return make_unique<PDFSynchronizer>( uri );
#endif
    case CONTENT_TYPE_SVG:
        return make_unique<SVGSynchronizer>( uri );
    case CONTENT_TYPE_TEXTURE:
        return make_unique<ImageSynchronizer>( uri );
    default:
        throw std::runtime_error( "No ContentSynchronizer for ContentType" );
    }
}

void ContentSynchronizer::onTextureReady( TilePtr tile )
{
    emit requestTileUpdate( shared_from_this(), TileWeakPtr( tile ));
}

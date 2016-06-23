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

#include "ThumbnailGeneratorFactory.h"

#include "config.h"
#if TIDE_ENABLE_PDF_SUPPORT
#  include "PDFContent.h"
#  include "PDFThumbnailGenerator.h"
#endif

#include "DefaultThumbnailGenerator.h"
#include "FolderThumbnailGenerator.h"
#include "ImageThumbnailGenerator.h"
#include "MovieThumbnailGenerator.h"
#include "PyramidThumbnailGenerator.h"
#include "StateThumbnailGenerator.h"

#include "DynamicTextureContent.h"
#include "MovieContent.h"
#include "TextureContent.h"

#include <QDir>

ThumbnailGeneratorPtr
ThumbnailGeneratorFactory::getGenerator( const QString& filename,
                                         const QSize& size )
{
    const QString& extension = QFileInfo( filename ).suffix().toLower();

    if( !filename.isEmpty() && QDir( filename ).exists( ))
        return ThumbnailGeneratorPtr( new FolderThumbnailGenerator( size ));

    if( extension == "dcx" )
        return ThumbnailGeneratorPtr( new StateThumbnailGenerator( size ));

    if( MovieContent::getSupportedExtensions().contains( extension ))
        return ThumbnailGeneratorPtr( new MovieThumbnailGenerator( size ));

    if( TextureContent::getSupportedExtensions().contains( extension ))
        return ThumbnailGeneratorPtr( new ImageThumbnailGenerator( size ));

    if( DynamicTextureContent::getSupportedExtensions().contains( extension ))
        return ThumbnailGeneratorPtr( new PyramidThumbnailGenerator( size ));

#if TIDE_ENABLE_PDF_SUPPORT
    if( PDFContent::getSupportedExtensions().contains( extension ))
        return ThumbnailGeneratorPtr( new PDFThumbnailGenerator( size ));
#endif

    return ThumbnailGeneratorPtr( new DefaultThumbnailGenerator( size ));
}

ThumbnailGeneratorPtr
ThumbnailGeneratorFactory::getDefaultGenerator( const QSize& size )
{
    return ThumbnailGeneratorPtr( new DefaultThumbnailGenerator( size ));
}

FolderThumbnailGeneratorPtr
ThumbnailGeneratorFactory::getFolderGenerator( const QSize& size )
{
    return FolderThumbnailGeneratorPtr( new FolderThumbnailGenerator( size ));
}

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

#include "DynamicTexture.h"

#include <iostream>
#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>

namespace
{
const int SUCCESS_RETURN_CODE = 0;
const int INVALID_PARAM_COUNT_ERROR_CODE = -1;
const int INVALID_IMAGE_ERROR_CODE = -2;
const int INVALID_OUTPUTDIR_ERROR_CODE = -3;
const int PYRAMID_CREATION_FAILED_ERROR_CODE = -4;
}

int main( int argc, char* argv[] )
{
    if( argc != 3 )
    {
        std::cout << "Usage: pyramidmaker imagefile outputdir" << std::endl;
        return INVALID_PARAM_COUNT_ERROR_CODE;
    }

    QCoreApplication app( argc, argv );

    const QString filename( argv[1] );
    std::cout << "source image filename: " << filename.toStdString() <<
                 std::endl;

    DynamicTexturePtr texture( new DynamicTexture( filename ));
    if( texture->getSize().isEmpty( ))
    {
        std::cerr << "The source image could not be read." << std::endl;
        return INVALID_IMAGE_ERROR_CODE;
    }

    const QString destDir( argv[2] );
    std::cout << "target location for image pyramid folder: " <<
                 destDir.toStdString() << std::endl;

    const QFileInfo dir( destDir );
    if( !dir.exists() || !dir.isWritable( ))
    {
        std::cerr << "Invalid or unwritable output directory." << std::endl;
        return INVALID_OUTPUTDIR_ERROR_CODE;
    }

    if( !texture->generateImagePyramid( destDir ))
    {
        std::cerr << "Image pyramid creation failed." << std::endl;
        return PYRAMID_CREATION_FAILED_ERROR_CODE;
    }

    std::cout << "Done generating image pyramid!" << std::endl;
    return SUCCESS_RETURN_CODE;
}

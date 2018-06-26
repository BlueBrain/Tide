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

#ifndef TIDEMOCK_STANDARDSCREENCONFIGS_H
#define TIDEMOCK_STANDARDSCREENCONFIGS_H

#include "configuration/SurfaceConfig.h"

inline SurfaceConfig standardWall()
{
    SurfaceConfig surface;
    surface.displayWidth = 7716;
    surface.displayHeight = 3264;
    return surface;
}

inline SurfaceConfig standardWallWithDimension()
{
    auto surface = standardWall();
    surface.dimensions = QSizeF{4 * 1.215, 3 * 0.686}; // 4x3 LX55HDS-L-ERO
    return surface;
}

inline SurfaceConfig narrowWall()
{
    SurfaceConfig surface;
    surface.displayWidth = 5784;
    surface.displayHeight = 3264;
    return surface;
}

inline SurfaceConfig narrowWallWithDimension()
{
    auto surface = narrowWall();
    surface.dimensions = QSizeF{3 * 1.215, 3 * 0.686}; // 3x3 LX55HDS-L-ERO
    return surface;
}

inline SurfaceConfig wideThinWall()
{
    SurfaceConfig surface;
    surface.displayWidth = 7720;
    surface.displayHeight = 2160;
    return surface;
}

inline SurfaceConfig wideThinWallWithDimension()
{
    auto surface = wideThinWall();
    surface.dimensions = QSizeF{2 * 2.159 + 0.03, 1.214}; // 2x1 UR9851
    return surface;
}

inline SurfaceConfig largeProjectionSurface()
{
    SurfaceConfig surface;
    surface.displayWidth = 11940;
    surface.displayHeight = 3424;
    return surface;
}

inline SurfaceConfig largeProjectionSurfaceWithDimensions()
{
    auto surface = largeProjectionSurface();
    surface.dimensions = QSizeF{8.0, 2.3};
    return surface;
}

#endif

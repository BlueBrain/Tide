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

#ifndef SURFACECONFIG_H
#define SURFACECONFIG_H

#include "types.h"

#include "scene/Background.h"

/**
 * A uniform display surface, composed of identical displays of the same size.
 *
 * The surface can optionally be subdivided in regular sub-groups of displays
 * called screens, in reference to the logical X screen used for rendering.
 */
struct SurfaceConfig
{
    uint displayWidth = 0;       // width of a display in pixels
    uint displayHeight = 0;      // height of a display in pixels
    uint displaysPerScreenX = 1; // number of displays per screen in the x axis
    uint displaysPerScreenY = 1; // number of displays per screen in the y axis
    uint screenCountX = 1;       // number of screens along the x axis
    uint screenCountY = 1;       // number of screens along the y axis
    int bezelWidth = 0;  // horizontal padding between two screens in pixels
    int bezelHeight = 0; // vertical padding between two screens in pixels
    QSizeF dimensions;   // physical dimensions in meters

    /** Background content and color. */
    BackgroundPtr background = Background::create();

    /** @return the width of a screen in pixels. */
    uint getScreenWidth() const;

    /** @return the height of a screen in pixels. */
    uint getScreenHeight() const;

    /**
     * @return the coordinates and dimensions of a specific screen in pixels.
     * @throw std::invalid_argument if the tileIndex does not exits.
     */
    QRect getScreenRect(const QPoint& tileIndex) const;

    /** @return total width of the surface in pixels, including bezels. */
    uint getTotalWidth() const;

    /** @return total height of the surface in pixels, including bezels. */
    uint getTotalHeight() const;

    /** @return total size of the surface in pixels, including bezels. */
    QSize getTotalSize() const;

    /** @return aspect ratio of the surface, including bezels. */
    double getAspectRatio() const;

    /** @return the size in pixels of an area in meters. */
    QSize toPixelSize(const QSizeF& sizeInMeters) const;
};

#endif

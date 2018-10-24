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

#define BOOST_TEST_MODULE LauncherPlacer

#include "localstreamer/LauncherPlacer.h"
#include "configuration/SurfaceConfig.h"

#include "StandardScreenConfigs.h"

#include <boost/test/unit_test.hpp>

namespace
{
constexpr auto targetSizeMeters = QSizeF{0.8, 0.75};
constexpr auto aspectRatio =
    targetSizeMeters.width() / targetSizeMeters.height();
constexpr auto maxSizePixels = QSize{1920, (int)(1920 / aspectRatio)};
constexpr auto minSizePixels = QSize{(int)(256 * aspectRatio), 256};

SurfaceConfig smallSurface()
{
    SurfaceConfig surface;
    surface.displayWidth = 640;
    surface.displayHeight = 480;
    return surface;
}

SurfaceConfig hugeSurface()
{
    SurfaceConfig surface;
    surface.displayWidth = 16000;
    surface.displayHeight = 8000;
    return surface;
}
}

void checkMargins(const QRect& window, const QSize& surfaceSize)
{
    BOOST_CHECK_GE(window.top(), 0.05 * surfaceSize.height());
    BOOST_CHECK_LE(window.bottom(), 0.95 * surfaceSize.height());

    BOOST_CHECK_GE(window.left(), (int)(0.4 * window.width()));
    BOOST_CHECK_LE(window.left(), 0.25 * surfaceSize.width());
    BOOST_CHECK_LE(window.right(), 0.95 * surfaceSize.width());
}

void checkDimensions(const QRect& window, const QSize& surfaceSize)
{
    BOOST_CHECK_LE(window.height(), 0.9 * surfaceSize.height());
    BOOST_CHECK_LE(window.width(), 0.9 * surfaceSize.width());

    BOOST_CHECK_LE(window.height(), maxSizePixels.height());
    BOOST_CHECK_LE(window.width(), maxSizePixels.width());

    BOOST_CHECK_GE(window.height(), minSizePixels.height());
    BOOST_CHECK_GE(window.width(), minSizePixels.width());
}

void checkVerticalPosition(const QRect& window, const SurfaceConfig& surface)
{
    // centered vertically on walls that likely don't start from the floor
    if (surface.dimensions.isValid() && surface.dimensions.height() < 1.6)
        BOOST_CHECK_CLOSE(window.center().y(), surface.getTotalHeight() * 0.5,
                          1);
    // otherwise placed somewhere above the middle of the screen
    else
        BOOST_CHECK_GE(window.center().y(), surface.getTotalHeight() * 0.3);
}

void testPlacement(const SurfaceConfig& surface, const QSize& expectedSize)
{
    const auto window = LauncherPlacer{surface}.getCoordinates();
    checkMargins(window, surface.getTotalSize());
    checkDimensions(window, surface.getTotalSize());
    checkVerticalPosition(window, surface);
    BOOST_CHECK_EQUAL(window.size(), expectedSize);
}

BOOST_AUTO_TEST_CASE(place_launcher_on_standard_wall)
{
    testPlacement(standardWall(), QSize{1740, 1632});
}

BOOST_AUTO_TEST_CASE(place_launcher_on_narrow_wall)
{
    testPlacement(narrowWall(), QSize{1740, 1632});
}

BOOST_AUTO_TEST_CASE(place_launcher_on_wide_thin_wall)
{
    testPlacement(wideThinWall(), QSize{1152, 1080});
}

BOOST_AUTO_TEST_CASE(place_launcher_on_large_surface)
{
    testPlacement(largeProjectionSurface(), QSize{1826, 1712});
}

BOOST_AUTO_TEST_CASE(place_launcher_on_small_surface)
{
    testPlacement(smallSurface(), minSizePixels);
}

BOOST_AUTO_TEST_CASE(place_launcher_on_huge_surface)
{
    testPlacement(hugeSurface(), maxSizePixels);
}

void testPlacementUsingSurfaceDimensions(const SurfaceConfig& surface)
{
    testPlacement(surface, surface.toPixelSize(targetSizeMeters));
}

BOOST_AUTO_TEST_CASE(place_launcher_on_standard_wall_with_metric_dimensions)
{
    testPlacementUsingSurfaceDimensions(standardWallWithDimension());
}

BOOST_AUTO_TEST_CASE(place_launcher_on_narrow_wall_with_metric_dimensions)
{
    testPlacementUsingSurfaceDimensions(narrowWallWithDimension());
}

BOOST_AUTO_TEST_CASE(place_launcher_on_wide_thin_wall_with_metric_dimensions)
{
    testPlacementUsingSurfaceDimensions(wideThinWallWithDimension());
}

BOOST_AUTO_TEST_CASE(place_launcher_on_large_surface_with_metric_dimensions)
{
    testPlacementUsingSurfaceDimensions(largeProjectionSurfaceWithDimensions());
}

/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE LineLayoutTests

#include <boost/test/unit_test.hpp>

#include "layout/LineLayout.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"

#include "DummyContent.h"
#include "ui.h"

namespace
{
const QSizeF wallSize(1000, 1000);
const QSize contentSize(800, 600);
const qreal aspectRatio{800.0 / 600.0};

ContentPtr make_dummy_content()
{
    return std::make_unique<DummyContent>(contentSize);
}
}

BOOST_AUTO_TEST_CASE(ui_available_surface)
{
    auto group = DisplayGroup::create(wallSize);
    const auto surface = ui::getFocusSurface(*group);

    BOOST_CHECK_EQUAL(surface.x(),
                      ui::getSideControlWidth() + ui::getButtonSize());
    BOOST_CHECK_EQUAL(surface.y(), ui::getButtonSize());
    BOOST_CHECK_EQUAL(surface.width(), wallSize.width() -
                                           ui::getSideControlWidth() -
                                           2 * ui::getButtonSize());
    BOOST_CHECK_EQUAL(surface.height(),
                      wallSize.height() - 2 * ui::getButtonSize());
}

BOOST_AUTO_TEST_CASE(layout_one_window)
{
    auto window = std::make_shared<Window>(make_dummy_content());
    const auto& content = window->getContent();

    auto group = DisplayGroup::create(wallSize);
    group->add(window);
    group->addFocusedWindow(window);

    LineLayout engine(*group);
    const auto coords = engine.getFocusedCoord(*window);

    const auto surface = ui::getFocusSurface(*group);
    const qreal expectedWidth = surface.width() - ui::getWindowControlsMargin();
    const qreal expectedHeight = expectedWidth / content.getAspectRatio();
    const qreal expectedY = (wallSize.height() - expectedHeight) / 2.0;

    // focus mode, vertically centered on wall and repects inner margin
    BOOST_CHECK_EQUAL(coords.x(), surface.x() + ui::getWindowControlsMargin());
    BOOST_CHECK_EQUAL(coords.y(), expectedY);
    BOOST_CHECK_EQUAL(coords.width(), expectedWidth);
    BOOST_CHECK_EQUAL(coords.height(), expectedHeight);
}

BOOST_AUTO_TEST_CASE(layout_two_windows)
{
    auto window1 = std::make_shared<Window>(make_dummy_content());
    auto window2 = std::make_shared<Window>(make_dummy_content());

    auto group = DisplayGroup::create(wallSize);
    group->add(window1);
    group->add(window2);
    group->addFocusedWindow(window1);
    group->addFocusedWindow(window2);

    LineLayout engine(*group);
    const auto coords1 = engine.getFocusedCoord(*window1);
    const auto coords2 = engine.getFocusedCoord(*window2);

    const auto surface = ui::getFocusSurface(*group);

    const auto expectedWidth =
        (surface.width() - 2 * ui::getWindowControlsMargin() -
         ui::getMinWindowSpacing()) /
        2;
    const auto expectedHeight = expectedWidth / aspectRatio;

    const auto expectedX1 = surface.x() + ui::getWindowControlsMargin();
    const auto expectedX2 = coords1.right() + ui::getWindowControlsMargin() +
                            ui::getMinWindowSpacing();
    const auto expectedY = (wallSize.height() - expectedHeight) / 2.0;

    BOOST_CHECK_EQUAL(coords1, QRectF(expectedX1, expectedY, expectedWidth,
                                      expectedHeight));
    BOOST_CHECK_EQUAL(coords2, QRectF(expectedX2, expectedY, expectedWidth,
                                      expectedHeight));
}

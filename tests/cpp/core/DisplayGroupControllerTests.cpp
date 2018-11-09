/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE DisplayGroupControllerTests
#include <boost/test/unit_test.hpp>

#include "control/DisplayGroupController.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"

#include "DummyContent.h"

namespace
{
const QSizeF WALL_SIZE(2000, 1000);
const QSize CONTENT_SIZE(800, 600);
const qreal CONTENT_AR = qreal(CONTENT_SIZE.width()) / CONTENT_SIZE.height();

ContentPtr makeDummyContent()
{
    return std::make_unique<DummyContent>(CONTENT_SIZE);
}
}

struct Fixture
{
    Fixture() { displayGroup->add(window); }
    WindowPtr window{std::make_shared<Window>(makeDummyContent())};
    Content& content{window->getContent()};
    DisplayGroupPtr displayGroup{DisplayGroup::create(WALL_SIZE)};
    DisplayGroupController controller{*displayGroup};
};

BOOST_FIXTURE_TEST_CASE(testShowWindowFullscreenAndExit, Fixture)
{
    BOOST_REQUIRE_EQUAL(content.getZoomRect(), UNIT_RECTF);
    BOOST_REQUIRE(!window->isFullscreen());
    BOOST_REQUIRE_EQUAL(window->size(), CONTENT_SIZE);

    window->setX(100);
    window->setY(150);
    window->setWidth(CONTENT_SIZE.width() / 2);
    window->setHeight(CONTENT_SIZE.height() / 4);
    content.setZoomRect(QRectF(0.2, 0.2, 0.7, 0.7));

    controller.showFullscreen(window->getID());

    BOOST_CHECK(window->isFullscreen());
    BOOST_CHECK_EQUAL(displayGroup->getFullscreenWindow(), window.get());
    const auto& coord = window->getDisplayCoordinates();
    BOOST_CHECK_EQUAL(coord.x(), (WALL_SIZE.width() - coord.width()) / 2);
    BOOST_CHECK_EQUAL(coord.y(), 0);
    BOOST_CHECK_EQUAL(coord.height(), WALL_SIZE.height());
    BOOST_CHECK_EQUAL(coord.width(), coord.height() * CONTENT_AR);
    BOOST_CHECK_EQUAL(content.getZoomRect(), UNIT_RECTF);

    controller.exitFullscreen();

    BOOST_CHECK(!window->isFullscreen());
    BOOST_CHECK_EQUAL(content.getZoomRect(), QRectF(0.2, 0.2, 0.7, 0.7));
    BOOST_CHECK_EQUAL(window->x(), 100);
    BOOST_CHECK_EQUAL(window->y(), 150);
    BOOST_CHECK_EQUAL(window->width(), CONTENT_SIZE.width() / 2);
    BOOST_CHECK_EQUAL(window->height(), CONTENT_SIZE.height() / 4);
}

BOOST_FIXTURE_TEST_CASE(testAdjustSizeOneToOne, Fixture)
{
    window->setWidth(CONTENT_SIZE.width() / 2);
    window->setHeight(CONTENT_SIZE.height() / 4);
    content.setZoomRect(QRectF(0.2, 0.2, 0.7, 0.7));

    const auto center = window->getDisplayCoordinates().center();

    controller.adjustSizeOneToOne(window->getID());

    BOOST_CHECK(!window->isFullscreen());
    const auto& coord = window->getDisplayCoordinates();
    BOOST_CHECK_EQUAL(coord.center().x(), center.x());
    BOOST_CHECK_EQUAL(coord.center().y(), center.y());
    BOOST_CHECK_EQUAL(coord.width(), CONTENT_SIZE.width());
    BOOST_CHECK_EQUAL(coord.height(), CONTENT_SIZE.height());
    BOOST_CHECK_EQUAL(content.getZoomRect(), UNIT_RECTF);
}

BOOST_FIXTURE_TEST_CASE(testAdjustSizeOneToOneHugeContent, Fixture)
{
    const auto hugeSize = WALL_SIZE * 2;
    content.setDimensions(hugeSize.toSize());

    BOOST_REQUIRE_EQUAL(content.getZoomRect(), UNIT_RECTF);
    BOOST_REQUIRE(!window->isFullscreen());
    BOOST_REQUIRE_EQUAL(window->size(), CONTENT_SIZE);

    window->setX(100);
    window->setY(150);
    window->setWidth(320);
    window->setHeight(460);
    content.setZoomRect(QRectF(0.2, 0.2, 0.7, 0.7));

    controller.adjustSizeOneToOne(window->getID());

    BOOST_CHECK(window->isFullscreen());
    BOOST_CHECK_EQUAL(displayGroup->getFullscreenWindow(), window.get());
    const auto& coord = window->getDisplayCoordinates();
    BOOST_CHECK_EQUAL(coord.center().x(), WALL_SIZE.width() / 2);
    BOOST_CHECK_EQUAL(coord.center().y(), WALL_SIZE.height() / 2);
    BOOST_CHECK_EQUAL(coord.width(), hugeSize.width());
    BOOST_CHECK_EQUAL(coord.height(), hugeSize.height());
    BOOST_CHECK_EQUAL(content.getZoomRect(), UNIT_RECTF);

    controller.exitFullscreen();

    BOOST_CHECK(!window->isFullscreen());
    BOOST_CHECK_EQUAL(content.getZoomRect(), QRectF(0.2, 0.2, 0.7, 0.7));
    BOOST_CHECK_EQUAL(window->x(), 100);
    BOOST_CHECK_EQUAL(window->y(), 150);
    BOOST_CHECK_EQUAL(window->width(), 320);
    BOOST_CHECK_EQUAL(window->height(), 460);
}

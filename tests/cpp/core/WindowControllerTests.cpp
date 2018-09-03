/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#define BOOST_TEST_MODULE WindowControllerTests

#include <boost/test/unit_test.hpp>

#include "control/WindowController.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

#include "DummyContent.h"

namespace
{
const QSizeF wallSize(1000, 1000);
const QSize CONTENT_SIZE(800, 600);
const QSize BIG_CONTENT_SIZE(CONTENT_SIZE*(Content::getMaxScale() + 1));
const QSize SMALL_CONTENT_SIZE(CONTENT_SIZE / 4);
const qreal CONTENT_AR =
    qreal(CONTENT_SIZE.width()) / qreal(CONTENT_SIZE.height());

ContentPtr make_dummy_content()
{
    return std::make_unique<DummyContent>(CONTENT_SIZE);
}
}

BOOST_AUTO_TEST_CASE(testResizeAndMove)
{
    Window window(make_dummy_content());
    const auto& content = window.getContent();

    auto displayGroup = DisplayGroup::create(wallSize);
    WindowController controller(window, *displayGroup);

    const QPointF targetPosition(124.2, 457.3);
    const QSizeF targetSize(0.7 * QSizeF(content.getDimensions()));

    controller.moveTo(targetPosition);
    controller.resize(targetSize);

    const QRectF& coords = window.getCoordinates();

    BOOST_CHECK_EQUAL(coords.topLeft(), targetPosition);
    BOOST_CHECK_EQUAL(coords.size(), targetSize);

    const QPointF targetCenterPosition(568.2, 389.0);
    controller.moveCenterTo(targetCenterPosition);

    BOOST_CHECK_EQUAL(coords.center(), targetCenterPosition);
    BOOST_CHECK_EQUAL(coords.size(), targetSize);

    const QPointF fixedCenter = coords.center();
    const QSizeF centeredSize(0.5 * QSizeF(content.getDimensions()));

    controller.resize(centeredSize, WindowPoint::CENTER);

    BOOST_CHECK_CLOSE(coords.center().x(), fixedCenter.x(), 0.00001);
    BOOST_CHECK_CLOSE(coords.center().y(), fixedCenter.y(), 0.00001);
    BOOST_CHECK_CLOSE(coords.width(), centeredSize.width(), 0.00001);
    BOOST_CHECK_CLOSE(coords.height(), centeredSize.height(), 0.00001);
}

BOOST_AUTO_TEST_CASE(testScaleByPixelDelta)
{
    Window window(make_dummy_content());

    auto displayGroup = DisplayGroup::create(wallSize);
    WindowController controller(window, *displayGroup);

    const QRectF& coords = window.getCoordinates();
    const auto pixelDelta = 40.0;

    controller.scale(QPointF(), pixelDelta);
    BOOST_CHECK_EQUAL(coords.height(), CONTENT_SIZE.height() + pixelDelta);
    BOOST_CHECK_EQUAL(coords.width(),
                      CONTENT_SIZE.width() + pixelDelta * CONTENT_AR);
    BOOST_CHECK_EQUAL(coords.x(), 0);
    BOOST_CHECK_EQUAL(coords.y(), 0);

    controller.scale(coords.bottomRight(), -pixelDelta);
    BOOST_CHECK_EQUAL(coords.size().width(), CONTENT_SIZE.width());
    BOOST_CHECK_EQUAL(coords.size().height(), CONTENT_SIZE.height());
    BOOST_CHECK_EQUAL(coords.y(), pixelDelta);
    BOOST_CHECK_CLOSE(coords.x(), pixelDelta * CONTENT_AR, 0.00001);
}

WindowPtr makeDummyWindow()
{
    auto window = std::make_shared<Window>(make_dummy_content());
    window->setCoordinates(QRectF(610, 220, 30, 40));

    const auto& coords = window->getCoordinates();
    BOOST_REQUIRE_EQUAL(coords.topLeft(), QPointF(610, 220));
    BOOST_REQUIRE_EQUAL(coords.center(), QPointF(625, 240));

    return window;
}

struct TestFixture
{
    WindowPtr window = makeDummyWindow();
    DisplayGroupPtr displayGroup = DisplayGroup::create(wallSize);
    WindowController controller{*window, *displayGroup};
};

BOOST_FIXTURE_TEST_CASE(testOneToOneSize, TestFixture)
{
    controller.adjustSize(SIZE_1TO1);

    // 1:1 size restored around existing window center
    const auto& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.size(), CONTENT_SIZE);
    BOOST_CHECK_EQUAL(coords.center(), QPointF(625, 240));
}

BOOST_FIXTURE_TEST_CASE(testOneToOneSizeUsesPreferedSize, TestFixture)
{
    deflect::SizeHints hints;
    hints.preferredWidth = CONTENT_SIZE.width() * 0.8;
    hints.preferredHeight = CONTENT_SIZE.height() * 1.1;
    window->getContent().setSizeHints(hints);

    controller.adjustSize(SIZE_1TO1);

    // 1:1 size restored around existing window center
    const auto& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.size(),
                      QSizeF(hints.preferredWidth, hints.preferredHeight));
    BOOST_CHECK_EQUAL(coords.center(), QPointF(625, 240));
}

BOOST_FIXTURE_TEST_CASE(testOneToOneFittingSize, TestFixture)
{
    controller.adjustSize(SIZE_1TO1_FITTING);

    // 1:1 size restored around existing window center
    const auto& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.size(), CONTENT_SIZE);
    BOOST_CHECK_EQUAL(coords.center(), QPointF(625, 240));

    // Big content constrained to 0.9 * wallSize
    window->getContent().setDimensions(2 * wallSize.toSize());
    controller.adjustSize(SIZE_1TO1_FITTING);
    BOOST_CHECK_EQUAL(coords.size(), 0.9 * wallSize);
    BOOST_CHECK_EQUAL(coords.center(), QPointF(625, 240));
}

BOOST_FIXTURE_TEST_CASE(testSizeLimitsBigContent, TestFixture)
{
    // Make a large content and validate it
    auto& content = window->getContent();
    content.setDimensions(BIG_CONTENT_SIZE);
    BOOST_REQUIRE_EQUAL(Content::getMaxScale(), 3.0);
    BOOST_REQUIRE_EQUAL(content.getMaxDimensions(),
                        BIG_CONTENT_SIZE * Content::getMaxScale());

    // Test controller and zoom limits
    BOOST_CHECK_EQUAL(controller.getMinSize(), QSize(300, 300));
    BOOST_CHECK_EQUAL(controller.getMaxSize(),
                      BIG_CONTENT_SIZE * Content::getMaxScale());
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(400, 300));

    const QSizeF normalMaxSize = controller.getMaxSize();
    window->getContent().setZoomRect(
        QRectF(QPointF(0.3, 0.1), QSizeF(0.25, 0.25)));
    BOOST_CHECK_EQUAL(controller.getMaxSize(), 0.25 * normalMaxSize);
}

BOOST_FIXTURE_TEST_CASE(testSizeLimitsSmallContent, TestFixture)
{
    // Make a small content and validate it
    auto& content = window->getContent();
    content.setDimensions(SMALL_CONTENT_SIZE);
    BOOST_REQUIRE_EQUAL(content.getMaxDimensions(),
                        SMALL_CONTENT_SIZE * Content::getMaxScale());
    BOOST_REQUIRE_EQUAL(Content::getMaxScale(), 3.0);

    // Test controller and zoom limits
    BOOST_CHECK_EQUAL(controller.getMinSize(), QSize(300, 300));
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(400, 300));
    BOOST_CHECK_EQUAL(controller.getMaxSize(),
                      SMALL_CONTENT_SIZE * Content::getMaxScale());

    const auto normalMaxSize = controller.getMaxSize();
    window->getContent().setZoomRect(
        QRectF(QPointF(0.3, 0.1), QSizeF(0.25, 0.25)));
    BOOST_CHECK_EQUAL(controller.getMaxSize(), 0.25 * normalMaxSize);
}

BOOST_FIXTURE_TEST_CASE(smallContentMadeFullscreenRespectsMaxContentSize,
                        TestFixture)
{
    auto& content = window->getContent();
    content.setDimensions(SMALL_CONTENT_SIZE);

    BOOST_REQUIRE(content.getMaxDimensions() < wallSize);

    controller.adjustSize(SizeState::SIZE_FULLSCREEN);

    BOOST_CHECK_EQUAL(window->getCoordinates().size(),
                      content.getMaxDimensions());
}

BOOST_FIXTURE_TEST_CASE(smallContentWithBigMaxSizeHintsCanBeMadeFullscreen,
                        TestFixture)
{
    auto& content = window->getContent();
    content.setDimensions(SMALL_CONTENT_SIZE);

    deflect::SizeHints hints;
    hints.maxWidth = std::numeric_limits<unsigned int>::max();
    hints.maxHeight = std::numeric_limits<unsigned int>::max();
    content.setSizeHints(hints);

    BOOST_REQUIRE_EQUAL(content.getMaxDimensions(),
                        QSize(std::numeric_limits<int>::max(),
                              std::numeric_limits<int>::max()));

    // Keep content aspect ratio
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    BOOST_CHECK_EQUAL(window->getCoordinates(),
                      QRectF(QPointF{0, 125}, QSizeF{1000, 750}));

    // Use preferred size aspect ratio
    hints.preferredWidth = wallSize.width() / 2;
    hints.preferredHeight = wallSize.height() / 2;
    content.setSizeHints(hints);
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    BOOST_CHECK_EQUAL(window->getCoordinates(),
                      QRectF(QPointF{0, 0}, wallSize));
}

BOOST_FIXTURE_TEST_CASE(testAspectRatioMinSize, TestFixture)
{
    // Make a content and validate MinSize keeps aspect ratio
    auto& content = window->getContent();
    content.setDimensions(QSize(400, 800));
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(300, 600));

    content.setDimensions(QSize(800, 1600));
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(300, 600));

    content.setDimensions(QSize(2000, 1500));
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(400, 300));

    window->setMode(Window::FULLSCREEN);
    content.setDimensions(QSize(800, 1600));
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(500, 1000));

    window->setMode(Window::STANDARD);
    content.setDimensions(QSize(800, 1600));
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(300, 600));

    window->setMode(Window::FULLSCREEN);
    content.setDimensions(QSize(2500, 1250));
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(1000, 500));

    window->setMode(Window::FOCUSED);
    content.setDimensions(QSize(2500, 1250));
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(600, 300));

    window->setMode(Window::STANDARD);
    const auto maxSize = QSize{CONTENT_SIZE * 2};
    const auto minSize = QSize{CONTENT_SIZE / 2};
    deflect::SizeHints hints;
    hints.maxWidth = maxSize.width();
    hints.maxHeight = maxSize.height();
    hints.minWidth = minSize.width();
    hints.minHeight = minSize.height();

    content.setSizeHints(hints);
    content.setDimensions(CONTENT_SIZE);

    controller.resize(maxSize, CENTER);
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(400, 300));

    window->setMode(Window::FULLSCREEN);
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(1000, 750));

    window->setMode(Window::FOCUSED);
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(400, 300));
}

BOOST_FIXTURE_TEST_CASE(testSizeHints, TestFixture)
{
    const auto maxSize = QSize{CONTENT_SIZE * 2};
    const auto minSize = QSize{CONTENT_SIZE / 2};
    deflect::SizeHints hints;
    hints.maxWidth = maxSize.width();
    hints.maxHeight = maxSize.height();
    hints.minWidth = minSize.width();
    hints.minHeight = minSize.height();
    hints.preferredWidth = CONTENT_SIZE.width();
    hints.preferredHeight = CONTENT_SIZE.height();
    auto& content = window->getContent();
    content.setSizeHints(hints);
    content.setDimensions(CONTENT_SIZE);
    const auto& coords = window->getCoordinates();

    // too big, constrains to maxSize
    controller.resize(maxSize * 2, CENTER);
    BOOST_CHECK_EQUAL(coords.size(), maxSize);

    // go back to preferred size
    controller.adjustSize(SIZE_1TO1);
    BOOST_CHECK_EQUAL(coords.size(), CONTENT_SIZE);

    // perfect max size
    controller.resize(maxSize, CENTER);
    BOOST_CHECK_EQUAL(coords.size(), maxSize);

    // too small, clamped to minSize
    controller.resize(minSize / 2, CENTER);
    BOOST_CHECK_EQUAL(coords.size(), minSize);
}

void _checkFullscreen(const QRectF& coords)
{
    // full screen, center on wall
    BOOST_CHECK_EQUAL(coords.x(), 0.0);
    BOOST_CHECK_EQUAL(coords.y(), 125.0);
    BOOST_CHECK_EQUAL(coords.width(), wallSize.width());
    BOOST_CHECK_EQUAL(coords.height(), wallSize.width() / CONTENT_AR);
}

void _checkFullscreenMax(const QRectF& coords)
{
    // full screen maximized, centered on wall
    BOOST_CHECK_CLOSE(coords.x(), -166.66666, 0.00001);
    BOOST_CHECK_EQUAL(coords.y(), 0.0);
    BOOST_CHECK_EQUAL(coords.width(), wallSize.height() * CONTENT_AR);
    BOOST_CHECK_EQUAL(coords.height(), wallSize.height());
}

struct ZoomedContentFixture
{
    Window window{_makeZoomedContent()};
    DisplayGroupPtr displayGroup = DisplayGroup::create(wallSize);
    WindowController controller{window, *displayGroup};

private:
    ContentPtr _makeZoomedContent()
    {
        auto content = make_dummy_content();
        content->setZoomRect(QRectF(0.25, 0.25, 0.5, 0.5));
        return content;
    }
};

BOOST_FIXTURE_TEST_CASE(testFullScreenSize, ZoomedContentFixture)
{
    controller.adjustSize(SIZE_FULLSCREEN);
    _checkFullscreen(window.getCoordinates());
    // zoom reset
    BOOST_CHECK_EQUAL(window.getContent().getZoomRect(), UNIT_RECTF);
}

BOOST_FIXTURE_TEST_CASE(testFullScreenMaxSize, ZoomedContentFixture)
{
    controller.adjustSize(SIZE_FULLSCREEN_MAX);
    _checkFullscreenMax(window.getCoordinates());
    // zoom reset
    BOOST_CHECK_EQUAL(window.getContent().getZoomRect(), UNIT_RECTF);
}

BOOST_FIXTURE_TEST_CASE(testToggleFullScreenMaxSize, ZoomedContentFixture)
{
    // No effect if window is not in fullscreen
    const auto originalCoord = window.getDisplayCoordinates();
    controller.toogleFullscreenMaxSize();
    BOOST_CHECK_EQUAL(originalCoord, window.getDisplayCoordinates());

    // Toggle between two modes when in fullscreen
    window.setMode(Window::WindowMode::FULLSCREEN);
    controller.toogleFullscreenMaxSize();
    _checkFullscreenMax(window.getFullscreenCoordinates());
    controller.toogleFullscreenMaxSize();
    _checkFullscreen(window.getFullscreenCoordinates());
    controller.toogleFullscreenMaxSize();
    _checkFullscreenMax(window.getFullscreenCoordinates());
    controller.toogleFullscreenMaxSize();
    _checkFullscreen(window.getFullscreenCoordinates());
}

BOOST_FIXTURE_TEST_CASE(testResizeAndMoveInFullScreenMode, ZoomedContentFixture)
{
    window.setMode(Window::WindowMode::FULLSCREEN);
    controller.adjustSize(SIZE_FULLSCREEN);
    _checkFullscreen(window.getDisplayCoordinates());

    // Window cannot move or scale down while in "FULLSCREEN"
    controller.moveBy({10.0, 20.0});
    controller.scale(window.getDisplayCoordinates().center(), -15.0);
    _checkFullscreen(window.getDisplayCoordinates());

    // Window can only move along the direction where it exceeds the displaywall
    controller.adjustSize(SIZE_FULLSCREEN_MAX);
    _checkFullscreenMax(window.getDisplayCoordinates());
    controller.moveBy({100.0, 100.0});
    const auto& coords = window.getDisplayCoordinates();
    BOOST_CHECK_CLOSE(coords.x(), -66.66666, 0.0001);
    BOOST_CHECK_EQUAL(coords.y(), 0.0);
    controller.moveBy({100.0, 100.0});
    BOOST_CHECK_EQUAL(coords.x(), 0.0);
    BOOST_CHECK_EQUAL(coords.y(), 0.0);

    // Window goes back to "FULLSCREEN" coordinates when scaling it down
    controller.scale(window.getDisplayCoordinates().center(), -500.0);
    _checkFullscreen(window.getDisplayCoordinates());
}

BOOST_AUTO_TEST_CASE(testResizeRelativeToBorder)
{
    Window window(make_dummy_content());
    auto displayGroup = DisplayGroup::create(wallSize);
    WindowController controller(window, *displayGroup);

    const auto originalCoords = window.getCoordinates();

    controller.resizeRelative(QPointF(5, 5));
    BOOST_CHECK(window.getCoordinates() == originalCoords);

    // These border resize conserve the window aspect ratio
    window.setActiveHandle(Window::TOP);
    controller.resizeRelative(QPointF(5, 5));
    BOOST_CHECK_EQUAL(window.getCoordinates().top() - 5, originalCoords.top());
    BOOST_CHECK_EQUAL(window.getCoordinates().width() + 5.0 * CONTENT_AR,
                      originalCoords.width());

    window.setActiveHandle(Window::BOTTOM);
    controller.resizeRelative(QPointF(2, 2));
    BOOST_CHECK_EQUAL(window.getCoordinates().bottom() - 2,
                      originalCoords.bottom());
    BOOST_CHECK_EQUAL(window.getCoordinates().width() + 3.0 * CONTENT_AR,
                      originalCoords.width());
}

BOOST_AUTO_TEST_CASE(testResizeRelativeToCorner)
{
    Window window(make_dummy_content());
    auto displayGroup = DisplayGroup::create(wallSize);
    WindowController controller(window, *displayGroup);

    const auto originalCoords = window.getCoordinates();

    // These corner resize alters the window aspect ratio
    static_cast<DummyContent&>(window.getContent()).fixedAspectRatio = false;
    BOOST_REQUIRE(window.setResizePolicy(Window::ADJUST_CONTENT));
    window.setActiveHandle(Window::TOP_RIGHT);
    controller.resizeRelative(QPointF(2, 10));
    BOOST_CHECK_EQUAL(window.getCoordinates().top() - 10, originalCoords.top());
    BOOST_CHECK_EQUAL(window.getCoordinates().right() - 2,
                      originalCoords.right());
    BOOST_CHECK_EQUAL(window.getCoordinates().height() + 10,
                      originalCoords.height());
    BOOST_CHECK_EQUAL(window.getCoordinates().width() - 2,
                      originalCoords.width());

    const auto prevCoords = window.getCoordinates();

    window.setActiveHandle(Window::BOTTOM_LEFT);
    controller.resizeRelative(QPointF(1, 2));
    BOOST_CHECK_EQUAL(window.getCoordinates().bottom() - 2,
                      prevCoords.bottom());
    BOOST_CHECK_EQUAL(window.getCoordinates().left() - 1, prevCoords.left());
    BOOST_CHECK_EQUAL(window.getCoordinates().height() - 2,
                      prevCoords.height());
    BOOST_CHECK_EQUAL(window.getCoordinates().width() + 1, prevCoords.width());
}

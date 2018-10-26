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
#include "control/WindowResizeHandlesController.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

#include "DummyContent.h"

namespace
{
constexpr QSizeF wallSize(1000, 1000);
constexpr QSize CONTENT_SIZE(800, 600);
constexpr qreal maxDelta = 0.00001;
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

    BOOST_CHECK_CLOSE(coords.center().x(), fixedCenter.x(), maxDelta);
    BOOST_CHECK_CLOSE(coords.center().y(), fixedCenter.y(), maxDelta);
    BOOST_CHECK_CLOSE(coords.width(), centeredSize.width(), maxDelta);
    BOOST_CHECK_CLOSE(coords.height(), centeredSize.height(), maxDelta);
}

BOOST_AUTO_TEST_CASE(testScaleByPixelDelta)
{
    Window window(make_dummy_content());

    auto displayGroup = DisplayGroup::create(wallSize);
    WindowController controller(window, *displayGroup);

    const QRectF& coords = window.getCoordinates();
    const auto pixelDelta = 40.0;

    controller.scale(QPointF(), pixelDelta);
    BOOST_CHECK_CLOSE(coords.height(), CONTENT_SIZE.height() + pixelDelta,
                      maxDelta);
    BOOST_CHECK_CLOSE(coords.width(),
                      CONTENT_SIZE.width() + pixelDelta * CONTENT_AR, maxDelta);
    BOOST_CHECK_EQUAL(coords.x(), 0);
    BOOST_CHECK_EQUAL(coords.y(), 0);

    controller.scale(coords.bottomRight(), -pixelDelta);
    BOOST_CHECK_CLOSE(coords.size().width(), CONTENT_SIZE.width(), maxDelta);
    BOOST_CHECK_CLOSE(coords.size().height(), CONTENT_SIZE.height(), maxDelta);
    BOOST_CHECK_CLOSE(coords.y(), pixelDelta, maxDelta);
    BOOST_CHECK_CLOSE(coords.x(), pixelDelta * CONTENT_AR, maxDelta);
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
    BOOST_CHECK(!window->getContent().isZoomed());
}

BOOST_FIXTURE_TEST_CASE(testOneToOneSizeUsesPreferedSize, TestFixture)
{
    deflect::SizeHints hints;
    hints.preferredWidth = CONTENT_SIZE.width() * 0.8;
    hints.preferredHeight = CONTENT_SIZE.height() * 1.1;
    hints.maxWidth = CONTENT_SIZE.width() * 2;
    hints.maxHeight = CONTENT_SIZE.height() * 2;
    window->getContent().setSizeHints(hints);
    static_cast<DummyContent&>(window->getContent()).fixedAspectRatio = false;
    static_cast<DummyContent&>(window->getContent()).zoomable = false;

    controller.adjustSize(SIZE_1TO1);

    // 1:1 size restored around existing window center
    const auto& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.size(),
                      QSizeF(hints.preferredWidth, hints.preferredHeight));
    BOOST_CHECK_EQUAL(coords.center(), QPointF(625, 240));
    BOOST_CHECK(!window->getContent().isZoomed());
}

BOOST_FIXTURE_TEST_CASE(testOneToOneFittingSize, TestFixture)
{
    controller.adjustSize(SIZE_1TO1_FITTING);

    // 1:1 size restored around existing window center
    const auto& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.size(), CONTENT_SIZE);
    BOOST_CHECK_EQUAL(coords.center(), QPointF(625, 240));
    BOOST_CHECK(!window->getContent().isZoomed());

    // Big content constrained to 0.9 * wallSize
    window->getContent().setDimensions(2 * wallSize.toSize());
    controller.adjustSize(SIZE_1TO1_FITTING);
    BOOST_CHECK_EQUAL(coords.size(), 0.9 * wallSize);
    BOOST_CHECK_EQUAL(coords.center(), QPointF(625, 240));
    BOOST_CHECK(!window->getContent().isZoomed());

    // And without zooming the content if the aspect ratio is different
    const auto bigSize =
        QSize{4 * wallSize.toSize().width(), 2 * wallSize.toSize().height()};
    const auto expectedSize =
        QSizeF{0.9 * wallSize.width(), 0.45 * wallSize.height()};
    window->getContent().setDimensions(bigSize);
    controller.adjustSize(SIZE_1TO1_FITTING);
    BOOST_CHECK_EQUAL(coords.size(), expectedSize);
    BOOST_CHECK_EQUAL(coords.center(), QPointF(625, 240));
    BOOST_CHECK(!window->getContent().isZoomed());
}

BOOST_FIXTURE_TEST_CASE(testSizeLimitsBigContent, TestFixture)
{
    // Make a large content and validate it
    auto& content = window->getContent();
    content.setDimensions(BIG_CONTENT_SIZE);
    BOOST_REQUIRE_EQUAL(Content::getMaxScale(), 3.0);
    BOOST_REQUIRE_EQUAL(content.getMaxDimensions(), BIG_CONTENT_SIZE);
    BOOST_REQUIRE_EQUAL(content.getMaxUpscaledDimensions(),
                        BIG_CONTENT_SIZE * Content::getMaxScale());

    // Test controller and zoom limits
    BOOST_CHECK_EQUAL(controller.getMinSize(), QSize(300, 300));
    BOOST_CHECK_EQUAL(controller.getMaxSize(),
                      BIG_CONTENT_SIZE * Content::getMaxScale());
    BOOST_CHECK_EQUAL(controller.getMinSizeAspectRatioCorrect(),
                      QSize(400, 300));

    const auto normalMaxSize = controller.getMaxSize();
    window->getContent().setZoomRect(
        QRectF(QPointF(0.3, 0.1), QSizeF(0.25, 0.25)));
    BOOST_CHECK_EQUAL(controller.getMaxSize(), 0.25 * normalMaxSize);
}

BOOST_FIXTURE_TEST_CASE(testSizeLimitsSmallContent, TestFixture)
{
    // Make a small content and validate it
    auto& content = window->getContent();
    content.setDimensions(SMALL_CONTENT_SIZE);
    BOOST_REQUIRE_EQUAL(content.getMaxDimensions(), SMALL_CONTENT_SIZE);
    BOOST_REQUIRE_EQUAL(content.getMaxUpscaledDimensions(),
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

    window->setMode(Window::WindowMode::FULLSCREEN);
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    BOOST_REQUIRE(window->isFullscreen());

    BOOST_CHECK_EQUAL(window->getDisplayCoordinates().size(),
                      content.getMaxUpscaledDimensions());
    BOOST_CHECK(!window->getContent().isZoomed());

    // Can't enlarge or reduce the window beyond its maximum size
    controller.scale(window->getDisplayCoordinates().center(), 5.0);
    BOOST_CHECK_EQUAL(window->getDisplayCoordinates().size(),
                      content.getMaxUpscaledDimensions());

    controller.scale(window->getDisplayCoordinates().center(), -5.0);
    BOOST_CHECK_EQUAL(window->getDisplayCoordinates().size(),
                      content.getMaxUpscaledDimensions());
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
    static_cast<DummyContent&>(content).zoomable = false;

    BOOST_REQUIRE_EQUAL(content.getMaxDimensions(),
                        QSize(std::numeric_limits<int>::max(),
                              std::numeric_limits<int>::max()));

    // Keep content aspect ratio
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    BOOST_CHECK_EQUAL(window->getCoordinates(),
                      QRectF(QPointF{0, 125}, QSizeF{1000, 750}));
    BOOST_CHECK(!window->getContent().isZoomed());

    // Always enforce content aspect ratio over preferred size if it is fixed
    hints.preferredWidth = wallSize.width() / 2;
    hints.preferredHeight = wallSize.height() / 2;
    content.setSizeHints(hints);
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    BOOST_CHECK_EQUAL(window->getCoordinates(),
                      QRectF(QPointF{0, 125}, QSizeF{1000, 750}));
    BOOST_CHECK(!window->getContent().isZoomed());

    // But use preferred size aspect ratio if it can change freely
    static_cast<DummyContent&>(window->getContent()).fixedAspectRatio = false;
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    BOOST_CHECK_EQUAL(window->getCoordinates(),
                      QRectF(QPointF{0, 0}, wallSize));
    BOOST_CHECK(!window->getContent().isZoomed());
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

void _checkFullscreen(const QRectF& coords)
{
    // full screen, center on wall
    BOOST_CHECK_EQUAL(coords.x(), 0.0);
    BOOST_CHECK_EQUAL(coords.y(), 125.0);
    BOOST_CHECK_EQUAL(coords.width(), wallSize.width());
    BOOST_CHECK_EQUAL(coords.height(), wallSize.width() / CONTENT_AR);
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
    static_cast<DummyContent&>(content).zoomable = false;

    const auto& coords = window->getCoordinates();

    // too big => constrains to maxSize * upscaling factor
    controller.resize(maxSize * Content::getMaxScale() * 1.5, CENTER);
    BOOST_CHECK_EQUAL(coords.size(), maxSize * Content::getMaxScale());

    // go back to preferred size
    controller.adjustSize(SIZE_1TO1);
    BOOST_CHECK_EQUAL(coords.size(), CONTENT_SIZE);

    // perfect max size * upscaling factor
    controller.resize(maxSize * Content::getMaxScale(), CENTER);
    BOOST_CHECK_EQUAL(coords.size(), maxSize * Content::getMaxScale());

    // too small, clamped to minSize
    controller.resize(minSize / 2, CENTER);
    BOOST_CHECK_EQUAL(coords.size(), minSize);

    // fullscreen fits according to prefered size
    window->setWidth(CONTENT_SIZE.width() * 4);
    window->setWidth(CONTENT_SIZE.height() * 2);
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    _checkFullscreen(coords);
    BOOST_CHECK(!window->getContent().isZoomed());

    // but content aspect ratio is presered if preferred size is inconsitent...
    hints.preferredWidth = CONTENT_SIZE.width();
    hints.preferredHeight = CONTENT_SIZE.width() * 2;
    content.setSizeHints(hints);
    window->setWidth(CONTENT_SIZE.width() / 4);
    window->setWidth(CONTENT_SIZE.height() / 2);
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    _checkFullscreen(coords);
    BOOST_CHECK(!window->getContent().isZoomed());

    // ...unless content aspect ratio can vary freely
    static_cast<DummyContent&>(window->getContent()).fixedAspectRatio = false;
    controller.adjustSize(SizeState::SIZE_FULLSCREEN);
    BOOST_CHECK_EQUAL(coords.x(), (wallSize.height() - coords.width()) / 2);
    BOOST_CHECK_EQUAL(coords.y(), 0.0);
    BOOST_CHECK_EQUAL(coords.width(), wallSize.height() / 2);
    BOOST_CHECK_EQUAL(coords.height(), wallSize.height());
    BOOST_CHECK(!window->getContent().isZoomed());
}

void _checkFullscreenMax(const QRectF& coords)
{
    // full screen maximized, centered on wall
    BOOST_CHECK_CLOSE(coords.x(), -166.66666, maxDelta);
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
    BOOST_CHECK(!window.getContent().isZoomed());
}

BOOST_FIXTURE_TEST_CASE(testFullScreenMaxSize, ZoomedContentFixture)
{
    controller.adjustSize(SIZE_FULLSCREEN_MAX);
    _checkFullscreenMax(window.getCoordinates());
    // zoom reset
    BOOST_CHECK(!window.getContent().isZoomed());
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

struct ResizeHandlesFixture
{
    Window window{make_dummy_content()};
    DisplayGroupPtr displayGroup{DisplayGroup::create(wallSize)};
    WindowResizeHandlesController controller{window, *displayGroup};
    const QRectF originalCoords{window.getCoordinates()};

    void enableFreeResizeMode()
    {
        auto& content = static_cast<DummyContent&>(window.getContent());
        content.fixedAspectRatio = false;
        content.zoomable = false;
        BOOST_REQUIRE(window.setResizePolicy(Window::ADJUST_CONTENT));
    }
};

BOOST_FIXTURE_TEST_CASE(testResizeHandlesWithFixedAspectRatio,
                        ResizeHandlesFixture)
{
    const auto& coord = window.getCoordinates();

    BOOST_REQUIRE(window.setResizePolicy(Window::KEEP_ASPECT_RATIO));

    controller.resizeRelative(QPointF(5.0, 5.0));
    BOOST_CHECK(coord == originalCoords);

    window.setActiveHandle(Window::TOP);
    controller.resizeRelative(QPointF(5.0, 5.0));
    BOOST_CHECK_CLOSE(coord.top(), originalCoords.top() + 5.0, maxDelta);
    BOOST_CHECK_CLOSE(coord.width(), originalCoords.width() - 5.0 * CONTENT_AR,
                      maxDelta);

    window.setActiveHandle(Window::BOTTOM);
    controller.resizeRelative(QPointF(2.0, 2.0));
    BOOST_CHECK_CLOSE(coord.bottom(), originalCoords.bottom() + 2.0, maxDelta);
    BOOST_CHECK_CLOSE(coord.width(), originalCoords.width() - 3.0 * CONTENT_AR,
                      maxDelta);
}

BOOST_FIXTURE_TEST_CASE(testResizeHandlesWithFreeAspectRatio,
                        ResizeHandlesFixture)
{
    const auto& coord = window.getCoordinates();

    // reduce window size towards bottom-left (modifies aspect ratio)
    enableFreeResizeMode();
    window.setActiveHandle(Window::TOP_RIGHT);
    controller.resizeRelative(QPointF(-2.0, 10.0));

    BOOST_CHECK_EQUAL(coord.top(), originalCoords.top() + 10.0);
    BOOST_CHECK_EQUAL(coord.right(), originalCoords.right() - 2.0);
    BOOST_CHECK_EQUAL(coord.height(), originalCoords.height() - 10.0);
    BOOST_CHECK_EQUAL(coord.width(), originalCoords.width() - 2.0);

    const auto prevCoords = window.getCoordinates();

    window.setActiveHandle(Window::BOTTOM_LEFT);
    controller.resizeRelative(QPointF(1.0, 2.0));
    BOOST_CHECK_EQUAL(coord.bottom(), prevCoords.bottom() + 2.0);
    BOOST_CHECK_EQUAL(coord.left(), prevCoords.left() + 1.0);
    BOOST_CHECK_EQUAL(coord.height(), prevCoords.height() + 2.0);
    BOOST_CHECK_EQUAL(coord.width(), prevCoords.width() - 1.0);
}

BOOST_FIXTURE_TEST_CASE(testResizeHandlesKeepsAspectRatioWhenExceedingMaxSize,
                        ResizeHandlesFixture)
{
    const auto& coord = window.getCoordinates();

    // enlarge towards top-right (keeps aspect ratio as it exceeds content size)
    enableFreeResizeMode();
    window.setActiveHandle(Window::TOP_RIGHT);
    controller.resizeRelative(QPointF(2.0, -10.0));

    BOOST_CHECK_EQUAL(coord.top(), originalCoords.top() - 10.0);
    BOOST_CHECK_CLOSE(coord.right(), originalCoords.right() + 10.0 * CONTENT_AR,
                      maxDelta);
    BOOST_CHECK_CLOSE(coord.height(), originalCoords.height() + 10.0, maxDelta);
    BOOST_CHECK_CLOSE(coord.width(), originalCoords.width() + 10.0 * CONTENT_AR,
                      maxDelta);
}

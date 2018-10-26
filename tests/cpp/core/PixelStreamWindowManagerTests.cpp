/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
/*                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>*/
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

#define BOOST_TEST_MODULE PixelStreamWindowManagerTests
#include <boost/test/unit_test.hpp>

#include <deflect/server/EventReceiver.h>
#include <deflect/server/Frame.h>

#include "config.h"
#include "control/DisplayGroupController.h"
#include "control/PixelStreamController.h"
#include "control/PixelStreamWindowManager.h"
#include "scene/Options.h"
#include "scene/PixelStreamContent.h"
#include "scene/Scene.h"
#include "scene/Window.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

namespace
{
const QString CONTENT_URI("bla");
const QString PANEL_URI("Launcher");
const QSize wallSize(1000, 1000);
const QSize defaultPixelStreamWindowSize(640, 480);
const QSize testWindowSize(500, 400);
const QPointF testWindowPos(400.0, 300.0);
const QSize testFrameSize(600, 500);
const QSize testFrameSize2(700, 600);

deflect::SizeHints makeSizeHints(const QSize& size)
{
    deflect::SizeHints hints;
    hints.preferredWidth = size.width();
    hints.preferredHeight = size.height();
    return hints;
}

deflect::server::Tile createTile(const QSize& size, const uint channel)
{
    deflect::server::Tile tile;
    tile.width = size.width();
    tile.height = size.height();
    tile.channel = channel;
    return tile;
}

deflect::server::FramePtr createTestFrame(const QSize& size,
                                          const QString& uri = CONTENT_URI)
{
    auto frame = std::make_shared<deflect::server::Frame>();
    frame->uri = uri;
    frame->tiles.push_back(createTile(size, 0));
    return frame;
}

deflect::server::FramePtr createTestFrame(const QSize& channel0Size,
                                          const QSize& channel1Size)
{
    auto frame = createTestFrame(channel0Size);
    frame->tiles.push_back(createTile(channel1Size, 1));
    return frame;
}

class DummyEventReceiver : public deflect::server::EventReceiver
{
public:
    DummyEventReceiver()
        : success(false)
    {
    }
    virtual void processEvent(deflect::Event /*event*/) { success = true; }
    bool success;
};
}

BOOST_AUTO_TEST_CASE(testNoStreamerWindowCreation)
{
    auto scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager(*scene);

    const auto& uri = CONTENT_URI;

    windowManager.openWindow(0, uri, testWindowSize, testWindowPos);
    auto window = windowManager.getWindows(uri)[0];
    BOOST_REQUIRE(window);

    BOOST_CHECK_EQUAL(window, windowManager.getWindows(uri)[0]);

    const QRectF& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.size(), testWindowSize);
    BOOST_CHECK_EQUAL(coords.center(), testWindowPos);

    windowManager.handleStreamEnd(uri);
    BOOST_CHECK(windowManager.getWindows(uri).empty());
}

BOOST_AUTO_TEST_CASE(requestFirstFrameSignalIsEmittedOnlyForAlreadyOpenWindows)
{
    auto scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager(*scene);

    uint requestFirstFrameEmitted = 0;
    QObject::connect(&windowManager,
                     &PixelStreamWindowManager::requestFirstFrame,
                     [&requestFirstFrameEmitted]() {
                         ++requestFirstFrameEmitted;
                     });

    windowManager.openWindow(0, CONTENT_URI, testWindowSize, testWindowPos);

    BOOST_CHECK(!requestFirstFrameEmitted);

    windowManager.handleStreamStart("ExternalStream");

    BOOST_CHECK(!requestFirstFrameEmitted);

    windowManager.handleStreamStart(CONTENT_URI);

    BOOST_CHECK(requestFirstFrameEmitted == 1);
}

BOOST_AUTO_TEST_CASE(testEventReceiver)
{
    auto scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager(*scene);
    windowManager.openWindow(0, CONTENT_URI, testWindowSize, testWindowPos);
    auto window = windowManager.getWindows(CONTENT_URI)[0];
    BOOST_REQUIRE(window);

    auto& content = dynamic_cast<PixelStreamContent&>(window->getContent());
    BOOST_REQUIRE(!content.hasEventReceivers());
    BOOST_REQUIRE(!content.getCaptureInteraction());

    DummyEventReceiver receiver;
    BOOST_REQUIRE(!receiver.success);

    auto controller = ContentController::create(*window);
    auto streamController =
        dynamic_cast<PixelStreamController*>(controller.get());
    BOOST_REQUIRE(streamController);

    streamController->notify(deflect::Event());
    BOOST_CHECK(!receiver.success);

    auto promise = std::make_shared<std::promise<bool>>();
    windowManager.registerEventReceiver(content.getUri(), false, &receiver,
                                        promise);
    BOOST_CHECK(promise->get_future().get());
    BOOST_CHECK(content.hasEventReceivers());
    BOOST_CHECK(content.getCaptureInteraction());
    BOOST_CHECK(!receiver.success);

    streamController->notify(deflect::Event());
    BOOST_CHECK(receiver.success);
}

BOOST_AUTO_TEST_CASE(testExplicitWindowCreation)
{
    auto scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager(*scene);
    auto& group = scene->getGroup(0);

    const auto& uri = CONTENT_URI;

    windowManager.openWindow(0, uri, testWindowSize, testWindowPos);
    auto window = windowManager.getWindows(uri)[0];
    BOOST_REQUIRE(window);

    BOOST_CHECK_EQUAL(window, windowManager.getWindows(uri)[0]);
    BOOST_CHECK_EQUAL(window, group.getWindow(window->getID()));

    windowManager.handleStreamStart(uri);
    BOOST_CHECK_EQUAL(window, windowManager.getWindows(uri)[0]);

    const auto& content = window->getContent();
    BOOST_CHECK_EQUAL(content.getUri(), uri);
    BOOST_CHECK_EQUAL(content.getType(), ContentType::pixel_stream);

    const QRectF& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.size(), testWindowSize);
    BOOST_CHECK_EQUAL(coords.center(), testWindowPos);

    // Check that the window is NOT resized to the first frame dimensions
    windowManager.updateStreamWindows(createTestFrame(testFrameSize));
    BOOST_CHECK_EQUAL(content.getDimensions(), testFrameSize);
    BOOST_CHECK_EQUAL(coords.size(), testWindowSize);
    BOOST_CHECK_EQUAL(coords.center(), testWindowPos);

    windowManager.handleStreamEnd(uri);
    BOOST_CHECK(windowManager.getWindows(uri).empty());
    BOOST_CHECK(!group.getWindow(window->getID()));
}

BOOST_AUTO_TEST_CASE(testImplicitWindowCreation)
{
    auto scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager(*scene);
    auto& group = scene->getGroup(0);

    const auto& uri = CONTENT_URI;
    // window will be positioned centered
    const auto pos = QPointF(wallSize.width() * 0.5, wallSize.height() * 0.5);

    windowManager.handleStreamStart(uri);
    BOOST_REQUIRE(windowManager.getWindows(uri).size() == 1);
    auto window = windowManager.getWindows(uri)[0];
    BOOST_CHECK_EQUAL(window, group.getWindow(window->getID()));

    const auto& content = window->getContent();
    BOOST_CHECK(content.getUri() == uri);
    BOOST_CHECK_EQUAL(content.getType(), ContentType::pixel_stream);
    BOOST_CHECK_EQUAL(content.getDimensions(), UNDEFINED_SIZE);

    const auto& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), defaultPixelStreamWindowSize);

    // Check that the window is resized to the first frame dimensions
    windowManager.updateStreamWindows(createTestFrame(testFrameSize));
    BOOST_CHECK_EQUAL(content.getDimensions(), testFrameSize);
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), testFrameSize);

    // Check that the window is NOT resized to the next frame dimensions
    windowManager.updateStreamWindows(createTestFrame(testFrameSize2));
    BOOST_CHECK_EQUAL(content.getDimensions(), testFrameSize2);
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), testFrameSize);

    windowManager.handleStreamEnd(uri);
    BOOST_CHECK(windowManager.getWindows(uri).empty());
    BOOST_CHECK(!group.getWindow(window->getID()));
}

BOOST_AUTO_TEST_CASE(testSizeHints)
{
    auto scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager(*scene);

    const auto& uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);
    auto window = windowManager.getWindows(uri)[0];
    const auto& content = window->getContent();

    BOOST_CHECK_EQUAL(content.getDimensions(), QSizeF());
    BOOST_CHECK_EQUAL(content.getMinDimensions(), QSizeF());
    BOOST_CHECK_EQUAL(content.getMaxDimensions(), QSizeF());
    BOOST_CHECK_EQUAL(content.getPreferredDimensions(), QSizeF());

    const QSize minSize(80, 100);
    const QSize maxSize(800, 1000);
    const QSize preferredSize(400, 500);
    deflect::SizeHints hints;
    hints.minWidth = minSize.width();
    hints.minHeight = minSize.height();
    hints.maxWidth = maxSize.width();
    hints.maxHeight = maxSize.height();
    hints.preferredWidth = preferredSize.width();
    hints.preferredHeight = preferredSize.height();
    windowManager.updateSizeHints(CONTENT_URI, hints);

    BOOST_CHECK_EQUAL(content.getDimensions(), preferredSize);
    BOOST_CHECK_EQUAL(content.getMinDimensions(), minSize);
    BOOST_CHECK_EQUAL(content.getMaxDimensions(), maxSize);
    BOOST_CHECK_EQUAL(content.getPreferredDimensions(), preferredSize);
}

BOOST_AUTO_TEST_CASE(hideAndShowWindows)
{
    auto scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager(*scene);

    const auto uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);
    auto window = windowManager.getWindows(uri)[0];

    BOOST_REQUIRE(window->isHidden());
    BOOST_REQUIRE(!window->isPanel());

    windowManager.showWindows(uri);
    BOOST_CHECK(!window->isHidden());

    windowManager.hideWindows(uri);
    BOOST_CHECK(window->isHidden());
}

BOOST_AUTO_TEST_CASE(hideAndShowPanel)
{
    auto scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager(*scene);

    const auto& uri = PANEL_URI;
    windowManager.openWindow(0, uri, testFrameSize, QPointF(),
                             StreamType::LAUNCHER);
    windowManager.handleStreamStart(uri);
    auto panel = windowManager.getWindows(uri)[0];

    BOOST_REQUIRE(panel->isPanel());
    BOOST_REQUIRE(!panel->isHidden());

    windowManager.hideWindows(uri);
    BOOST_CHECK(panel->isHidden());
    windowManager.showWindows(uri);
    BOOST_CHECK(!panel->isHidden());

    auto promise = std::make_shared<std::promise<bool>>();
    DummyEventReceiver receiver;
    windowManager.registerEventReceiver(uri, false, &receiver, promise);

    windowManager.hideWindows(uri);
    BOOST_CHECK(panel->isHidden());

    windowManager.showWindows(uri);
    BOOST_CHECK(!panel->isHidden());
}

struct SingleSurface
{
    ScenePtr scene = Scene::create(wallSize);
    PixelStreamWindowManager windowManager{*scene};

    bool externalStreamOpened = false;

    SingleSurface()
    {
        QObject::connect(&windowManager,
                         &PixelStreamWindowManager::externalStreamOpening,
                         [this]() { externalStreamOpened = true; });
    }
};

BOOST_FIXTURE_TEST_CASE(external_stream_signal_open_after_sizehints,
                        SingleSurface)
{
    const auto& uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);

    const auto window = windowManager.getWindows(uri)[0];

    BOOST_CHECK(!externalStreamOpened);
    BOOST_CHECK(window->isHidden());

    const auto expectedDimensions = QSize{400, 300};
    windowManager.updateSizeHints(uri, makeSizeHints(expectedDimensions));

    BOOST_CHECK(externalStreamOpened);
    BOOST_CHECK(window->isHidden());
    BOOST_CHECK_EQUAL(window->getContent().getDimensions(), expectedDimensions);
}

BOOST_FIXTURE_TEST_CASE(check_external_stream_signal_open_after_firstframe,
                        SingleSurface)
{
    const auto& uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);

    const auto window = windowManager.getWindows(uri)[0];

    BOOST_CHECK_EQUAL(externalStreamOpened, false);
    BOOST_CHECK(window->isHidden());

    const auto expectedDimensions = QSize{400, 300};
    windowManager.updateStreamWindows(createTestFrame(expectedDimensions));

    BOOST_CHECK_EQUAL(externalStreamOpened, true);
    BOOST_CHECK(window->isHidden());
    BOOST_CHECK_EQUAL(window->getContent().getDimensions(), expectedDimensions);
}

BOOST_FIXTURE_TEST_CASE(stream_content_size_adjusts_to_frame_size,
                        SingleSurface)
{
    const auto& uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);

    const auto window = windowManager.getWindows(uri)[0];
    const auto& content = window->getContent();

    const auto expectedDimensions = QSize{400, 300};
    windowManager.updateSizeHints(uri, makeSizeHints(expectedDimensions));

    BOOST_REQUIRE_EQUAL(content.getDimensions(), expectedDimensions);

    const auto newDimensions = QSize{200, 200};
    windowManager.updateStreamWindows(createTestFrame(newDimensions));

    BOOST_CHECK_EQUAL(content.getDimensions(), newDimensions);

    const auto newDimensions2 = QSize{400, 300};
    windowManager.updateStreamWindows(createTestFrame(newDimensions2));

    BOOST_CHECK_EQUAL(content.getDimensions(), newDimensions2);
}

BOOST_FIXTURE_TEST_CASE(local_streams_open_without_validation_signal,
                        SingleSurface)
{
    windowManager.openWindow(0, PANEL_URI, testFrameSize, QPointF(),
                             StreamType::LAUNCHER);
    windowManager.handleStreamStart(PANEL_URI);

    const auto webbrowserUri = "Webbrowser_0";
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    const auto type = StreamType::WEBBROWSER;
#else
    const auto type = StreamType::WHITEBOARD;
#endif

    windowManager.openWindow(0, webbrowserUri, testFrameSize, QPointF(), type);
    windowManager.handleStreamStart(webbrowserUri);

    const auto whiteboardUri = "Whiteboard_0";
    windowManager.openWindow(0, whiteboardUri, testFrameSize, QPointF(),
                             StreamType::WHITEBOARD);
    windowManager.handleStreamStart(whiteboardUri);

    BOOST_CHECK(!externalStreamOpened);
    BOOST_CHECK(!windowManager.getWindows(PANEL_URI).at(0)->isHidden());
    BOOST_CHECK(!windowManager.getWindows(webbrowserUri).at(0)->isHidden());
    BOOST_CHECK(!windowManager.getWindows(whiteboardUri).at(0)->isHidden());
}

BOOST_FIXTURE_TEST_CASE(large_stream_is_sized_to_fit_displaygroup_when_opening,
                        SingleSurface)
{
    windowManager.handleStreamStart(CONTENT_URI);
    const auto frame = createTestFrame(2 * wallSize);
    windowManager.updateStreamWindows(frame);

    BOOST_REQUIRE(externalStreamOpened);

    const auto window = windowManager.getWindows(CONTENT_URI)[0];

    BOOST_CHECK_EQUAL(window->getContent().getDimensions(), 2 * wallSize);
    BOOST_CHECK(
        scene->getGroup(0).getCoordinates().contains(window->getCoordinates()));
}

BOOST_FIXTURE_TEST_CASE(
    multi_channel_stream_shows_first_channel_on_single_surface, SingleSurface)
{
    windowManager.handleStreamStart(CONTENT_URI);
    const auto frame = createTestFrame(testFrameSize, testFrameSize2);
    windowManager.updateStreamWindows(frame);

    BOOST_REQUIRE(externalStreamOpened);

    const auto window = windowManager.getWindows(CONTENT_URI)[0];
    const auto& content =
        dynamic_cast<const PixelStreamContent&>(window->getContent());

    BOOST_CHECK_EQUAL(content.getUri(), CONTENT_URI);
    BOOST_CHECK_EQUAL(content.getChannel(), 0);
    BOOST_CHECK_EQUAL(content.getDimensions(), testFrameSize);
}

struct MultiSurface
{
    DisplayGroupPtr group0 = DisplayGroup::create(wallSize);
    DisplayGroupPtr group1 = DisplayGroup::create(wallSize);
    DisplayGroupPtr group2 = DisplayGroup::create(wallSize);
    ScenePtr scene = Scene::create({group0, group1, group2});
    PixelStreamWindowManager windowManager{*scene};

    uint externalStreamsOpened = 0u;

    MultiSurface()
    {
        QObject::connect(&windowManager,
                         &PixelStreamWindowManager::externalStreamOpening,
                         [this]() { ++externalStreamsOpened; });
    }
};

BOOST_FIXTURE_TEST_CASE(display_local_stream_on_secondary_surfaces,
                        MultiSurface)
{
    const auto webbrowserUri = "Webbrowser_0";
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    const auto type = StreamType::WEBBROWSER;
#else
    const auto type = StreamType::WHITEBOARD;
#endif

    windowManager.openWindow(1, webbrowserUri, testFrameSize, QPointF(), type);

    BOOST_REQUIRE(group0->getWindows().empty());
    BOOST_REQUIRE(group1->getWindows().size() == 1);
    BOOST_REQUIRE(group2->getWindows().empty());
    const auto whiteboardUri = "Whiteboard_0";

    windowManager.openWindow(2, whiteboardUri, testFrameSize, QPointF(),
                             StreamType::WHITEBOARD);

    BOOST_REQUIRE(group0->getWindows().empty());
    BOOST_REQUIRE(group1->getWindows().size() == 1);
    BOOST_REQUIRE(group2->getWindows().size() == 1);

    BOOST_CHECK_EQUAL(group1->getWindows()[0].get(),
                      windowManager.getWindows(webbrowserUri).at(0).get());

    BOOST_CHECK_EQUAL(group2->getWindows()[0].get(),
                      windowManager.getWindows(whiteboardUri).at(0).get());

    windowManager.updateStreamWindows(
        createTestFrame(testFrameSize, webbrowserUri));
    windowManager.updateStreamWindows(
        createTestFrame(testFrameSize, whiteboardUri));

    BOOST_CHECK(group0->getWindows().empty());
    BOOST_CHECK(group1->getWindows().size() == 1);
    BOOST_CHECK(group2->getWindows().size() == 1);

    BOOST_CHECK(!externalStreamsOpened);
}

BOOST_FIXTURE_TEST_CASE(multi_channel_stream_opens_on_multiple_surfaces,
                        MultiSurface)
{
    windowManager.handleStreamStart(CONTENT_URI);

    BOOST_REQUIRE(group0->getWindows().size() == 1);
    BOOST_CHECK(group1->getWindows().empty());
    BOOST_CHECK(group2->getWindows().empty());

    const auto frame = createTestFrame(testFrameSize, testFrameSize2);
    windowManager.updateStreamWindows(frame);

    BOOST_CHECK_EQUAL(externalStreamsOpened, 1);

    BOOST_REQUIRE(group0->getWindows().size() == 1);
    BOOST_REQUIRE(group1->getWindows().size() == 1);
    BOOST_CHECK(group2->getWindows().empty());

    const auto& window0 = *group0->getWindows()[0];
    const auto& window1 = *group1->getWindows()[0];
    const auto& c0 = window0.getContent();
    const auto& c1 = window1.getContent();
    const auto& content0 = dynamic_cast<const PixelStreamContent&>(c0);
    const auto& content1 = dynamic_cast<const PixelStreamContent&>(c1);

    BOOST_CHECK(window0.isHidden());
    BOOST_CHECK(window1.isHidden());

    BOOST_CHECK_EQUAL(content0.getUri(), CONTENT_URI);
    BOOST_CHECK_EQUAL(content1.getUri(), CONTENT_URI);

    BOOST_CHECK_EQUAL(content0.getChannel(), 0);
    BOOST_CHECK_EQUAL(content1.getChannel(), 1);

    BOOST_CHECK_EQUAL(content0.getDimensions(), testFrameSize);
    BOOST_CHECK_EQUAL(content1.getDimensions(), testFrameSize2);

    windowManager.showWindows(CONTENT_URI);

    BOOST_CHECK(!window0.isHidden());
    BOOST_CHECK(!window1.isHidden());

    // New frame updates the size of the first window and closes the second
    windowManager.updateStreamWindows(createTestFrame(testFrameSize2));

    BOOST_REQUIRE(group0->getWindows().size() == 1);
    BOOST_CHECK(group1->getWindows().empty());
    BOOST_CHECK(group2->getWindows().empty());

    BOOST_CHECK_EQUAL(content0.getUri(), CONTENT_URI);
    BOOST_CHECK_EQUAL(content0.getChannel(), 0);
    BOOST_CHECK_EQUAL(content0.getDimensions(), testFrameSize2);

    windowManager.handleStreamEnd(CONTENT_URI);

    BOOST_CHECK(group0->getWindows().empty());
    BOOST_CHECK(group1->getWindows().empty());
    BOOST_CHECK(group2->getWindows().empty());
}

BOOST_FIXTURE_TEST_CASE(multi_channel_stream_goes_fullscreen_on_all_surfaces,
                        MultiSurface)
{
    windowManager.handleStreamStart(CONTENT_URI);
    const auto frame = createTestFrame(testFrameSize, testFrameSize2);
    windowManager.updateStreamWindows(frame);
    windowManager.showWindows(CONTENT_URI);

    BOOST_REQUIRE(group0->getWindows().size() == 1);
    BOOST_REQUIRE(group1->getWindows().size() == 1);

    const auto& window0 = *group0->getWindows()[0];
    const auto& window1 = *group1->getWindows()[0];

    DisplayGroupController{*group0}.showFullscreen(window0.getID());
    BOOST_CHECK(window0.isFullscreen());
    BOOST_CHECK(window1.isFullscreen());

    DisplayGroupController{*group1}.exitFullscreen();
    BOOST_CHECK(!window0.isFullscreen());
    BOOST_CHECK(!window1.isFullscreen());
}

BOOST_FIXTURE_TEST_CASE(multi_channel_stream_closes_on_all_surfaces,
                        MultiSurface)
{
    windowManager.handleStreamStart(CONTENT_URI);
    const auto frame = createTestFrame(testFrameSize, testFrameSize2);
    windowManager.updateStreamWindows(frame);
    windowManager.showWindows(CONTENT_URI);

    BOOST_REQUIRE(group1->getWindows().size() == 1);

    const auto& window1 = *group1->getWindows()[0];
    DisplayGroupController{*group1}.remove(window1.getID());

    BOOST_CHECK(group0->getWindows().empty());
    BOOST_CHECK(group1->getWindows().empty());
}

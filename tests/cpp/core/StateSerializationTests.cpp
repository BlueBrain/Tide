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

#define BOOST_TEST_MODULE StateSerializationTests
#include <boost/test/unit_test.hpp>

#include "State.h"
#include "StateSerializationHelper.h"
#include "scene/Content.h"
#include "scene/ContentFactory.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "scene/ErrorContent.h"
#include "scene/TextureContent.h"
#include "serialization/utils.h"
#include "types.h"

#include "DummyContent.h"
#include "imageCompare.h"

#include <QtCore/QDir>
#include <QtGui/QImage>

// QCoreApplication is required by QtXml for loading legacy configuration files
#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

namespace
{
const QSize wallSize(3840 * 2 + 14, 1080 * 3 + 2 * 12);
const QSize CONTENT_SIZE(100, 100);
const int DUMMY_PARAM_VALUE = 10;
const QString DUMMY_URI = "/dummuy/uri";
const QString INVALID_URI = "/invalid/uri";
const QString VALID_TEXTURE_URI = "wall.png";
const QString LEGACY_URI = "legacy.dcx";
const QString STATE_V0_URI = "state_v0.dcx";
const QString STATE_V0_PREVIEW_FILE = "state_v0.dcxpreview";
const QString STATE_V0_BROKEN_URI = "state_v0_broken.dcx";
const QString STATE_V3_URI = "state_v3.dcx";
const QString STATE_V3_NOTITLES_URI = "state_v3_noTitles.dcx";
const QString STATE_V4_URI = "state_v4.dcx";
const QString TEST_DIR = "tmp";
const QSize VALID_TEXTURE_SIZE(256, 128);
}

ContentWindowPtr makeValidTestWindow(const QString& uri)
{
    auto content = ContentFactory::getContent(uri);
    BOOST_REQUIRE(content);
    BOOST_REQUIRE_EQUAL(content->getDimensions(), VALID_TEXTURE_SIZE);
    return std::make_shared<ContentWindow>(std::move(content));
}

ContentWindowPtr makeInvalidTestWindow(const QString& uri)
{
    auto missingContent = std::make_unique<TextureContent>(uri);
    return std::make_shared<ContentWindow>(std::move(missingContent));
}

ScenePtr makeTestScene()
{
    return Scene::create(wallSize);
}

State makeTestStateCopy()
{
    auto dummyContent = new DummyContent(CONTENT_SIZE, DUMMY_URI);
    ContentPtr content(dummyContent);
    dummyContent->dummyParam_ = DUMMY_PARAM_VALUE;
    auto window = std::make_shared<ContentWindow>(std::move(content));

    auto displayGroup = DisplayGroup::create(wallSize);
    displayGroup->addContentWindow(window);

    const auto scene = Scene::create(displayGroup);

    State state(scene);
    return serialization::xmlCopy(state);
}

BOOST_AUTO_TEST_CASE(
    testWhenStateIsSerializedAndDeserializedThenContentPropertiesArePreserved)
{
    auto state = makeTestStateCopy();
    auto contentWindows = state.getScene()->getGroup(0).getContentWindows();

    BOOST_REQUIRE_EQUAL(contentWindows.size(), 1);
    auto& content = contentWindows[0]->getContent();
    auto& dummyContent = dynamic_cast<DummyContent&>(content);

    const auto dimensions = dummyContent.getDimensions();

    BOOST_CHECK_EQUAL(dimensions, CONTENT_SIZE);
    BOOST_CHECK_EQUAL(dummyContent.dummyParam_, DUMMY_PARAM_VALUE);
    BOOST_CHECK_EQUAL(dummyContent.getType(), CONTENT_TYPE_ANY);
    BOOST_CHECK_EQUAL(dummyContent.getURI().toStdString(),
                      DUMMY_URI.toStdString());
}

BOOST_AUTO_TEST_CASE(testWhenOpeningInvalidLegacyStateThenReturnFalse)
{
    State state;
    BOOST_CHECK(!state.legacyLoadXML(INVALID_URI));
}

BOOST_AUTO_TEST_CASE(testWhenOpeningValidLegacyStateThenContentIsLoaded)
{
    State state;
    BOOST_CHECK(state.legacyLoadXML(LEGACY_URI));
    auto contentWindows = state.getScene()->getGroup(0).getContentWindows();

    BOOST_REQUIRE_EQUAL(contentWindows.size(), 1);
}

BOOST_AUTO_TEST_CASE(testStateSerializationHelperReadingFromLegacyFile)
{
    StateSerializationHelper helper{makeTestScene()};
    SceneConstPtr scene;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW(scene = helper.load(LEGACY_URI).result());
    BOOST_CHECK(scene);

    BOOST_CHECK_EQUAL(scene->getSurfaces().size(), 1);
    BOOST_CHECK_EQUAL(scene->getGroup(0).getContentWindows().size(), 1);
}

BOOST_AUTO_TEST_CASE(testWhenOpeningBrokenStateThenNoExceptionIsThrown)
{
    StateSerializationHelper helper(makeTestScene());
    SceneConstPtr scene;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW(scene = helper.load(STATE_V0_BROKEN_URI).result());
    BOOST_CHECK(!scene);
}

void checkContent(const Content& content)
{
    BOOST_CHECK_EQUAL(content.getDimensions(), VALID_TEXTURE_SIZE);
    BOOST_CHECK_EQUAL(content.getType(), CONTENT_TYPE_TEXTURE);
    BOOST_CHECK_EQUAL(content.getURI().toStdString(),
                      VALID_TEXTURE_URI.toStdString());
}

void checkLegacyWindow(ContentWindowPtr window)
{
    BOOST_CHECK_EQUAL(window->getContent().getZoomRect().width(), 1.0 / 1.5);
    BOOST_CHECK_EQUAL(window->getCoordinates(), QRectF(0.25, 0.25, 0.5, 0.5));

    checkContent(window->getContent());
}

void checkWindow(ContentWindowPtr window)
{
    BOOST_CHECK_EQUAL(window->getContent().getZoomRect().width(), 1.0 / 1.5);

    BOOST_CHECK_EQUAL(window->getCoordinates().x(), 0.25 * wallSize.width());
    BOOST_CHECK_EQUAL(window->getCoordinates().y(), 0.25 * wallSize.height());
    BOOST_CHECK_EQUAL(window->getCoordinates().size(), 0.5 * wallSize);

    checkContent(window->getContent());
}

void checkWindowVersion0(ContentWindowPtr window)
{
    BOOST_CHECK_EQUAL(window->getContent().getZoomRect().width(), 1.0 / 1.5);

    // The window's content size is 256x128, the window has width and height of
    // 0.5 and a position of (0.25; 0.25).
    // So the estimated AR of the saved group should be 2.0.
    // Current wall size is QSize(7694, 3264) -> AR ~= 2.357. So the window will
    // retain its height of 0.5*wallHeight and its width will be 2*height.
    const qreal expectedHeight = 0.5 * wallSize.height();
    const qreal expectedWidth = 2.0 * expectedHeight;

    BOOST_CHECK_EQUAL(window->getCoordinates().size(),
                      QSize(expectedWidth, expectedHeight));

    // The denormalized group will be positioned inside the new group at (0,0):
    const qreal estimatedOldGroupWidth = 2.0 * expectedWidth;
    const qreal expectedX = 0.25 * estimatedOldGroupWidth;
    const qreal expectedY = 0.25 * wallSize.height();

    BOOST_CHECK_EQUAL(window->getCoordinates().x(), expectedX);
    BOOST_CHECK_EQUAL(window->getCoordinates().y(), expectedY);

    checkContent(window->getContent());
}

void checkWindowVersion3(ContentWindowPtr window)
{
    BOOST_CHECK_EQUAL(window->getContent().getZoomRect().width(), 1.0);
    BOOST_CHECK_EQUAL(window->getCoordinates(), QRectF(486, 170, 600, 300));
    BOOST_CHECK(!window->isFocused());

    checkContent(window->getContent());
}

void checkWindowVersion4(ContentWindowPtr window)
{
    BOOST_CHECK_EQUAL(window->getContent().getZoomRect().width(), 1.0);
    BOOST_CHECK_EQUAL(window->getCoordinates(), QRectF(486, 170, 600, 300));
    BOOST_CHECK(window->isFocused());

    checkContent(window->getContent());
}

BOOST_AUTO_TEST_CASE(testWhenOpeningValidStateThenContentIsLoaded)
{
    State state;
    bool success = false;
    const auto file = STATE_V0_URI.toStdString();
    BOOST_CHECK_NO_THROW(success = serialization::fromXmlFile(state, file));
    BOOST_REQUIRE(success);

    auto scene = state.getScene();
    BOOST_CHECK_EQUAL(scene->getSurfaces().size(), 1);
    const auto& windows = scene->getGroup(0).getContentWindows();
    BOOST_REQUIRE_EQUAL(windows.size(), 1);

    checkLegacyWindow(windows[0]);
}

BOOST_AUTO_TEST_CASE(testStateSerializationHelperReadingFromVersion0File)
{
    StateSerializationHelper helper(makeTestScene());

    SceneConstPtr scene;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW(scene = helper.load(STATE_V0_URI).result());
    BOOST_REQUIRE(scene);

    const auto& group = scene->getGroup(0);
    BOOST_REQUIRE_EQUAL(group.getContentWindows().size(), 1);

    // The file contains only normalized coordinates, so all the windows have
    // to be denormalized to be adjusted to the new displaygroup.
    BOOST_REQUIRE_EQUAL(group.getCoordinates().size(), wallSize);
    checkWindowVersion0(group.getContentWindows()[0]);
}

BOOST_AUTO_TEST_CASE(testWhenOpeningValidVersion3StateThenContentIsLoaded)
{
    State state;
    bool success = false;
    const auto file = STATE_V3_URI.toStdString();
    BOOST_CHECK_NO_THROW(success = serialization::fromXmlFile(state, file));
    BOOST_REQUIRE(success);

    auto scene = state.getScene();
    const auto& group = scene->getGroup(0);
    const auto& windows = group.getContentWindows();
    BOOST_REQUIRE_EQUAL(windows.size(), 1);
    BOOST_CHECK_EQUAL(group.getCoordinates(), QRectF(0, 0, 1536, 648));

    checkWindowVersion3(group.getContentWindows()[0]);
}

BOOST_AUTO_TEST_CASE(testStateSerializationHelperReadingFromVersion3File)
{
    StateSerializationHelper helper(makeTestScene());

    SceneConstPtr scene;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW(scene = helper.load(STATE_V3_URI).result());
    BOOST_REQUIRE(scene);

    const auto& group = scene->getGroup(0);

    BOOST_REQUIRE_EQUAL(group.getContentWindows().size(), 1);
    BOOST_CHECK_EQUAL(group.getCoordinates(), QRectF(QPointF(0, 0), wallSize));

    checkWindowVersion3(group.getContentWindows()[0]);
}

BOOST_AUTO_TEST_CASE(
    testStateSerializationHelperReadingFromVersion3NoTitlesFile)
{
    StateSerializationHelper helper(makeTestScene());

    SceneConstPtr scene;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW(scene = helper.load(STATE_V3_NOTITLES_URI).result());
    BOOST_REQUIRE(scene);

    const auto& group = scene->getGroup(0);

    BOOST_REQUIRE_EQUAL(group.getContentWindows().size(), 1);
    BOOST_CHECK_EQUAL(group.getCoordinates(), QRectF(QPointF(0, 0), wallSize));

    checkWindowVersion3(group.getContentWindows()[0]);
}

BOOST_AUTO_TEST_CASE(testStateSerializationHelperReadingFromVersion4File)
{
    auto testScene = makeTestScene();
    const auto& displayGroup = testScene->getGroup(0);
    StateSerializationHelper helper(testScene);
    BOOST_CHECK_EQUAL(displayGroup.hasFocusedWindows(), false);

    SceneConstPtr scene;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW(scene = helper.load(STATE_V4_URI).result());
    BOOST_REQUIRE(scene);

    const auto& group = scene->getGroup(0);

    BOOST_REQUIRE_EQUAL(group.getContentWindows().size(), 1);
    BOOST_CHECK_EQUAL(group.getCoordinates(), QRectF(QPointF(0, 0), wallSize));
    BOOST_CHECK_EQUAL(group.hasFocusedWindows(), true);

    checkWindowVersion4(group.getContentWindows()[0]);
}

DisplayGroupPtr createTestDisplayGroup()
{
    auto window = makeValidTestWindow(VALID_TEXTURE_URI);

    const QPointF position(0.25 * wallSize.width(), 0.25 * wallSize.height());
    window->setCoordinates(QRectF(position, 0.5 * wallSize));
    window->getContent().setZoomRect(QRectF(0.1, 0.2, 1.0 / 1.5, 1.0 / 1.5));

    auto displayGroup = DisplayGroup::create(wallSize);
    displayGroup->addContentWindow(window);
    return displayGroup;
}

inline auto listFiles(const QString& dir)
{
    return QDir{dir}.entryList(QDir::NoDotAndDotDot | QDir::Files);
}

void cleanup(const QString& dir)
{
    for (const auto& file : listFiles(dir))
        QFile::remove(dir + "/" + file);
}

void setupTestDir()
{
    if (!QDir().mkdir(TEST_DIR))
        cleanup(TEST_DIR);
    // empty folders contain 2 elements: '.' and '..'
    BOOST_REQUIRE_EQUAL(QDir(TEST_DIR).count(), 2);
}

void cleanupTestDir()
{
    QDir(TEST_DIR).removeRecursively();
}

BOOST_AUTO_TEST_CASE(testStateSerializationToFile)
{
    // 1) Setup
    setupTestDir();

    // 2) Test saving
    auto displayGroup = createTestDisplayGroup();
    StateSerializationHelper helper(Scene::create(displayGroup));
    BOOST_CHECK(helper.save(TEST_DIR + "/test.dcx").result());

    const auto files = listFiles(TEST_DIR);
    BOOST_CHECK_EQUAL(files.size(), 2);
    BOOST_CHECK(files.contains("test.dcx"));
    BOOST_CHECK(files.contains("test.dcxpreview"));

    // 3) Check preview image.
    //    Observations have shown that a 2% error margin is imperceptible.
    const float previewError =
        compareImages(TEST_DIR + "/test.dcxpreview", STATE_V0_PREVIEW_FILE);
    BOOST_CHECK_LT(previewError, 0.02f);

    // 4) Test restoring
    StateSerializationHelper loader(Scene::create(displayGroup));
    auto loadedScene = loader.load(TEST_DIR + "/test.dcx").result();
    BOOST_REQUIRE(loadedScene);
    const auto& loadedGroup = loadedScene->getGroup(0);

    BOOST_REQUIRE_EQUAL(loadedGroup.getContentWindows().size(),
                        displayGroup->getContentWindows().size());
    BOOST_REQUIRE_EQUAL(loadedGroup.getCoordinates(),
                        QRectF(QPointF(0, 0), wallSize));
    checkWindow(loadedGroup.getContentWindows()[0]);

    // 4) Cleanup
    cleanupTestDir();
}

BOOST_AUTO_TEST_CASE(testStateSerializationUploadedToFile)
{
    const auto tempDir = QDir::tempPath();
    const auto uploadDir = QDir{TEST_DIR}.absolutePath() + "/";

    // 1) Setup
    if (!QDir().mkdir(uploadDir))
        cleanup(uploadDir);

    // 2) Create new file and put it into system temp folder
    const auto uploadedFile = QString{"uploaded.png"};
    const auto newValidUri = tempDir + "/" + uploadedFile;
    QFile::copy(VALID_TEXTURE_URI, newValidUri);

    // 3) Test saving
    auto displayGroup = createTestDisplayGroup();
    auto contentWindow = makeValidTestWindow(newValidUri);
    displayGroup->addContentWindow(contentWindow);
    StateSerializationHelper helper{Scene::create(displayGroup)};
    BOOST_CHECK(
        helper.save(uploadDir + "test.dcx", tempDir, uploadDir).result());
    const auto savedSessionDir = uploadDir + "test/";

    BOOST_CHECK(!listFiles(tempDir).contains(uploadedFile));
    BOOST_CHECK(listFiles(savedSessionDir).contains(uploadedFile));
    BOOST_CHECK(listFiles(uploadDir).contains("test.dcx"));
    BOOST_CHECK(listFiles(uploadDir).contains("test.dcxpreview"));
    BOOST_CHECK_EQUAL(contentWindow->getContent().getURI(),
                      savedSessionDir + uploadedFile);

    // 4) Add another file with the same name and test saving
    QFile::copy(VALID_TEXTURE_URI, newValidUri);
    auto newContentWindow = makeValidTestWindow(newValidUri);
    displayGroup->addContentWindow(newContentWindow);
    BOOST_CHECK(
        helper.save(uploadDir + "test.dcx", tempDir, uploadDir).result());

    BOOST_CHECK(!listFiles(tempDir).contains(uploadedFile));
    BOOST_CHECK(listFiles(savedSessionDir).contains(uploadedFile));
    BOOST_CHECK(listFiles(savedSessionDir).contains("uploaded_1.png"));
    BOOST_CHECK_EQUAL(newContentWindow->getContent().getURI(),
                      savedSessionDir + "uploaded_1.png");

    QDir{uploadDir}.removeRecursively();
}

BOOST_AUTO_TEST_CASE(testErrorContentWhenFileMissing)
{
    // 1) setup
    setupTestDir();
    const auto file = TEST_DIR + "/missing_content.dcx";

    auto group = DisplayGroup::create(wallSize);
    group->addContentWindow(makeInvalidTestWindow(INVALID_URI));
    group->addContentWindow(makeValidTestWindow(VALID_TEXTURE_URI));
    auto scene = Scene::create(group);

    // 2) saving session
    BOOST_CHECK(StateSerializationHelper{scene}.save(file).result());

    // 3) check restoring session
    auto loader = StateSerializationHelper{scene};
    auto loadedScene = loader.load(file).result();
    BOOST_REQUIRE(loadedScene);
    const auto& loadedGroup = loadedScene->getGroup(0);

    BOOST_REQUIRE_EQUAL(loadedGroup.getContentWindows().size(),
                        group->getContentWindows().size());

    const auto& errorContent = loadedGroup.getContentWindows()[0]->getContent();
    {
        // WAR compiler warnings with BOOST_CHECK_NO_THROW macro
        const auto e = dynamic_cast<const ErrorContent*>(&errorContent);
        BOOST_CHECK(e);
    }
    BOOST_CHECK_EQUAL(errorContent.getURI(), ":/img/error.png");
    BOOST_CHECK_EQUAL(errorContent.getFilePath(), INVALID_URI);
    BOOST_CHECK_EQUAL(errorContent.getTitle(), "uri");

    const auto& validContent = loadedGroup.getContentWindows()[1]->getContent();
    BOOST_CHECK_EQUAL(validContent.getURI(), VALID_TEXTURE_URI);
    BOOST_CHECK_EQUAL(validContent.getFilePath(), VALID_TEXTURE_URI);
    BOOST_CHECK_EQUAL(validContent.getTitle(), VALID_TEXTURE_URI);

    // 4) saving session a second time (error content is not saved)
    BOOST_CHECK(StateSerializationHelper{loadedScene}.save(file).result());

    // 5) check restoring session a second time
    loadedScene = loader.load(file).result();
    BOOST_REQUIRE(loadedScene);
    const auto& loadedGroup2 = loadedScene->getGroup(0);

    BOOST_REQUIRE_EQUAL(loadedGroup2.getContentWindows().size(), 1);
    const auto& validContent2 =
        loadedGroup2.getContentWindows()[0]->getContent();
    BOOST_CHECK_EQUAL(validContent2.getURI(), VALID_TEXTURE_URI);
    BOOST_CHECK_EQUAL(validContent2.getFilePath(), VALID_TEXTURE_URI);
    BOOST_CHECK_EQUAL(validContent2.getTitle(), VALID_TEXTURE_URI);

    // 5) cleanup
    cleanupTestDir();
}

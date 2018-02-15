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

#define BOOST_TEST_MODULE Configuration

#include "MinimalGlobalQtApp.h"

#include "WallConfiguration.h"
#include "configuration/Configuration.h"
#include "scene/Background.h"
#include "json/serialization.h"
#include "json/templates.h"

#include <QDir>

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

#define CONFIG_TEST_FILENAME "./configuration.xml"
#define CONFIG_TEST_FILENAME_EMPTY "./configuration_empty.xml"
#define CONFIG_TEST_FILENAME_MINIMAL "./configuration_minimal.xml"
#define CONFIG_TEST_FILENAME_STEREO "./configuration_stereo.xml"
#define CONFIG_TEST_FILENAME_JSON "./configuration.json"

BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

namespace
{
const std::vector<Screen> referenceScreenConfigs{
    {0, ":0.2", {0, 0}, {0, 0}, deflect::View::mono, true},
    {0, ":0.0", {0, 1080}, {0, 1}, deflect::View::mono, true},
    {0, ":0.1", {0, 2160}, {0, 2}, deflect::View::mono, true},
    {0, ":0.2", {0, 0}, {1, 0}, deflect::View::mono, true},
    {0, ":0.0", {0, 1080}, {1, 1}, deflect::View::mono, true},
    {0, ":0.1", {0, 2160}, {1, 2}, deflect::View::mono, true}};

void checkEqual(const Screen& a, const Screen& b)
{
    BOOST_CHECK_EQUAL(a.surfaceIndex, b.surfaceIndex);
    BOOST_CHECK_EQUAL(a.display, b.display);
    BOOST_CHECK_EQUAL(a.position, b.position);
    BOOST_CHECK_EQUAL(a.globalIndex, b.globalIndex);
    BOOST_CHECK_EQUAL((int)a.stereoMode, (int)b.stereoMode);
    BOOST_CHECK_EQUAL(a.fullscreen, b.fullscreen);
}
}

BOOST_AUTO_TEST_CASE(test_configuration_file_empty)
{
    BOOST_CHECK_THROW(Configuration config(CONFIG_TEST_FILENAME_EMPTY),
                      std::invalid_argument);
    BOOST_CHECK_THROW(WallConfiguration config(CONFIG_TEST_FILENAME_EMPTY, 1),
                      std::invalid_argument);
}

void testReferenceSurface(const Surface& surface)
{
    BOOST_CHECK_EQUAL(surface.displayWidth, 1920);
    BOOST_CHECK_EQUAL(surface.displayHeight, 1080);

    BOOST_CHECK_EQUAL(surface.displaysPerScreenX, 2);
    BOOST_CHECK_EQUAL(surface.displaysPerScreenY, 1);

    BOOST_CHECK_EQUAL(surface.screenCountX, 2);
    BOOST_CHECK_EQUAL(surface.screenCountY, 3);

    BOOST_CHECK_EQUAL(surface.bezelWidth, 14);
    BOOST_CHECK_EQUAL(surface.bezelHeight, 12);

    BOOST_CHECK_EQUAL(surface.getScreenWidth(), 3854);
    BOOST_CHECK_EQUAL(surface.getScreenHeight(), 1080);

    BOOST_CHECK_EQUAL(surface.getScreenRect(QPoint(0, 0)),
                      QRect(0, 0, 3854, 1080));
    BOOST_CHECK_EQUAL(surface.getScreenRect(QPoint(0, 1)),
                      QRect(0, 1080 + 12, 3854, 1080));
    BOOST_CHECK_EQUAL(surface.getScreenRect(QPoint(0, 2)),
                      QRect(0, 2 * (1080 + 12), 3854, 1080));
    BOOST_CHECK_EQUAL(surface.getScreenRect(QPoint(1, 0)),
                      QRect(3854 + 14, 0, 3854, 1080));
    BOOST_CHECK_EQUAL(surface.getScreenRect(QPoint(1, 1)),
                      QRect(3854 + 14, 1080 + 12, 3854, 1080));
    BOOST_CHECK_EQUAL(surface.getScreenRect(QPoint(1, 2)),
                      QRect(3854 + 14, 2 * (1080 + 12), 3854, 1080));
    BOOST_CHECK_THROW(surface.getScreenRect(QPoint(2, 0)),
                      std::invalid_argument);
    BOOST_CHECK_THROW(surface.getScreenRect(QPoint(0, 3)),
                      std::invalid_argument);
    BOOST_CHECK_THROW(surface.getScreenRect(QPoint(-1, 0)),
                      std::invalid_argument);
    BOOST_CHECK_THROW(surface.getScreenRect(QPoint(0, -1)),
                      std::invalid_argument);

    BOOST_CHECK_EQUAL(surface.getTotalWidth(), 7722);
    BOOST_CHECK_EQUAL(surface.getTotalHeight(), 3264);
    BOOST_CHECK_EQUAL(surface.getTotalSize(), QSize(7722, 3264));

    BOOST_CHECK_EQUAL(surface.getAspectRatio(), 7722.0 / 3264.0);
}

BOOST_AUTO_TEST_CASE(test_minimal_configuration_default_values)
{
    const Configuration config(CONFIG_TEST_FILENAME_MINIMAL);

    BOOST_CHECK_EQUAL(config.master.headless, false);
    BOOST_CHECK_EQUAL(config.master.webservicePort, 8888);

    BOOST_CHECK_EQUAL((int)config.global.swapsync, (int)SwapSync::software);

    BOOST_CHECK_EQUAL(config.settings.infoName, QString());
    BOOST_CHECK_EQUAL(config.settings.inactivityTimeout, 60);
    BOOST_CHECK_EQUAL(config.settings.touchpointsToWakeup, 1);
    BOOST_CHECK_EQUAL(config.settings.contentMaxScale, 0.0);
    BOOST_CHECK_EQUAL(config.settings.contentMaxScaleVectorial, 0.0);

    BOOST_CHECK_EQUAL(config.folders.contents, QDir::homePath());
    BOOST_CHECK_EQUAL(config.folders.sessions, QDir::homePath());
    BOOST_CHECK_EQUAL(config.folders.upload, QDir::tempPath());
    BOOST_CHECK_EQUAL(config.folders.tmp, QDir::tempPath());

    BOOST_CHECK_EQUAL(config.webbrowser.defaultUrl, "http://www.google.com");

    BOOST_CHECK_EQUAL(config.whiteboard.saveDir, QDir::tempPath());

    const auto& surface = config.surfaces[0];

    BOOST_CHECK_EQUAL(surface.displayWidth, 1920);
    BOOST_CHECK_EQUAL(surface.displayHeight, 1080);

    BOOST_CHECK_EQUAL(surface.displaysPerScreenX, 1);
    BOOST_CHECK_EQUAL(surface.displaysPerScreenY, 1);

    BOOST_CHECK_EQUAL(surface.screenCountX, 1);
    BOOST_CHECK_EQUAL(surface.screenCountY, 1);

    BOOST_CHECK_EQUAL(surface.getScreenWidth(), 1920);
    BOOST_CHECK_EQUAL(surface.getScreenHeight(), 1080);

    BOOST_CHECK_EQUAL(surface.bezelWidth, 0);
    BOOST_CHECK_EQUAL(surface.bezelHeight, 0);

    BOOST_CHECK_EQUAL(surface.getTotalWidth(), 1920);
    BOOST_CHECK_EQUAL(surface.getTotalHeight(), 1080);
    BOOST_CHECK_EQUAL(surface.getTotalSize(), QSize(1920, 1080));

    BOOST_CHECK_EQUAL(surface.getAspectRatio(), 1920.0 / 1080.0);

    BOOST_CHECK_EQUAL(surface.getScreenRect({0, 0}), QRect(0, 0, 1920, 1080));
}

BOOST_AUTO_TEST_CASE(test_surface_default_values)
{
    const auto surface = json::create<Surface>(QJsonObject());

    BOOST_CHECK_EQUAL(surface.displayWidth, 0);
    BOOST_CHECK_EQUAL(surface.displayHeight, 0);

    BOOST_CHECK_EQUAL(surface.displaysPerScreenX, 1);
    BOOST_CHECK_EQUAL(surface.displaysPerScreenY, 1);

    BOOST_CHECK_EQUAL(surface.screenCountX, 1);
    BOOST_CHECK_EQUAL(surface.screenCountY, 1);

    BOOST_CHECK_EQUAL(surface.bezelWidth, 0);
    BOOST_CHECK_EQUAL(surface.bezelHeight, 0);

    BOOST_CHECK_EQUAL(surface.getScreenWidth(), 0);
    BOOST_CHECK_EQUAL(surface.getScreenHeight(), 0);
    BOOST_CHECK_EQUAL(surface.getTotalSize(), QSize(0, 0));

    BOOST_CHECK_EQUAL(surface.getTotalWidth(), 0);
    BOOST_CHECK_EQUAL(surface.getTotalHeight(), 0);

    BOOST_CHECK_EQUAL(surface.getAspectRatio(), 0.0);

    BOOST_CHECK_EQUAL(surface.getScreenRect({0, 0}), QRect(0, 0, 0, 0));
}

BOOST_AUTO_TEST_CASE(test_wall_configuration)
{
    const auto processIndex = 2;
    const WallConfiguration config(CONFIG_TEST_FILENAME, processIndex);
    BOOST_REQUIRE_EQUAL(config.processIndex, processIndex);

    BOOST_CHECK_EQUAL(config.process.host, "bbplxviz03i");
    BOOST_CHECK_EQUAL(config.processCountForHost, 3);

    const auto& screens = config.process.screens;
    BOOST_REQUIRE_EQUAL(screens.size(), 1);
    const auto& screen = screens.at(0);
    BOOST_CHECK_EQUAL(screen.surfaceIndex, 0);
    BOOST_CHECK_EQUAL(screen.fullscreen, true);
    BOOST_CHECK_EQUAL(screen.display, ":0.1");
    BOOST_CHECK_EQUAL(screen.globalIndex, QPoint(0, 2));
    BOOST_CHECK_EQUAL(screen.position, QPoint(0, 2160));
    BOOST_CHECK(screen.stereoMode == deflect::View::mono);
    BOOST_CHECK_EQUAL((int)config.swapsync, (int)SwapSync::hardware);
}

BOOST_AUTO_TEST_CASE(test_stereo_configuration)
{
    const auto processIndexLeft = 0;
    WallConfiguration configLeft(CONFIG_TEST_FILENAME_STEREO, processIndexLeft);
    BOOST_REQUIRE_EQUAL(configLeft.processIndex, processIndexLeft);
    BOOST_CHECK_EQUAL(configLeft.process.host, "localhost");
    BOOST_CHECK_EQUAL(configLeft.processCountForHost, 4);

    BOOST_REQUIRE_EQUAL(configLeft.process.screens.size(), 1);
    const auto& screenLeft = configLeft.process.screens.at(0);
    BOOST_CHECK_EQUAL(screenLeft.surfaceIndex, 0);
    BOOST_CHECK_EQUAL(screenLeft.display, ":0.0");
    BOOST_CHECK_EQUAL(screenLeft.globalIndex, QPoint(0, 0));
    BOOST_CHECK_EQUAL(screenLeft.position, QPoint(0, 0));
    BOOST_CHECK(screenLeft.stereoMode == deflect::View::left_eye);

    const auto processIndexRight = 1;
    WallConfiguration configRight(CONFIG_TEST_FILENAME_STEREO,
                                  processIndexRight);
    BOOST_REQUIRE_EQUAL(configRight.processIndex, processIndexRight);
    BOOST_CHECK_EQUAL(configRight.process.host, "localhost");
    BOOST_CHECK_EQUAL(configRight.processCountForHost, 4);

    BOOST_REQUIRE_EQUAL(configRight.process.screens.size(), 1);
    const auto& screenRight = configRight.process.screens.at(0);
    BOOST_CHECK_EQUAL(screenRight.surfaceIndex, 0);
    BOOST_CHECK_EQUAL(screenRight.display, ":0.1");
    BOOST_CHECK_EQUAL(screenRight.globalIndex, QPoint(0, 0));
    BOOST_CHECK_EQUAL(screenRight.position, QPoint(0, 0));
    BOOST_CHECK(screenRight.stereoMode == deflect::View::right_eye);

    BOOST_REQUIRE_LT(screenLeft.surfaceIndex, configLeft.surfaces.size());
    const auto& surface = configLeft.surfaces[screenLeft.surfaceIndex];

    BOOST_CHECK_EQUAL(surface.bezelWidth, -60);
    BOOST_CHECK_EQUAL(surface.bezelHeight, 0);

    BOOST_CHECK_EQUAL(surface.getScreenWidth(), 1920);
    BOOST_CHECK_EQUAL(surface.getScreenHeight(), 1200);

    BOOST_CHECK_EQUAL(surface.getTotalWidth(), 1920 * 2 - 60);
    BOOST_CHECK_EQUAL(surface.getTotalHeight(), 1200);

    BOOST_CHECK_EQUAL(surface.screenCountX, 2);
    BOOST_CHECK_EQUAL(surface.screenCountY, 1);
}

struct FixtureXml
{
    const QString filename{CONFIG_TEST_FILENAME};
    const QString savedFilename{"./configuration_modified.xml"};
};
struct FixtureJson
{
    const QString filename{CONFIG_TEST_FILENAME_JSON};
    const QString savedFilename{"./configuration_modified.json"};
};
using Fixtures = boost::mpl::vector<FixtureXml, FixtureJson>;

BOOST_FIXTURE_TEST_CASE_TEMPLATE(test_reference_configuration, F, Fixtures, F)
{
    const Configuration config(F::filename);

    BOOST_CHECK_EQUAL(config.master.headless, true);
    BOOST_CHECK_EQUAL(config.master.planarSerialPort, "/dev/ttyS0");
    BOOST_CHECK_EQUAL(config.master.webservicePort, 10000);

    BOOST_CHECK_EQUAL(config.settings.infoName, "TestWall");
    BOOST_CHECK_EQUAL(config.settings.inactivityTimeout, 27);
    BOOST_CHECK_EQUAL(config.settings.touchpointsToWakeup, 10);
    BOOST_CHECK_EQUAL(config.settings.contentMaxScale, 4.4);
    BOOST_CHECK_EQUAL(config.settings.contentMaxScaleVectorial, 8.8);

    BOOST_CHECK_EQUAL(config.folders.contents,
                      "/nfs4/bbp.epfl.ch/visualization/DisplayWall/media");
    BOOST_CHECK_EQUAL(config.folders.sessions,
                      "/nfs4/bbp.epfl.ch/visualization/DisplayWall/sessions");
    BOOST_CHECK_EQUAL(config.folders.upload,
                      "/nfs4/bbp.epfl.ch/media/DisplayWall/upload");
    BOOST_CHECK_EQUAL(config.folders.tmp,
                      "/nfs4/bbp.epfl.ch/media/DisplayWall/tmp");

    BOOST_CHECK_EQUAL(config.launcher.display, ":0");
    BOOST_CHECK_EQUAL(config.launcher.demoServiceUrl,
                      "https://visualization-dev.humanbrainproject.eu/rrm");
    BOOST_CHECK_EQUAL(config.launcher.demoServiceImageDir,
                      "/nfs4/bbp.epfl.ch/visualization/demo_previews");

    BOOST_CHECK_EQUAL(config.webbrowser.defaultUrl, "http://bbp.epfl.ch");
    BOOST_CHECK_EQUAL(config.webbrowser.defaultSize, QSize(1680, 1320));

    BOOST_CHECK_EQUAL(config.whiteboard.saveDir,
                      "/nfs4/bbp.epfl.ch/media/DisplayWall/whiteboard/");
    BOOST_CHECK_EQUAL(config.whiteboard.defaultSize, QSize(1570, 1240));

    const auto& background = *config.background;
    BOOST_CHECK(background.getColor() == QColor("#242424"));
    BOOST_CHECK_EQUAL(background.getUri(), "wall.png");

    BOOST_REQUIRE_EQUAL(config.surfaces.size(), 1);
    testReferenceSurface(config.surfaces[0]);

    // Check processes
    BOOST_REQUIRE_EQUAL(config.processes.size(), 6);
    for (int i = 0; i < 3; ++i)
        BOOST_CHECK_EQUAL(config.processes[i].host, "bbplxviz03i");
    for (int i = 3; i < 6; ++i)
        BOOST_CHECK_EQUAL(config.processes[i].host, "bbplxviz04i");

    // Check processes' screen
    for (int i = 0; i < 6; ++i)
    {
        BOOST_REQUIRE_EQUAL(config.processes[i].screens.size(), 1);
        checkEqual(config.processes[i].screens[0], referenceScreenConfigs[i]);
    }
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(test_save_configuration, F, Fixtures, F)
{
    {
        Configuration config(F::filename);
        config.background->setColor(QColor("#123456"));
        BOOST_CHECK(config.save(F::savedFilename));
    }

    // Check reloading
    const Configuration config(F::savedFilename);
    BOOST_CHECK(config.background->getColor() == QColor("#123456"));
}

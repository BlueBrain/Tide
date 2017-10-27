/*********************************************************************/
/* Copyright (c) 2014-2017, EPFL/Blue Brain Project                  */
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
#include <boost/test/unit_test.hpp>

#include "MinimalGlobalQtApp.h"

#include "MasterConfiguration.h"
#include "WallConfiguration.h"
#include "scene/VectorialContent.h"

#include <QDir>

// clang-format off
#define CONFIG_TEST_FILENAME_EMPTY "./configuration_empty.xml"
#define CONFIG_TEST_FILENAME_MINIMAL "./configuration_minimal.xml"
#define CONFIG_TEST_FILENAME "./configuration.xml"
#define CONFIG_TEST_FILENAME_II "./configuration_default.xml"
#define CONFIG_TEST_FILENAME_STEREO "./configuration_stereo.xml"

#define CONFIG_EXPECTED_BACKGROUND "/nfs4/bbp.epfl.ch/visualization/DisplayWall/media/background.png"
#define CONFIG_EXPECTED_BACKGROUND_COLOR "#242424"
#define CONFIG_EXPECTED_CONTENT_DIR "/nfs4/bbp.epfl.ch/visualization/DisplayWall/media"
#define CONFIG_EXPECTED_SERIAL_PORT "/dev/ttyS0"
#define CONFIG_EXPECTED_INFO_NAME "TestWall"
#define CONFIG_EXPECTED_PLANAR_TIMEOUT 45
#define CONFIG_EXPECTED_DEFAULT_PLANAR_TIMEOUT 60
#define CONFIG_EXPECTED_SESSIONS_DIR "/nfs4/bbp.epfl.ch/visualization/DisplayWall/sessions"
#define CONFIG_EXPECTED_LAUNCHER_DISPLAY ":0"
#define CONFIG_EXPECTED_DEMO_SERVICE_URL "https://visualization-dev.humanbrainproject.eu/viz/rendering-resource-manager/v1"
#define CONFIG_EXPECTED_DEMO_SERVICE_IMAGE_DIR "/nfs4/bbp.epfl.ch/visualization/resources/software/displaywall/demo_previews"

#define CONFIG_EXPECTED_WEBSERVICE_PORT 10000
#define CONFIG_EXPECTED_DEFAULT_WEBSERVICE_PORT 8888
#define CONFIG_EXPECTED_URL "http://bbp.epfl.ch"
#define CONFIG_EXPECTED_DEFAULT_URL "http://www.google.com"
#define CONFIG_EXPECTED_WHITEBOARD_SAVE_FOLDER "/nfs4/bbp.epfl.ch/media/DisplayWall/whiteboard/"
#define CONFIG_EXPECTED_DEFAULT_WHITEBOARD_SAVE_FOLDER "/tmp/"

#define CONFIG_EXPECTED_APPLAUNCHER "/some/path/to/launcher.qml"
#define CONFIG_EXPECTED_DEFAULT_APPLAUNCHER ""
// clang-format on

BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

BOOST_AUTO_TEST_CASE(test_configuration_default_values)
{
    Configuration config(CONFIG_TEST_FILENAME_MINIMAL);

    BOOST_CHECK_EQUAL(config.getDisplaysPerScreenY(), 1);
    BOOST_CHECK_EQUAL(config.getDisplaysPerScreenX(), 1);

    BOOST_CHECK_EQUAL(config.getBezelHeight(), 0);
    BOOST_CHECK_EQUAL(config.getBezelWidth(), 0);

    BOOST_CHECK_EQUAL(config.getDisplayHeight(), 1080);
    BOOST_CHECK_EQUAL(config.getDisplayWidth(), 1920);

    BOOST_CHECK_EQUAL(config.getScreenHeight(), 1080);
    BOOST_CHECK_EQUAL(config.getScreenWidth(), 1920);

    BOOST_CHECK_EQUAL(config.getTotalHeight(), 1080);
    BOOST_CHECK_EQUAL(config.getTotalWidth(), 1920);

    BOOST_CHECK_EQUAL(config.getTotalSize(), QSize(1920, 1080));
    BOOST_CHECK_EQUAL(config.getAspectRatio(), 1920.00 / 1080.00);

    BOOST_CHECK_EQUAL(config.getTotalScreenCountX(), 1);
    BOOST_CHECK_EQUAL(config.getTotalScreenCountY(), 1);

    BOOST_CHECK_EQUAL(config.getScreenRect({0, 0}), QRect(0, 0, 1920, 1080));

    BOOST_CHECK_EQUAL(config.getFullscreen(), false);
    BOOST_CHECK_EQUAL((int)config.getSwapSync(), (int)SwapSync::software);
}

BOOST_AUTO_TEST_CASE(test_configuration_file_empty)
{
    BOOST_CHECK_THROW(Configuration config(CONFIG_TEST_FILENAME_EMPTY),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_configuration_all_values)
{
    // Make sure the default values are strictly positive
    BOOST_REQUIRE_GT(Content::getMaxScale(), 0.0);
    BOOST_REQUIRE_GT(VectorialContent::getMaxScale(), 0.0);

    // Make sure the values will be modified by loading the configuation file
    BOOST_REQUIRE_NE(Content::getMaxScale(), 4.0);
    BOOST_REQUIRE_NE(VectorialContent::getMaxScale(), 8.0);

    Configuration config(CONFIG_TEST_FILENAME);

    BOOST_CHECK_EQUAL(config.getFullscreen(), true);

    BOOST_CHECK_EQUAL(config.getDisplaysPerScreenX(), 2);
    BOOST_CHECK_EQUAL(config.getDisplaysPerScreenY(), 1);

    BOOST_CHECK_EQUAL(config.getBezelHeight(), 12);
    BOOST_CHECK_EQUAL(config.getBezelWidth(), 14);

    BOOST_CHECK_EQUAL(config.getDisplayHeight(), 1080);
    BOOST_CHECK_EQUAL(config.getDisplayWidth(), 1920);

    BOOST_CHECK_EQUAL(config.getScreenHeight(), 1080);
    BOOST_CHECK_EQUAL(config.getScreenWidth(), 3854);

    BOOST_CHECK_EQUAL(config.getTotalHeight(), 3264);
    BOOST_CHECK_EQUAL(config.getTotalWidth(), 7722);

    BOOST_CHECK_EQUAL(config.getTotalSize(), QSize(7722, 3264));
    BOOST_CHECK_EQUAL(config.getAspectRatio(), 7722.0 / 3264.0);

    BOOST_CHECK_EQUAL(config.getTotalScreenCountX(), 2);
    BOOST_CHECK_EQUAL(config.getTotalScreenCountY(), 3);

    BOOST_CHECK_EQUAL(Content::getMaxScale(), 4.0);
    BOOST_CHECK_EQUAL(VectorialContent::getMaxScale(), 8.0);

    BOOST_CHECK_EQUAL((int)config.getSwapSync(), (int)SwapSync::hardware);
}

BOOST_AUTO_TEST_CASE(test_wall_configuration)
{
    const auto processIndex = 3; // note: starts from 1, not 0
    WallConfiguration config(CONFIG_TEST_FILENAME, processIndex);
    BOOST_REQUIRE_EQUAL(config.getProcessIndex(), processIndex);

    BOOST_CHECK_EQUAL(config.getHost(), "bbplxviz03i");
    BOOST_CHECK_EQUAL(config.getProcessCountForHost(), 3);

    const auto& screens = config.getScreens();
    BOOST_REQUIRE_EQUAL(screens.size(), 1);
    const auto& screen = screens.at(0);
    BOOST_CHECK_EQUAL(screen.display, ":0.1");
    BOOST_CHECK_EQUAL(screen.globalIndex, QPoint(0, 2));
    BOOST_CHECK_EQUAL(screen.position, QPoint(0, 2160));
    BOOST_CHECK(screen.stereoMode == deflect::View::mono);
}

BOOST_AUTO_TEST_CASE(test_stereo_configuration)
{
    Configuration config(CONFIG_TEST_FILENAME_STEREO);

    BOOST_CHECK_EQUAL(config.getBezelHeight(), 0);
    BOOST_CHECK_EQUAL(config.getBezelWidth(), -60);

    BOOST_CHECK_EQUAL(config.getScreenWidth(), 1920);
    BOOST_CHECK_EQUAL(config.getScreenHeight(), 1200);

    BOOST_CHECK_EQUAL(config.getTotalWidth(), 1920 * 2 - 60);
    BOOST_CHECK_EQUAL(config.getTotalHeight(), 1200);

    BOOST_CHECK_EQUAL(config.getTotalScreenCountX(), 2);
    BOOST_CHECK_EQUAL(config.getTotalScreenCountY(), 1);

    const auto processIndexLeft = 1; // note: starts from 1, not 0
    WallConfiguration configLeft(CONFIG_TEST_FILENAME_STEREO, processIndexLeft);
    BOOST_REQUIRE_EQUAL(configLeft.getProcessIndex(), processIndexLeft);
    BOOST_CHECK_EQUAL(configLeft.getHost(), "localhost");
    BOOST_CHECK_EQUAL(configLeft.getProcessCountForHost(), 4);

    BOOST_REQUIRE_EQUAL(configLeft.getScreens().size(), 1);
    const auto& screenLeft = configLeft.getScreens().at(0);
    BOOST_CHECK_EQUAL(screenLeft.display, ":0.0");
    BOOST_CHECK_EQUAL(screenLeft.globalIndex, QPoint(0, 0));
    BOOST_CHECK_EQUAL(screenLeft.position, QPoint(0, 0));
    BOOST_CHECK(screenLeft.stereoMode == deflect::View::left_eye);

    const auto processIndexRight = 2; // note: starts from 1, not 0
    WallConfiguration configRight(CONFIG_TEST_FILENAME_STEREO,
                                  processIndexRight);
    BOOST_REQUIRE_EQUAL(configRight.getProcessIndex(), processIndexRight);
    BOOST_CHECK_EQUAL(configRight.getHost(), "localhost");
    BOOST_CHECK_EQUAL(configRight.getProcessCountForHost(), 4);

    BOOST_REQUIRE_EQUAL(configRight.getScreens().size(), 1);
    const auto& screenRight = configRight.getScreens().at(0);

    BOOST_CHECK_EQUAL(screenRight.display, ":0.1");
    BOOST_CHECK_EQUAL(screenRight.globalIndex, QPoint(0, 0));
    BOOST_CHECK_EQUAL(screenRight.position, QPoint(0, 0));
    BOOST_CHECK(screenRight.stereoMode == deflect::View::right_eye);
}

BOOST_AUTO_TEST_CASE(test_master_configuration)
{
    MasterConfiguration config(CONFIG_TEST_FILENAME);

    BOOST_CHECK_EQUAL(config.getHeadless(), true);

    BOOST_CHECK_EQUAL(config.getPlanarSerialPort(),
                      CONFIG_EXPECTED_SERIAL_PORT);

    BOOST_CHECK_EQUAL(config.getPlanarTimeout(),
                      CONFIG_EXPECTED_PLANAR_TIMEOUT);

    BOOST_CHECK_EQUAL(config.getContentDir(), CONFIG_EXPECTED_CONTENT_DIR);
    BOOST_CHECK_EQUAL(config.getSessionsDir(), CONFIG_EXPECTED_SESSIONS_DIR);

    BOOST_CHECK_EQUAL(config.getLauncherDisplay(),
                      CONFIG_EXPECTED_LAUNCHER_DISPLAY);
    BOOST_CHECK_EQUAL(config.getDemoServiceUrl(),
                      CONFIG_EXPECTED_DEMO_SERVICE_URL);
    BOOST_CHECK_EQUAL(config.getDemoServiceImageFolder(),
                      CONFIG_EXPECTED_DEMO_SERVICE_IMAGE_DIR);

    BOOST_CHECK_EQUAL(config.getWebServicePort(),
                      CONFIG_EXPECTED_WEBSERVICE_PORT);
    BOOST_CHECK_EQUAL(config.getWebBrowserDefaultURL(), CONFIG_EXPECTED_URL);

    BOOST_CHECK(config.getBackgroundColor() ==
                QColor(CONFIG_EXPECTED_BACKGROUND_COLOR));
    BOOST_CHECK_EQUAL(config.getBackgroundUri(), CONFIG_EXPECTED_BACKGROUND);

    BOOST_CHECK_EQUAL(config.getAppLauncherFile(), CONFIG_EXPECTED_APPLAUNCHER);

    BOOST_CHECK_EQUAL(config.getWhiteboardSaveFolder(),
                      CONFIG_EXPECTED_WHITEBOARD_SAVE_FOLDER);

    BOOST_CHECK_EQUAL(config.getInfoName(), CONFIG_EXPECTED_INFO_NAME);
}

BOOST_AUTO_TEST_CASE(test_master_configuration_default_values)
{
    MasterConfiguration config(CONFIG_TEST_FILENAME_II);

    BOOST_CHECK_EQUAL(config.getHeadless(), false);
    BOOST_CHECK_EQUAL(config.getContentDir(), QDir::homePath());
    BOOST_CHECK_EQUAL(config.getSessionsDir(), QDir::homePath());
    BOOST_CHECK_EQUAL(config.getWebServicePort(),
                      CONFIG_EXPECTED_DEFAULT_WEBSERVICE_PORT);
    BOOST_CHECK_EQUAL(config.getWebBrowserDefaultURL(),
                      CONFIG_EXPECTED_DEFAULT_URL);
    BOOST_CHECK_EQUAL(config.getAppLauncherFile(),
                      CONFIG_EXPECTED_DEFAULT_APPLAUNCHER);
    BOOST_CHECK_EQUAL(config.getWhiteboardSaveFolder(),
                      CONFIG_EXPECTED_DEFAULT_WHITEBOARD_SAVE_FOLDER);
    BOOST_CHECK_EQUAL(config.getPlanarTimeout(),
                      CONFIG_EXPECTED_DEFAULT_PLANAR_TIMEOUT);
    BOOST_CHECK_EQUAL(config.getInfoName(), QString());
}

BOOST_AUTO_TEST_CASE(test_save_configuration)
{
    {
        MasterConfiguration config(CONFIG_TEST_FILENAME);
        config.setBackgroundColor(QColor("#123456"));
        BOOST_CHECK(config.save("configuration_modified.xml"));
    }

    // Check reloading
    MasterConfiguration config("configuration_modified.xml");
    BOOST_CHECK(config.getBackgroundColor() == QColor("#123456"));
}

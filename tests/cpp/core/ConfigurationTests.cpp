/*********************************************************************/
/* Copyright (c) 2014-2016, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE Configuration
#include <boost/test/unit_test.hpp>

#include "MinimalGlobalQtApp.h"

#include "MasterConfiguration.h"
#include "scene/VectorialContent.h"
#include "WallConfiguration.h"

#include <QDir>

#define CONFIG_TEST_FILENAME "./configuration.xml"
#define CONFIG_TEST_FILENAME_II "./configuration_default.xml"

#define CONFIG_EXPECTED_BACKGROUND "/nfs4/bbp.epfl.ch/visualization/DisplayWall/media/background.png"
#define CONFIG_EXPECTED_BACKGROUND_COLOR "#242424"
#define CONFIG_EXPECTED_CONTENT_DIR "/nfs4/bbp.epfl.ch/visualization/DisplayWall/media"
#define CONFIG_EXPECTED_SESSIONS_DIR "/nfs4/bbp.epfl.ch/visualization/DisplayWall/sessions"
#define CONFIG_EXPECTED_LAUNCHER_DISPLAY ":0"
#define CONFIG_EXPECTED_DEMO_SERVICE_URL "https://visualization-dev.humanbrainproject.eu/viz/rendering-resource-manager/v1"
#define CONFIG_EXPECTED_DEMO_SERVICE_IMAGE_DIR "/nfs4/bbp.epfl.ch/visualization/resources/software/displaywall/demo_previews"
#define CONFIG_EXPECTED_DISPLAY ":0.2"
#define CONFIG_EXPECTED_HOST_NAME "bbplxviz03i"

#define CONFIG_EXPECTED_WEBSERVICE_PORT 10000
#define CONFIG_EXPECTED_DEFAULT_WEBSERVICE_PORT 8888
#define CONFIG_EXPECTED_URL "http://bbp.epfl.ch"
#define CONFIG_EXPECTED_DEFAULT_URL "http://www.google.com"

#define CONFIG_EXPECTED_APPLAUNCHER "/some/path/to/launcher.qml"
#define CONFIG_EXPECTED_DEFAULT_APPLAUNCHER ""

BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp );

void testBaseParameters( const Configuration& config )
{
    BOOST_CHECK_EQUAL( config.getFullscreen(), true );

    BOOST_CHECK_EQUAL( config.getMullionHeight(), 12 );
    BOOST_CHECK_EQUAL( config.getMullionWidth(), 14 );

    BOOST_CHECK_EQUAL( config.getScreenHeight(), 1080 );
    BOOST_CHECK_EQUAL( config.getScreenWidth(), 3840 );

    BOOST_CHECK_EQUAL( config.getTotalHeight(), 3264 );
    BOOST_CHECK_EQUAL( config.getTotalWidth(), 7694 );

    BOOST_CHECK_EQUAL( config.getTotalScreenCountX(), 2 );
    BOOST_CHECK_EQUAL( config.getTotalScreenCountY(), 3 );

    BOOST_CHECK_EQUAL( Content::getMaxScale(), 4.0 );
    BOOST_CHECK_EQUAL( VectorialContent::getMaxScale(), 8.0 );
}

BOOST_AUTO_TEST_CASE( test_configuration )
{
    // Make sure the values are modified by loading the configuation file
    BOOST_REQUIRE_NE( Content::getMaxScale(), 4.0 );
    BOOST_REQUIRE_NE( VectorialContent::getMaxScale(), 8.0 );

    // Make sure the values are strictly positive
    BOOST_REQUIRE_GT( Content::getMaxScale(), 0.0 );
    BOOST_REQUIRE_GT( VectorialContent::getMaxScale(), 0.0 );

    Configuration config( CONFIG_TEST_FILENAME );

    testBaseParameters( config );
}

BOOST_AUTO_TEST_CASE( test_wall_configuration )
{
    WallConfiguration config( CONFIG_TEST_FILENAME, 1 );

    BOOST_CHECK_EQUAL( config.getDisplay().toStdString(), CONFIG_EXPECTED_DISPLAY );
    BOOST_CHECK( config.getGlobalScreenIndex() == QPoint( 0,0 ) );
    BOOST_CHECK_EQUAL( config.getHost().toStdString(), CONFIG_EXPECTED_HOST_NAME );}

BOOST_AUTO_TEST_CASE( test_master_configuration )
{
    MasterConfiguration config( CONFIG_TEST_FILENAME );

    BOOST_CHECK_EQUAL( config.getContentDir().toStdString(), CONFIG_EXPECTED_CONTENT_DIR );
    BOOST_CHECK_EQUAL( config.getSessionsDir().toStdString(), CONFIG_EXPECTED_SESSIONS_DIR );

    BOOST_CHECK_EQUAL( config.getLauncherDisplay().toStdString(), CONFIG_EXPECTED_LAUNCHER_DISPLAY );
    BOOST_CHECK_EQUAL( config.getDemoServiceUrl().toStdString(), CONFIG_EXPECTED_DEMO_SERVICE_URL );
    BOOST_CHECK_EQUAL( config.getDemoServiceImageFolder().toStdString(), CONFIG_EXPECTED_DEMO_SERVICE_IMAGE_DIR );

    BOOST_CHECK_EQUAL( config.getWebServicePort(), CONFIG_EXPECTED_WEBSERVICE_PORT );
    BOOST_CHECK_EQUAL( config.getWebBrowserDefaultURL().toStdString(), CONFIG_EXPECTED_URL );

    BOOST_CHECK( config.getBackgroundColor() == QColor( CONFIG_EXPECTED_BACKGROUND_COLOR ));
    BOOST_CHECK_EQUAL( config.getBackgroundUri().toStdString(), CONFIG_EXPECTED_BACKGROUND );

    BOOST_CHECK_EQUAL( config.getAppLauncherFile().toStdString(), CONFIG_EXPECTED_APPLAUNCHER );
}

BOOST_AUTO_TEST_CASE( test_master_configuration_default_values )
{
    MasterConfiguration config( CONFIG_TEST_FILENAME_II );

    BOOST_CHECK_EQUAL( config.getContentDir().toStdString(), QDir::homePath().toStdString() );
    BOOST_CHECK_EQUAL( config.getSessionsDir().toStdString(), QDir::homePath().toStdString() );
    BOOST_CHECK_EQUAL( config.getWebServicePort(), CONFIG_EXPECTED_DEFAULT_WEBSERVICE_PORT );
    BOOST_CHECK_EQUAL( config.getWebBrowserDefaultURL().toStdString(), CONFIG_EXPECTED_DEFAULT_URL );
    BOOST_CHECK_EQUAL( config.getAppLauncherFile().toStdString(), CONFIG_EXPECTED_DEFAULT_APPLAUNCHER );
}

BOOST_AUTO_TEST_CASE( test_save_configuration )
{
    {
        MasterConfiguration config( CONFIG_TEST_FILENAME );
        config.setBackgroundColor( QColor( "#123456" ));
        BOOST_CHECK( config.save( "configuration_modified.xml" ));
    }

    // Check reloading
    MasterConfiguration config( "configuration_modified.xml" );
    BOOST_CHECK( config.getBackgroundColor() == QColor( "#123456" ));
}

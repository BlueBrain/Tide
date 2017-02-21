/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE StateSerializationTests
#include <boost/test/unit_test.hpp>

#include "scene/Content.h"
#include "scene/ContentFactory.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "scene/TextureContent.h"
#include "serialization/utils.h"
#include "State.h"
#include "StateSerializationHelper.h"
#include "types.h"

#include "DummyContent.h"
#include "imageCompare.h"

#include <QtCore/QDir>
#include <QtGui/QImage>

// QCoreApplication is required by QtXml for loading legacy configuration files
#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp );

namespace
{
const QSize wallSize( 3840*2+14, 1080*3+2*12 );
const QSize CONTENT_SIZE( 100, 100 );
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
const QSize VALID_TEXTURE_SIZE( 256, 128 );
}

State makeTestStateCopy()
{
    DummyContent* dummyContent = new DummyContent( DUMMY_URI );
    ContentPtr content( dummyContent );
    dummyContent->dummyParam_ = DUMMY_PARAM_VALUE;

    content->setDimensions( CONTENT_SIZE );
    ContentWindowPtr window( new ContentWindow( content ));

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    displayGroup->addContentWindow( window );

    State state( displayGroup );
    return serialization::xmlCopy( state );
}

BOOST_AUTO_TEST_CASE( testWhenStateIsSerializedAndDeserializedThenContentPropertiesArePreserved )
{
    State state = makeTestStateCopy();
    auto contentWindows = state.getDisplayGroup()->getContentWindows();
    bool showWindowTitles = state.getDisplayGroup()->getShowWindowTitles();

    BOOST_REQUIRE_EQUAL( contentWindows.size(), 1 );
    BOOST_REQUIRE_EQUAL( showWindowTitles, true );
    Content* content = contentWindows[0]->getContent().get();
    DummyContent* dummyContent = dynamic_cast< DummyContent* >( content );
    BOOST_REQUIRE( dummyContent );

    const QSize dimensions = dummyContent->getDimensions();

    BOOST_CHECK_EQUAL( dimensions, CONTENT_SIZE );
    BOOST_CHECK_EQUAL( dummyContent->dummyParam_, DUMMY_PARAM_VALUE );
    BOOST_CHECK_EQUAL( dummyContent->getType(), CONTENT_TYPE_ANY );
    BOOST_CHECK_EQUAL( dummyContent->getURI().toStdString(),
                       DUMMY_URI.toStdString( ));
}

BOOST_AUTO_TEST_CASE( testWhenOpeningInvalidLegacyStateThenReturnFalse )
{
    State state;
    BOOST_CHECK( !state.legacyLoadXML( INVALID_URI ));
}

BOOST_AUTO_TEST_CASE( testWhenOpeningValidLegacyStateThenContentIsLoaded )
{
    State state;
    BOOST_CHECK( state.legacyLoadXML( LEGACY_URI ));
    ContentWindowPtrs contentWindows = state.getDisplayGroup()->getContentWindows();

    BOOST_REQUIRE_EQUAL( contentWindows.size(), 1 );
    BOOST_REQUIRE_EQUAL( state.getDisplayGroup()->getShowWindowTitles(), false );
}

BOOST_AUTO_TEST_CASE( testStateSerializationHelperReadingFromLegacyFile )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );

    DisplayGroupConstPtr group;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW( group = helper.load( LEGACY_URI ).result( ));
    BOOST_CHECK( group );

    BOOST_CHECK_EQUAL( group->getContentWindows().size(), 1 );
    BOOST_CHECK_EQUAL( group->getShowWindowTitles(), false );
}

BOOST_AUTO_TEST_CASE( testWhenOpeningBrokenStateThenNoExceptionIsThrown )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );

    DisplayGroupConstPtr group;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW( group = helper.load( STATE_V0_BROKEN_URI ).result( ));
    BOOST_CHECK( !group );
}

void checkContent( ContentPtr content )
{
    BOOST_CHECK_EQUAL( content->getDimensions(), VALID_TEXTURE_SIZE );
    BOOST_CHECK_EQUAL( content->getType(), CONTENT_TYPE_TEXTURE );
    BOOST_CHECK_EQUAL( content->getURI().toStdString(),
                       VALID_TEXTURE_URI.toStdString() );
}

void checkLegacyWindow( ContentWindowPtr window )
{
    BOOST_CHECK_EQUAL( window->getContent()->getZoomRect().width(), 1.0/1.5 );
    BOOST_CHECK_EQUAL( window->getCoordinates(), QRectF( 0.25, 0.25, 0.5, 0.5 ));

    checkContent( window->getContent( ));
}

void checkWindow( ContentWindowPtr window )
{
    BOOST_CHECK_EQUAL( window->getContent()->getZoomRect().width(), 1.0/1.5 );

    BOOST_CHECK_EQUAL( window->getCoordinates().x(), 0.25 * wallSize.width( ));
    BOOST_CHECK_EQUAL( window->getCoordinates().y(), 0.25 * wallSize.height( ));
    BOOST_CHECK_EQUAL( window->getCoordinates().size(), 0.5 * wallSize );

    checkContent( window->getContent( ));
}

void checkWindowVersion0( ContentWindowPtr window )
{
    BOOST_CHECK_EQUAL( window->getContent()->getZoomRect().width(), 1.0/1.5 );

    // The window's content size is 256x128, the window has width and height of
    // 0.5 and a position of (0.25; 0.25).
    // So the estimated AR of the saved group should be 2.0.
    // Current wall size is QSize(7694, 3264) -> AR ~= 2.357. So the window will
    // retain its height of 0.5*wallHeight and its width will be 2*height.
    const qreal expectedHeight = 0.5 * wallSize.height();
    const qreal expectedWidth = 2.0 * expectedHeight;

    BOOST_CHECK_EQUAL( window->getCoordinates().size(),
                       QSize( expectedWidth, expectedHeight ));

    // The denormalized group will be positioned inside the new group at (0,0):
    const qreal estimatedOldGroupWidth = 2.0 * expectedWidth;
    const qreal expectedX = 0.25 * estimatedOldGroupWidth;
    const qreal expectedY = 0.25 * wallSize.height();

    BOOST_CHECK_EQUAL( window->getCoordinates().x(), expectedX );
    BOOST_CHECK_EQUAL( window->getCoordinates().y(), expectedY );

    checkContent( window->getContent( ));
}

void checkWindowVersion3( ContentWindowPtr window )
{
    BOOST_CHECK_EQUAL( window->getContent()->getZoomRect().width(), 1.0 );
    BOOST_CHECK_EQUAL( window->getCoordinates(), QRectF( 486, 170, 600, 300 ));
    BOOST_CHECK( !window->isFocused( ));

    checkContent( window->getContent( ));
}

void checkWindowVersion4( ContentWindowPtr window )
{
    BOOST_CHECK_EQUAL( window->getContent()->getZoomRect().width(), 1.0 );
    BOOST_CHECK_EQUAL( window->getCoordinates(), QRectF( 486, 170, 600, 300 ));
    BOOST_CHECK( window->isFocused( ));

    checkContent( window->getContent( ));
}

BOOST_AUTO_TEST_CASE( testWhenOpeningValidStateThenContentIsLoaded )
{
    State state;
    bool success = false;
    const auto file = STATE_V0_URI.toStdString();
    BOOST_CHECK_NO_THROW( success = serialization::fromXmlFile( state, file ));
    BOOST_REQUIRE( success );

    const auto& windows = state.getDisplayGroup()->getContentWindows();
    BOOST_REQUIRE_EQUAL( windows.size(), 1 );
    BOOST_CHECK_EQUAL( state.getDisplayGroup()->getShowWindowTitles(), false );

    checkLegacyWindow( windows[0] );
}

BOOST_AUTO_TEST_CASE( testStateSerializationHelperReadingFromVersion0File )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );

    DisplayGroupConstPtr group;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW( group = helper.load( STATE_V0_URI ).result( ));
    BOOST_REQUIRE( group );
    BOOST_REQUIRE_EQUAL( group->getContentWindows().size(), 1 );
    BOOST_REQUIRE_EQUAL( group->getShowWindowTitles(), false );

    // The file contains only normalized coordinates, so all the windows have
    // to be denormalized to be adjusted to the new displaygroup.
    BOOST_REQUIRE_EQUAL( group->getCoordinates().size(), wallSize );
    checkWindowVersion0( group->getContentWindows()[0] );
}

BOOST_AUTO_TEST_CASE( testWhenOpeningValidVersion3StateThenContentIsLoaded )
{
    State state;
    bool success = false;
    const auto file = STATE_V3_URI.toStdString();
    BOOST_CHECK_NO_THROW( success = serialization::fromXmlFile( state, file ));
    BOOST_REQUIRE( success );

    const auto& windows = state.getDisplayGroup()->getContentWindows();
    BOOST_REQUIRE_EQUAL( windows.size(), 1 );
    BOOST_CHECK_EQUAL( state.getDisplayGroup()->getShowWindowTitles(), true );
    BOOST_CHECK_EQUAL( state.getDisplayGroup()->getCoordinates(), QRectF( 0, 0, 1536, 648 ));

    checkWindowVersion3( state.getDisplayGroup()->getContentWindows()[0] );
}

BOOST_AUTO_TEST_CASE( testStateSerializationHelperReadingFromVersion3File )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );

    DisplayGroupConstPtr group;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW( group = helper.load( STATE_V3_URI ).result( ));
    BOOST_REQUIRE( group );

    BOOST_REQUIRE_EQUAL( group->getContentWindows().size(), 1 );
    BOOST_CHECK_EQUAL( group->getShowWindowTitles(), true );
    BOOST_CHECK_EQUAL( group->getCoordinates(),
                       QRectF( QPointF( 0, 0 ), wallSize ));

    checkWindowVersion3( group->getContentWindows()[0] );
}

BOOST_AUTO_TEST_CASE( testStateSerializationHelperReadingFromVersion3NoTitlesFile )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );

    DisplayGroupConstPtr group;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW( group = helper.load( STATE_V3_NOTITLES_URI ).result( ));
    BOOST_REQUIRE( group );

    BOOST_REQUIRE_EQUAL( group->getContentWindows().size(), 1 );
    BOOST_CHECK_EQUAL( group->getShowWindowTitles(), false );
    BOOST_CHECK_EQUAL( group->getCoordinates(),
                       QRectF( QPointF( 0, 0 ), wallSize ));

    checkWindowVersion3( group->getContentWindows()[0] );
}

BOOST_AUTO_TEST_CASE( testStateSerializationHelperReadingFromVersion4File )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    StateSerializationHelper helper( displayGroup );
    BOOST_CHECK_EQUAL( displayGroup->hasFocusedWindows(), false );

    DisplayGroupConstPtr group;
    // cppcheck-suppress redundantAssignment
    BOOST_CHECK_NO_THROW( group = helper.load( STATE_V4_URI ).result( ));
    BOOST_REQUIRE( group );

    BOOST_REQUIRE_EQUAL( group->getContentWindows().size(), 1 );
    BOOST_CHECK_EQUAL( group->getShowWindowTitles(), true );
    BOOST_CHECK_EQUAL( group->getCoordinates(),
                       QRectF( QPointF( 0, 0 ), wallSize ));
    BOOST_CHECK_EQUAL( group->hasFocusedWindows(), true );

    checkWindowVersion4( group->getContentWindows()[0] );
}

DisplayGroupPtr createTestDisplayGroup()
{
    ContentPtr content = ContentFactory::getContent( VALID_TEXTURE_URI );
    BOOST_REQUIRE( content );
    BOOST_REQUIRE_EQUAL( content->getDimensions(), VALID_TEXTURE_SIZE );
    ContentWindowPtr contentWindow( new ContentWindow( content ));
    const QPointF position( 0.25 * wallSize.width(), 0.25 * wallSize.height( ));
    contentWindow->setCoordinates( QRectF( position, 0.5 * wallSize ));
    content->setZoomRect( QRectF( 0.1, 0.2, 1.0/1.5, 1.0/1.5 ));
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    displayGroup->addContentWindow( contentWindow );
    return displayGroup;
}

void cleanupTestDir()
{
    const QStringList files = QDir( TEST_DIR ).entryList( QDir::NoDotAndDotDot |
                                                          QDir::Files );
    foreach( QString file, files )
        QFile::remove( TEST_DIR + "/" + file );
}

BOOST_AUTO_TEST_CASE( testStateSerializationToFile )
{
    // 1) Setup
    QDir dir;
    if ( !dir.mkdir( TEST_DIR ))
        cleanupTestDir();
    // empty folders contain 2 elements: '.' and '..'
    BOOST_REQUIRE_EQUAL( QDir( TEST_DIR ).count(), 2 );

    // 2) Test saving
    DisplayGroupPtr displayGroup = createTestDisplayGroup();
    StateSerializationHelper helper( displayGroup );
    BOOST_CHECK( helper.save( TEST_DIR + "/test.dcx" ).result( ));

    const QStringList files = QDir( TEST_DIR ).entryList( QDir::NoDotAndDotDot |
                                                          QDir::Files );
    BOOST_CHECK_EQUAL( files.size(), 2 );
    BOOST_CHECK( files.contains( "test.dcx" ));
    BOOST_CHECK( files.contains( "test.dcxpreview" ));

    // 3) Check preview image.
    //    Observations have shown that a 2% error margin is imperceptible.
    const float previewError = compareImages( TEST_DIR + "/test.dcxpreview",
                                              STATE_V0_PREVIEW_FILE );
    BOOST_CHECK_LT( previewError, 0.02f );

    // 4) Test restoring
    StateSerializationHelper loader( displayGroup );
    DisplayGroupConstPtr loadedGroup =
            loader.load( TEST_DIR + "/test.dcx" ).result();
    BOOST_REQUIRE( loadedGroup );

    BOOST_REQUIRE_EQUAL( loadedGroup->getContentWindows().size(),
                         displayGroup->getContentWindows().size( ));
    BOOST_REQUIRE_EQUAL( loadedGroup->getShowWindowTitles(),
                         displayGroup->getShowWindowTitles());
    BOOST_REQUIRE_EQUAL( loadedGroup->getCoordinates(),
                         QRectF( QPointF( 0, 0 ), wallSize ));
    checkWindow( loadedGroup->getContentWindows()[0] );

    // 4) Cleanup
    QDir( TEST_DIR ).removeRecursively();
}

BOOST_AUTO_TEST_CASE( testStateSerializationUploadedToFile )
{
    // 1) Setup
    QDir dir;
    if ( !dir.mkdir( TEST_DIR ))
        cleanupTestDir();

    QDir uploadDir( TEST_DIR );
    QDir tempDir( QDir::tempPath( ));

    // 2) Create new file and put it into system temp folder
    const QString newValidFile("uploaded.png");
    const QString newValidUri = QDir::tempPath() + "/" + newValidFile;
    QFile::copy( VALID_TEXTURE_URI, newValidUri );

    // 3) Test saving
    DisplayGroupPtr displayGroup = createTestDisplayGroup();
    ContentPtr content = ContentFactory::getContent( newValidUri );
    BOOST_REQUIRE( content );
    BOOST_REQUIRE_EQUAL( content->getDimensions(), VALID_TEXTURE_SIZE );
    ContentWindowPtr contentWindow( new ContentWindow( content ));
    displayGroup->addContentWindow( contentWindow );
    StateSerializationHelper helper( displayGroup );
    BOOST_CHECK( helper.save( TEST_DIR + "/test.dcx",
                              TEST_DIR ).result( ));
    QDir sessionDir( TEST_DIR+"/test" );
    QStringList tempDirFiles = tempDir.entryList( QDir::NoDotAndDotDot |
                                                  QDir::Files );
    BOOST_CHECK( !tempDirFiles.contains( newValidFile ));

    QStringList uploadDirFiles = uploadDir.entryList( QDir::NoDotAndDotDot |
                                                      QDir::Files );
    QStringList sessionFiles = sessionDir.entryList( QDir::NoDotAndDotDot |
                                                     QDir::Files );

    BOOST_CHECK( sessionFiles.contains( newValidFile ));
    BOOST_CHECK( uploadDirFiles.contains( "test.dcx" ));
    BOOST_CHECK( uploadDirFiles.contains( "test.dcxpreview" ));
    BOOST_CHECK( contentWindow->getContent()->getURI() ==
                 TEST_DIR + "/test/" + "uploaded.png" );

    //4) Add another file with the same name and test saving
    QFile::copy( VALID_TEXTURE_URI, newValidUri );
    content = ContentFactory::getContent( newValidUri );
    ContentWindowPtr newContentWindow( new ContentWindow( content ));
    displayGroup->addContentWindow( newContentWindow );
    BOOST_CHECK( helper.save( TEST_DIR + "/test.dcx",
                              TEST_DIR ).result( ));
    uploadDirFiles = uploadDir.entryList( QDir::NoDotAndDotDot | QDir::Files );
    sessionFiles = sessionDir.entryList( QDir::NoDotAndDotDot | QDir::Files );

    BOOST_CHECK( !tempDirFiles.contains( newValidFile ));
    BOOST_CHECK( sessionFiles.contains( "uploaded.png" ));
    BOOST_CHECK( sessionFiles.contains( "uploaded_1.png" ));
    BOOST_CHECK( newContentWindow->getContent()->getURI() ==
                 TEST_DIR + "/test/" + "uploaded_1.png" );

    uploadDir.removeRecursively();
}

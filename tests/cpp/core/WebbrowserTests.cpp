/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#define BOOST_TEST_MODULE WebBrowser
#include <boost/test/unit_test.hpp>

#include "localstreamer/WebkitPixelStreamer.h"

#include <QWebElementCollection>
#include <QWebFrame>
#include <QWebPage>
#include <QWebView>
#include <QDir>

#include "GlobalQtApp.h"
#include "glVersion.h"

#define GL_REQ_VERSION  2
#define TEST_PAGE_URL   "/webgl_interaction.html"
#define EMPTY_PAGE_URL  "about:blank"

BOOST_GLOBAL_FIXTURE( GlobalQtApp );

QString testPageURL()
{
    return "file://" + QDir::currentPath() + TEST_PAGE_URL;
}

BOOST_AUTO_TEST_CASE( test_webgl_support )
{
    if( !hasGLXDisplay() || !glVersionGreaterEqual( GL_REQ_VERSION ))
        return;

    // load the webgl website, exec() returns when loading is finished
    WebkitPixelStreamer* streamer = new WebkitPixelStreamer( QSize(640,480), EMPTY_PAGE_URL );
    QObject::connect( streamer->getView(), SIGNAL(loadFinished(bool)),
                      QApplication::instance(), SLOT(quit()));
    streamer->setUrl( testPageURL( ));
    QApplication::instance()->exec();

    QWebPage* page = streamer->getView()->page();
    BOOST_REQUIRE( page );
    QWebFrame* frame = page->mainFrame();
    BOOST_REQUIRE( frame );
    QWebElementCollection webglCanvases = frame->findAllElements( "canvas[id=webgl-canvas]" );
    BOOST_REQUIRE_EQUAL( webglCanvases.count(), 1 );

    // http://stackoverflow.com/questions/11871077/proper-way-to-detect-webgl-support
    QVariant hasContext = frame->evaluateJavaScript(
                "hasContext = window.WebGLRenderingContext !== null;");
    BOOST_REQUIRE( hasContext.toBool( ));

    QVariant hasGL = frame->evaluateJavaScript(
                "try {"
                "  gl = canvas.getContext(\"webgl\");"
                "  hasGL = true;"
                "}"
                "catch(e) {"
                "  try {"
                "    gl = canvas.getContext(\"experimental-webgl\");"
                "    hasGL = true;"
                "  }"
                "  catch(e) {"
                "    hasGL = false;"
                "  }"
                "}");
    BOOST_CHECK( hasGL.toBool( ));

    delete streamer;
}

BOOST_AUTO_TEST_CASE( test_webgl_interaction )
{
    if( !hasGLXDisplay() || !glVersionGreaterEqual( GL_REQ_VERSION ))
        return;

    // load the webgl website, exec() returns when loading is finished
    WebkitPixelStreamer* streamer = new WebkitPixelStreamer( QSize(640, 480), EMPTY_PAGE_URL );
    QObject::connect( streamer->getView(), SIGNAL(loadFinished(bool)),
                      QApplication::instance(), SLOT(quit()));
    streamer->setUrl( testPageURL( ));
    QApplication::instance()->exec();

    QWebPage* page = streamer->getView()->page();
    BOOST_REQUIRE( page );
    QWebFrame* frame = page->mainFrame();
    BOOST_REQUIRE( frame );

    // Normalized mouse coordinates
    deflect::Event pressState;
    pressState.mouseX = 0.1;
    pressState.mouseY = 0.1;
    pressState.mouseLeft = true;
    pressState.type = deflect::Event::EVT_PRESS;

    deflect::Event moveState;
    moveState.mouseX = 0.2;
    moveState.mouseY = 0.2;
    moveState.mouseLeft = true;
    moveState.type = deflect::Event::EVT_MOVE;

    deflect::Event releaseState;
    releaseState.mouseX = 0.2;
    releaseState.mouseY = 0.2;
    releaseState.mouseLeft = true;
    releaseState.type = deflect::Event::EVT_RELEASE;

    streamer->processEvent(pressState);
    streamer->processEvent(moveState);
    streamer->processEvent(releaseState);

    const int expectedDisplacementX = (releaseState.mouseX-pressState.mouseX) *
                                streamer->size().width() / streamer->getView()->zoomFactor();
    const int expectedDisplacementY = (releaseState.mouseY-pressState.mouseY) *
                                streamer->size().height() / streamer->getView()->zoomFactor();

    QString jsX = QString("deltaX == %1;").arg(expectedDisplacementX);
    QString jsY = QString("deltaY == %1;").arg(expectedDisplacementY);

    QVariant validDeltaX = frame->evaluateJavaScript(jsX);
    QVariant validDeltaY = frame->evaluateJavaScript(jsY);

    BOOST_CHECK( validDeltaX.toBool());
    BOOST_CHECK( validDeltaY.toBool());

    delete streamer;
}

BOOST_AUTO_TEST_CASE( test_webgl_press_release )
{
    if( !hasGLXDisplay() || !glVersionGreaterEqual( GL_REQ_VERSION ))
        return;

    // load the webgl website, exec() returns when loading is finished
    WebkitPixelStreamer* streamer = new WebkitPixelStreamer( QSize(640, 480), EMPTY_PAGE_URL );
    QObject::connect( streamer->getView(), SIGNAL(loadFinished(bool)),
                      QApplication::instance(), SLOT(quit()));
    streamer->setUrl( testPageURL( ));
    QApplication::instance()->exec();

    QWebPage* page = streamer->getView()->page();
    BOOST_REQUIRE( page );
    QWebFrame* frame = page->mainFrame();
    BOOST_REQUIRE( frame );

    // Normalized mouse coordinates
    deflect::Event pressReleaseEvent;
    pressReleaseEvent.mouseX = 0.1;
    pressReleaseEvent.mouseY = 0.1;
    pressReleaseEvent.mouseLeft = true;

    pressReleaseEvent.type = deflect::Event::EVT_PRESS;
    streamer->processEvent(pressReleaseEvent);

    pressReleaseEvent.type = deflect::Event::EVT_RELEASE;
    streamer->processEvent(pressReleaseEvent);

    const int expectedPosX = pressReleaseEvent.mouseX * streamer->size().width() /
                             streamer->getView()->zoomFactor();
    const int expectedPosY = pressReleaseEvent.mouseY * streamer->size().height() /
                             streamer->getView()->zoomFactor();

    QString jsX = QString("lastMouseX == %1;").arg(expectedPosX);
    QString jsY = QString("lastMouseY == %1;").arg(expectedPosY);

    BOOST_CHECK( frame->evaluateJavaScript(jsX).toBool());
    BOOST_CHECK( frame->evaluateJavaScript(jsY).toBool());

    delete streamer;
}

BOOST_AUTO_TEST_CASE( test_webgl_wheel )
{
    if( !hasGLXDisplay() || !glVersionGreaterEqual( GL_REQ_VERSION ))
        return;

    // load the webgl website, exec() returns when loading is finished
    WebkitPixelStreamer* streamer = new WebkitPixelStreamer( QSize(640, 480), EMPTY_PAGE_URL );
    QObject::connect( streamer->getView(), SIGNAL(loadFinished(bool)),
                      QApplication::instance(), SLOT(quit()));
    streamer->setUrl( testPageURL( ));
    QApplication::instance()->exec();

    QWebPage* page = streamer->getView()->page();
    BOOST_REQUIRE( page );
    QWebFrame* frame = page->mainFrame();
    BOOST_REQUIRE( frame );

    // Normalized mouse coordinates
    deflect::Event wheelState;
    wheelState.mouseX = 0.1;
    wheelState.mouseY = 0.1;
    wheelState.dy = 40;
    wheelState.type = deflect::Event::EVT_WHEEL;

    streamer->processEvent(wheelState);

    const int expectedPosX = wheelState.mouseX * streamer->size().width() /
                             streamer->getView()->zoomFactor();
    const int expectedPosY = wheelState.mouseY * streamer->size().height() /
                             streamer->getView()->zoomFactor();
    const int expectedWheelDelta = wheelState.dy;

    QString jsX = QString("lastMouseX == %1;").arg(expectedPosX);
    QString jsY = QString("lastMouseY == %1;").arg(expectedPosY);
    QString jsD = QString("wheelDelta == %1;").arg(expectedWheelDelta);

    BOOST_CHECK( frame->evaluateJavaScript(jsX).toBool());
    BOOST_CHECK( frame->evaluateJavaScript(jsY).toBool());
    BOOST_CHECK( frame->evaluateJavaScript(jsD).toBool());

    delete streamer;
}

BOOST_AUTO_TEST_CASE( test_localstorage )
{
    if( !hasGLXDisplay() || !glVersionGreaterEqual( GL_REQ_VERSION ))
        return;

    // load the webgl website, exec() returns when loading is finished
    WebkitPixelStreamer* streamer = new WebkitPixelStreamer( QSize(640, 480), testPageURL( ));
    QObject::connect( streamer->getView(), SIGNAL(loadFinished(bool)),
                      QApplication::instance(), SLOT(quit()));
    QApplication::instance()->exec();

    QWebPage* page = streamer->getView()->page();
    BOOST_REQUIRE( page );
    QWebFrame* frame = page->mainFrame();
    BOOST_REQUIRE( frame );

    BOOST_CHECK( page->settings()->testAttribute( QWebSettings::LocalStorageEnabled ));

    const QVariant hasLocalStorage = frame->evaluateJavaScript(
                                                "var hasLocalStorage = false;"
                                                "if( window.localStorage ) {"
                                                "  hasLocalStorage = true;"
                                                "}");
    BOOST_CHECK( hasLocalStorage.toBool( ));

    delete streamer;
}

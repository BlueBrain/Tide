/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE ContentControllerTests
#include <boost/test/unit_test.hpp>

#include "DummyContent.h"

#include "config.h"
#include "control/ContentController.h"
#if TIDE_ENABLE_PDF_SUPPORT
#  include "control/PDFController.h"
#endif
#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
#  include "control/WebbrowserController.h"
#endif
#include "control/ZoomController.h"
#include "scene/ContentFactory.h"
#include "scene/ContentWindow.h"


namespace
{
const int WIDTH = 512;
const int HEIGHT = 256;
}

class TestController : public ContentController
{
public:
    explicit TestController( ContentWindow& window )
        : ContentController( window )
    {}

    QPointF normalize( const QPointF& point ) const
    {
        return getNormalizedPoint( point );
    }
};

BOOST_AUTO_TEST_CASE( testFactoryMethod )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    auto& dummyContent = dynamic_cast<DummyContent&>( *content );

    auto controller = ContentController::create( window );
    BOOST_CHECK( dynamic_cast<ContentController*>( controller.get( )));

    dummyContent.type = CONTENT_TYPE_PIXEL_STREAM;
    BOOST_CHECK_THROW( ContentController::create( window ),
                       std::bad_cast );
    ContentWindow streamWin( ContentFactory::getPixelStreamContent( "xyz" ));
    BOOST_CHECK_NO_THROW( controller = ContentController::create( streamWin ));
    BOOST_CHECK( dynamic_cast<PixelStreamController*>( controller.get( )));

#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
    dummyContent.type = CONTENT_TYPE_WEBBROWSER;
    BOOST_CHECK_THROW( ContentController::create( window ),
                       std::bad_cast );
    ContentWindow webWindow( ContentFactory::getWebbrowserContent( "abc" ));
    BOOST_CHECK_NO_THROW( controller = ContentController::create( webWindow ));
    BOOST_CHECK( dynamic_cast<WebbrowserController*>( controller.get( )));
#endif

#if TIDE_ENABLE_PDF_SUPPORT
    dummyContent.type = CONTENT_TYPE_PDF;
    controller = ContentController::create( window );
    BOOST_CHECK( dynamic_cast<PDFController*>( controller.get( )));
#endif

    dummyContent.type = CONTENT_TYPE_IMAGE_PYRAMID;
    controller = ContentController::create( window );
    BOOST_CHECK( dynamic_cast<ZoomController*>( controller.get( )));

    dummyContent.type = CONTENT_TYPE_TEXTURE;
    controller = ContentController::create( window );
    BOOST_CHECK( dynamic_cast<ZoomController*>( controller.get( )));

    dummyContent.type = CONTENT_TYPE_SVG;
    controller = ContentController::create( window );
    BOOST_CHECK( dynamic_cast<ContentController*>( controller.get( )));

    dummyContent.type = CONTENT_TYPE_MOVIE;
    controller = ContentController::create( window );
    BOOST_CHECK( dynamic_cast<ContentController*>( controller.get( )));
}

BOOST_AUTO_TEST_CASE( testNormalizedPosition )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    TestController controller( window );
    const QPointF point( WIDTH * 0.5, HEIGHT * 0.25 );
    const QPointF zero( 0.0, 0.0 );
    const QPointF one( WIDTH, HEIGHT );

    BOOST_CHECK_EQUAL( controller.normalize( point ), QPointF( 0.5, 0.25 ));
    BOOST_CHECK_EQUAL( controller.normalize( zero ), QPointF( 0.0, 0.0 ));
    BOOST_CHECK_EQUAL( controller.normalize( one ), QPointF( 1.0, 1.0 ));
}

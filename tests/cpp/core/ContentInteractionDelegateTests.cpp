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

#define BOOST_TEST_MODULE ContentInteractionDelegateTests
#include <boost/test/unit_test.hpp>

#include "ContentWindow.h"
#include "ContentInteractionDelegate.h"
#include "PDFInteractionDelegate.h"
#include "PixelStreamInteractionDelegate.h"
#include "ZoomInteractionDelegate.h"

#include "DummyContent.h"

namespace
{
const int WIDTH = 512;
const int HEIGHT = 256;
}

class TestDelegate : public ContentInteractionDelegate
{
public:
    explicit TestDelegate( ContentWindow& window )
        : ContentInteractionDelegate( window )
    {}

    QPointF normalize( const QPointF& point ) const
    {
        return getNormalizedPoint( point );
    }
};

BOOST_AUTO_TEST_CASE( testNormalizedPosition )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    TestDelegate delegate( window );
    const QPointF point( WIDTH * 0.5, HEIGHT * 0.25 );
    const QPointF zero( 0.0, 0.0 );
    const QPointF one( WIDTH, HEIGHT );

    BOOST_CHECK_EQUAL( delegate.normalize( point ), QPointF( 0.5, 0.25 ));
    BOOST_CHECK_EQUAL( delegate.normalize( zero ), QPointF( 0.0, 0.0 ));
    BOOST_CHECK_EQUAL( delegate.normalize( one ), QPointF( 1.0, 1.0 ));
}

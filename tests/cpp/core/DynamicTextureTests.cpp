/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE DynamicTextureTests

#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "DynamicTexture.h"

namespace
{
const QString TEST_FILE( "./dynamictexture.pyr" );
const QSize LOD0_SIZE( 1920, 1172 );
const QSize TILE_SIZE( 512, 312 );
const QString PATH( "/some/path/to/dynamictexture.png.pyramid/" );
}

BOOST_AUTO_TEST_CASE( testBasicInfo )
{
    DynamicTexture texture( TEST_FILE );

    BOOST_CHECK_EQUAL( texture.getSize(), LOD0_SIZE );
}

BOOST_AUTO_TEST_CASE( testTileCount )
{
    DynamicTexture texture( TEST_FILE );
    BOOST_REQUIRE_EQUAL( texture.getMaxLod(), 2 );

    BOOST_CHECK_EQUAL( texture.getTilesCount( 2 ), QSize( 1, 1 ));
    BOOST_CHECK_EQUAL( texture.getTilesCount( 1 ), QSize( 2, 2 ));
    BOOST_CHECK_EQUAL( texture.getTilesCount( 0 ), QSize( 4, 4 ));
}

BOOST_AUTO_TEST_CASE( testTileIndex )
{
    DynamicTexture texture( TEST_FILE );
    BOOST_REQUIRE_EQUAL( texture.getMaxLod(), 2 );

    BOOST_CHECK_EQUAL( texture.getFirstTileId( 2 ), 0 );
    BOOST_CHECK_EQUAL( texture.getFirstTileId( 1 ), 1 );
    BOOST_CHECK_EQUAL( texture.getFirstTileId( 0 ), 5 );
}

BOOST_AUTO_TEST_CASE( testTileCoord )
{
    DynamicTexture texture( TEST_FILE );
    BOOST_REQUIRE_EQUAL( texture.getMaxLod(), 2 );

    for( int lod = 0; lod < 2; ++lod )
    {
        const QSize tilesCount = texture.getTilesCount( lod );
        for( int y = 0; y < tilesCount.height(); ++y )
        {
            for( int x = 0; x < tilesCount.width(); ++x )
            {
                const QRect coord = texture.getTileCoord( lod, x, y );
                BOOST_CHECK_EQUAL( coord.size(), TILE_SIZE );
                BOOST_CHECK_EQUAL( coord.x(), x * TILE_SIZE.width( ));
                BOOST_CHECK_EQUAL( coord.y(), y * TILE_SIZE.height( ));
            }
        }
    }
}

void checkFilename( const DynamicTexture& texture, const int index,
                    const QString& filename )
{
    BOOST_CHECK_EQUAL( texture.getTileFilename( index ).toStdString(),
                       QString( PATH + filename ).toStdString( ));
}

BOOST_AUTO_TEST_CASE( testTileFilename )
{
    DynamicTexture texture( TEST_FILE );
    BOOST_REQUIRE_EQUAL( texture.getMaxLod(), 2 );

    checkFilename( texture, 0, "0.png" );
    checkFilename( texture, 1, "0-0.png" );
    checkFilename( texture, 2, "0-1.png" );
    checkFilename( texture, 3, "0-3.png" );
    checkFilename( texture, 4, "0-2.png" );

    checkFilename( texture, 5, "0-0-0.png" );
    checkFilename( texture, 6, "0-0-1.png" );
    checkFilename( texture, 7, "0-1-0.png" );
    checkFilename( texture, 8, "0-1-1.png" );

    checkFilename( texture, 9, "0-0-3.png" );
    checkFilename( texture, 10, "0-0-2.png" );
    checkFilename( texture, 11, "0-1-3.png" );
    checkFilename( texture, 12, "0-1-2.png" );


    checkFilename( texture, 13, "0-3-0.png" );
    checkFilename( texture, 14, "0-3-1.png" );
    checkFilename( texture, 15, "0-2-0.png" );
    checkFilename( texture, 16, "0-2-1.png" );

    checkFilename( texture, 17, "0-3-3.png" );
    checkFilename( texture, 18, "0-3-2.png" );
    checkFilename( texture, 19, "0-2-3.png" );
    checkFilename( texture, 20, "0-2-2.png" );
}

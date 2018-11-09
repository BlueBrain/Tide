/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#define BOOST_TEST_MODULE DeflectSerializationTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "serialization/deflectTypes.h"
#include "serialization/includes.h"

#include <iostream>

BOOST_AUTO_TEST_CASE(testTileSerialization)
{
    deflect::server::Tile tile;
    tile.x = 212;
    tile.y = 365;
    tile.height = 32;
    tile.width = 78;
    tile.imageData = "Z&*#HUIRB";
    tile.format = deflect::Format::rgba;
    tile.rowOrder = deflect::RowOrder::top_down;
    tile.view = deflect::View::side_by_side;

    // serialize
    std::stringstream stream;
    {
        boost::archive::binary_oarchive oa(stream);
        oa << tile;
    }

    // deserialize
    deflect::server::Tile tileDeserialized;
    {
        boost::archive::binary_iarchive ia(stream);
        ia >> tileDeserialized;
    }

    BOOST_CHECK_EQUAL(tile.x, tileDeserialized.x);
    BOOST_CHECK_EQUAL(tile.y, tileDeserialized.y);
    BOOST_CHECK_EQUAL(tile.height, tileDeserialized.height);
    BOOST_CHECK_EQUAL(tile.width, tileDeserialized.width);
    BOOST_CHECK_EQUAL(tile.imageData.toStdString(),
                      tileDeserialized.imageData.toStdString());
    BOOST_CHECK_EQUAL((int)tile.format, (int)tileDeserialized.format);
    BOOST_CHECK_EQUAL((int)tile.rowOrder, (int)tileDeserialized.rowOrder);
    BOOST_CHECK_EQUAL((int)tile.view, (int)tileDeserialized.view);
}

BOOST_AUTO_TEST_CASE(testFrameSerialization)
{
    deflect::server::Frame frame;
    frame.tiles.push_back(deflect::server::Tile());
    frame.tiles.push_back(deflect::server::Tile());
    frame.uri = "SomeUri";

    frame.tiles[0].view = deflect::View::right_eye;
    frame.tiles[1].view = deflect::View::left_eye;

    // serialize
    std::stringstream stream;
    {
        boost::archive::binary_oarchive oa(stream);
        oa << frame;
    }

    // deserialize
    deflect::server::Frame frameDeserialized;
    {
        boost::archive::binary_iarchive ia(stream);
        ia >> frameDeserialized;
    }

    BOOST_CHECK_EQUAL(frame.tiles.size(), frameDeserialized.tiles.size());
    BOOST_CHECK_EQUAL(frame.uri.toStdString(),
                      frameDeserialized.uri.toStdString());
    BOOST_CHECK_EQUAL((int)frame.tiles[0].view,
                      (int)frameDeserialized.tiles[0].view);
    BOOST_CHECK_EQUAL((int)frame.tiles[1].view,
                      (int)frameDeserialized.tiles[1].view);
}

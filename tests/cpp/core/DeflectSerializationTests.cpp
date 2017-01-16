/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE DeflectSerializationTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "serialization/includes.h"
#include "serialization/deflectTypes.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE( testSegementParametersSerialization )
{
    deflect::SegmentParameters params;
    params.x = 212;
    params.y = 365;
    params.height = 32;
    params.width = 78;
    params.compressed = false;

    // serialize
    std::stringstream stream;
    {
        boost::archive::binary_oarchive oa( stream );
        oa << params;
    }

    // deserialize
    deflect::SegmentParameters paramsDeserialized;
    {
        boost::archive::binary_iarchive ia( stream );
        ia >> paramsDeserialized;
    }

    BOOST_CHECK_EQUAL( params.x, paramsDeserialized.x );
    BOOST_CHECK_EQUAL( params.y, paramsDeserialized.y );
    BOOST_CHECK_EQUAL( params.height, paramsDeserialized.height );
    BOOST_CHECK_EQUAL( params.width, paramsDeserialized.width );
    BOOST_CHECK_EQUAL( params.compressed, paramsDeserialized.compressed );
}

BOOST_AUTO_TEST_CASE( testFrameSerialization )
{
    deflect::Frame frame;
    frame.segments.push_back( deflect::Segment() );
    frame.segments.push_back( deflect::Segment() );
    frame.uri = "SomeUri";
    frame.view = deflect::View::right_eye;

    // serialize
    std::stringstream stream;
    {
        boost::archive::binary_oarchive oa( stream );
        oa << frame;
    }

    // deserialize
    deflect::Frame frameDeserialized;
    {
        boost::archive::binary_iarchive ia( stream );
        ia >> frameDeserialized;
    }

    BOOST_CHECK_EQUAL( frame.segments.size(),
                       frameDeserialized.segments.size( ));
    BOOST_CHECK_EQUAL( frame.uri.toStdString(),
                       frameDeserialized.uri.toStdString() );
    BOOST_CHECK_EQUAL( (int)frame.view, (int)frameDeserialized.view );
}

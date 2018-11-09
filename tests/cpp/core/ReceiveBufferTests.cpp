/*********************************************************************/
/* Copyright (c) 2014-2016, EPFL/Blue Brain Project                  */
/*                     Daniel Nachbaur<daniel.nachbaur@epfl.ch>      */
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

#define BOOST_TEST_MODULE ReceiveBufferTests
#include <boost/test/unit_test.hpp>

#include "network/ReceiveBuffer.h"
#include "serialization/utils.h"

BOOST_AUTO_TEST_CASE(testReceiveBufferConstruction)
{
    ReceiveBuffer buffer;
    BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(testSetSize)
{
    ReceiveBuffer buffer;
    buffer.setSize(1);
    BOOST_CHECK_EQUAL(buffer.size(), 1);

    buffer.setSize(11);
    BOOST_CHECK_EQUAL(buffer.size(), 11);

    buffer.setSize(5);
    BOOST_CHECK_EQUAL(buffer.size(), 5);
}

namespace
{
template <typename T>
T serializeAndDeserialize(const T& data)
{
    const auto serialized = serialization::toBinary(data);

    ReceiveBuffer buffer;
    buffer.setSize(serialized.size());
    memcpy(buffer.data(), serialized.data(), serialized.size());

    return serialization::get<T>(buffer);
}
}

BOOST_AUTO_TEST_CASE(testSerializeInt)
{
    const int foo = 42;
    const int newFoo = serializeAndDeserialize(foo);

    BOOST_CHECK_EQUAL(foo, newFoo);
}

BOOST_AUTO_TEST_CASE(testSerializeMultipleData)
{
    const std::string dataString("hello world");
    const bool dataBool(false);
    const float dataFloat(42.56f);

    const std::string newDataString = serializeAndDeserialize(dataString);
    const bool newDataBool = serializeAndDeserialize(dataBool);
    const float newDataFloat = serializeAndDeserialize(dataFloat);

    BOOST_CHECK_EQUAL(dataString, newDataString);
    BOOST_CHECK_EQUAL(dataBool, newDataBool);
    BOOST_CHECK_EQUAL(dataFloat, newDataFloat);
}

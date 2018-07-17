/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#define BOOST_TEST_MODULE ScreenControllerFactoryTest

#include <boost/test/unit_test.hpp>

#include "hardware/MultiScreenController.h"
#include "hardware/PlanarController.h"
#include "hardware/ScreenControllerFactory.h"

#include <QMap>

inline std::ostream& operator<<(std::ostream& str,
                                const PlanarController::Type type)
{
    switch (type)
    {
    case PlanarController::Type::Matrix:
        str << "Matrix";
        break;
    case PlanarController::Type::TV_UR9850:
        str << "TV_UR9850";
        break;
    case PlanarController::Type::TV_UR9851:
        str << "TV_UR9851";
        break;
    }
    return str;
}

BOOST_AUTO_TEST_CASE(testExceptionWhenSerialDeviceCannotBeOpened)
{
    BOOST_CHECK_THROW(ScreenControllerFactory::create(""), std::runtime_error);
    BOOST_CHECK_THROW(ScreenControllerFactory::create(";"), std::runtime_error);
    BOOST_CHECK_THROW(ScreenControllerFactory::create("/dev/null;/dev/das"),
                      std::runtime_error);
    BOOST_CHECK_THROW(ScreenControllerFactory::create(
                          "/dev/null#UR9850;/dev/null#UR9851"),
                      std::runtime_error);
    BOOST_CHECK_THROW(ScreenControllerFactory::create("/dev/null#UR9850"),
                      std::runtime_error);
    BOOST_CHECK_THROW(ScreenControllerFactory::create("/dev/null"),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(testParsingSingleController)
{
    const QMap<QString, PlanarController::Type> expectedMatrix{
        {"/dev/usb1", PlanarController::Type::Matrix}};

    const auto processedMatrix =
        ScreenControllerFactory::parseInputString("/dev/usb1");

    BOOST_CHECK_EQUAL_COLLECTIONS(expectedMatrix.begin(), expectedMatrix.end(),
                                  processedMatrix.begin(),
                                  processedMatrix.end());

    const QMap<QString, PlanarController::Type> expectedUR9850{
        {"/dev/usb1", PlanarController::Type::TV_UR9850}};

    const auto processedUR9850 =
        ScreenControllerFactory::parseInputString("/dev/usb1#UR9850");

    BOOST_CHECK_EQUAL_COLLECTIONS(expectedUR9850.begin(), expectedUR9850.end(),
                                  processedUR9850.begin(),
                                  processedUR9850.end());

    const QMap<QString, PlanarController::Type> expectedUR9851{
        {"/dev/usb1", PlanarController::Type::TV_UR9851}};

    const auto processedUR9851 =
        ScreenControllerFactory::parseInputString("/dev/usb1#UR9851");

    BOOST_CHECK_EQUAL_COLLECTIONS(expectedUR9851.begin(), expectedUR9851.end(),
                                  processedUR9851.begin(),
                                  processedUR9851.end());
}

BOOST_AUTO_TEST_CASE(testParsingMultipleConnectionsWithType)
{
    const QMap<QString, PlanarController::Type> expected{
        {"/dev/usb1", PlanarController::Type::Matrix},
        {"/dev/usb2", PlanarController::Type::TV_UR9850},
        {"/dev/usb3", PlanarController::Type::TV_UR9851}};

    const auto processed = ScreenControllerFactory::parseInputString(
        "/dev/usb1;/dev/usb2#UR9850;/dev/usb3#UR9851");

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                  processed.begin(), processed.end());
}

BOOST_AUTO_TEST_CASE(testParsingMultipleConnectionsWithOneEmpty)
{
    const QMap<QString, PlanarController::Type> expected{
        {"/dev/usb1", PlanarController::Type::Matrix}};
    const auto processed =
        ScreenControllerFactory::parseInputString(";/dev/usb1");

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                  processed.begin(), processed.end());
}

BOOST_AUTO_TEST_CASE(testParsingMultipleConnectionsWithOneEmptyDevice)
{
    const QMap<QString, PlanarController::Type> expected{
        {"/dev/usb1", PlanarController::Type::Matrix}};
    const auto processed =
        ScreenControllerFactory::parseInputString("#UR9850;/dev/usb1");

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                  processed.begin(), processed.end());
}

BOOST_AUTO_TEST_CASE(testParsingConnectionsWithEmptyDeviceType)
{
    const QMap<QString, PlanarController::Type> expected{
        {"/dev/usb1", PlanarController::Type::Matrix}};
    const auto processed =
        ScreenControllerFactory::parseInputString("/dev/usb1#");

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                  processed.begin(), processed.end());
}

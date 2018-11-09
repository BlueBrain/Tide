/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE ScreenLockTest

#include <boost/test/unit_test.hpp>

#include "scene/ScreenLock.h"

#include <QList>
#include <QObject>

namespace
{
const QString STREAM1("Stream1");
const QString STREAM2("Stream2");
}

struct Fixture
{
    ScreenLockPtr screenLock = ScreenLock::create();
    bool locked = false;
    bool listChanged = false;
    bool lockModified = false;
    QList<QString> acceptedStreams;
    QList<QString> rejectedStreams;

    Fixture()
    {
        QObject::connect(screenLock.get(), &ScreenLock::lockChanged,
                         [this](bool lockState) { locked = lockState; });

        QObject::connect(screenLock.get(), &ScreenLock::streamListChanged,
                         [this]() { listChanged = true; });

        QObject::connect(screenLock.get(), &ScreenLock::modified,
                         [this]() { lockModified = true; });

        QObject::connect(screenLock.get(), &ScreenLock::streamAccepted,
                         [this](QString uri) { acceptedStreams.append(uri); });
    }
};

BOOST_FIXTURE_TEST_CASE(test_locking, Fixture)
{
    QObject::connect(screenLock.get(), &ScreenLock::streamRejected,
                     [this](QString uri) {
                         acceptedStreams.removeOne(uri);
                         rejectedStreams.append(uri);
                     });

    screenLock->lock();
    BOOST_CHECK_EQUAL(locked, true);
    BOOST_CHECK_EQUAL(lockModified, true);

    screenLock->requestStreamAcceptance(STREAM1);
    BOOST_CHECK_EQUAL(listChanged, true);

    listChanged = false;
    screenLock->requestStreamAcceptance(STREAM1);
    BOOST_CHECK_EQUAL(listChanged, false);

    BOOST_CHECK(screenLock->getPendingStreams().length() == 1);

    screenLock->acceptStream(STREAM1);
    BOOST_CHECK_EQUAL(acceptedStreams.contains(STREAM1), true);
    BOOST_CHECK(screenLock->getPendingStreams().length() == 0);
    acceptedStreams.clear();

    screenLock->acceptStream(STREAM1);
    BOOST_CHECK_EQUAL(acceptedStreams.contains(STREAM1), false);

    screenLock->requestStreamAcceptance(STREAM1);
    BOOST_CHECK(screenLock->getPendingStreams().length() == 1);

    screenLock->rejectStream(STREAM1);
    BOOST_CHECK_EQUAL(rejectedStreams.contains(STREAM1), true);

    listChanged = false;
    lockModified = false;
    screenLock->rejectStream(STREAM1);
    screenLock->rejectStream(STREAM2);
    BOOST_CHECK_EQUAL(listChanged, false);
    BOOST_CHECK_EQUAL(lockModified, false);
    BOOST_CHECK(rejectedStreams.length() == 1);
    BOOST_CHECK(screenLock->getPendingStreams().length() == 0);

    screenLock->requestStreamAcceptance(STREAM1);
    BOOST_CHECK(screenLock->getPendingStreams().length() == 1);
    screenLock->cancelStreamAcceptance(STREAM1);
    BOOST_CHECK(screenLock->getPendingStreams().length() == 0);

    listChanged = false;
    lockModified = false;
    screenLock->cancelStreamAcceptance(STREAM1);
    BOOST_CHECK_EQUAL(listChanged, false);
    BOOST_CHECK_EQUAL(lockModified, false);
}

BOOST_FIXTURE_TEST_CASE(test_unlocking, Fixture)
{
    screenLock->lock();
    screenLock->requestStreamAcceptance(STREAM1);
    screenLock->requestStreamAcceptance(STREAM2);

    BOOST_CHECK(screenLock->getPendingStreams().length() == 2);
    BOOST_CHECK_EQUAL(listChanged, true);
    BOOST_CHECK_EQUAL(lockModified, true);

    screenLock->unlock();
    BOOST_CHECK_EQUAL(acceptedStreams.contains(STREAM1), true);
    BOOST_CHECK_EQUAL(acceptedStreams.contains(STREAM2), true);
    BOOST_CHECK(screenLock->getPendingStreams().length() == 0);
    BOOST_CHECK_EQUAL(locked, false);
    acceptedStreams.clear();

    listChanged = false;
    lockModified = false;
    screenLock->requestStreamAcceptance(STREAM1);

    BOOST_CHECK(acceptedStreams.length() == 1);
    BOOST_CHECK_EQUAL(listChanged, false);
    BOOST_CHECK_EQUAL(lockModified, false);
}

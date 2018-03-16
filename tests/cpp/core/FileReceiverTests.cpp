/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
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

#define BOOST_TEST_MODULE FileReceiverTests

#include <boost/test/unit_test.hpp>

#include "rest/FileReceiver.h"
#include "json/json.h"

#include <rockets/http/request.h>
#include <rockets/http/response.h>

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QObject>

using namespace rockets;

namespace
{
const QString imageUri{"wall.png"};
const QPointF expectedPosition{25.0, 17.4};
const uint expectedSurfaceIndex{1};

std::string _readImageFile(const QString& filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    return file.readAll().toStdString();
}

http::Request _makeFileRequest(const QString& filename)
{
    http::Request request;
    request.body = json::dump(QJsonObject{
        {"filename", filename}, {"surfaceIndex", 1}, {"x", 25.0}, {"y", 17.4}});
    return request;
}

http::Request _makeFileRequestWithoutPosition(const QString& filename)
{
    http::Request request;
    request.body = json::dump(QJsonObject{{"filename", filename}});
    return request;
}

http::Request _makeDataRequest(const QString& filename)
{
    http::Request request;
    request.path = filename.toStdString();
    request.body = _readImageFile(imageUri);
    return request;
}

QString _parseJsonResponse(const std::string& responseBody)
{
    const auto object = json::parse(responseBody);
    BOOST_REQUIRE(object.value("url").isString());
    return object.value("url").toString();
}

inline QString _tempFile(const QString& filename)
{
    return QDir::tempPath() + "/" + filename;
}

struct OpenListener
{
    bool open = false;
    uint openSurfaceIndex = 999999;
    QString openUri;
    QPointF openPosition;

    OpenListener() = default;
    OpenListener(FileReceiver& fileReceiver)
    {
        auto openCallback = [this](const uint surfaceIndex, const QString uri,
                                   const QPointF pos, BoolCallback callback) {
            open = true;
            openSurfaceIndex = surfaceIndex;
            openUri = uri;
            openPosition = pos;
            callback(true);
        };
        QObject::connect(&fileReceiver, &FileReceiver::open, openCallback);
    }
};
}

BOOST_AUTO_TEST_CASE(testUnsupportedFileType)
{
    FileReceiver fileReceiver{QDir::tempPath()};

    const auto urlRequest = _makeFileRequest("wall.xyz");
    const auto response = fileReceiver.prepareUpload(urlRequest).get();
    BOOST_CHECK_EQUAL(response.code, 405);
}

BOOST_AUTO_TEST_CASE(testOnlyFileExtensionWithNoName)
{
    FileReceiver fileReceiver{QDir::tempPath()};

    const auto urlRequest = _makeFileRequest(".png");
    const auto response = fileReceiver.prepareUpload(urlRequest).get();
    BOOST_CHECK_EQUAL(response.code, 405);
}

BOOST_AUTO_TEST_CASE(testUploadFileWithSpecialCharacters)
{
    FileReceiver fileReceiver{QDir::tempPath()};

    OpenListener listener{fileReceiver};

    const auto imageName = QString("ué I.n_$t.png");
    const auto imageNameEncoded = QString("u%C3%A9%20I.n_$t.png");

    const auto type = http::Header::CONTENT_TYPE;

    // Prepare upload of image
    const auto uploadResponse =
        fileReceiver.prepareUpload(_makeFileRequest(imageName)).get();
    BOOST_CHECK_EQUAL(listener.open, false);
    BOOST_CHECK_EQUAL(uploadResponse.code, 200);
    BOOST_CHECK_EQUAL(uploadResponse.headers.at(type), "application/json");

    const auto receivedUri = _parseJsonResponse(uploadResponse.body);
    BOOST_CHECK_EQUAL(receivedUri, imageNameEncoded);

    // Upload image
    const auto dataRequest = _makeDataRequest(receivedUri);
    const auto dataResponse = fileReceiver.handleUpload(dataRequest).get();
    BOOST_CHECK_EQUAL(dataResponse.code, 201);
    BOOST_CHECK_EQUAL(listener.open, true);
    BOOST_CHECK_EQUAL(listener.openUri, _tempFile(imageName));
    BOOST_CHECK_EQUAL(listener.openSurfaceIndex, expectedSurfaceIndex);
    BOOST_CHECK_EQUAL(listener.openPosition, expectedPosition);
    BOOST_CHECK(QFile(_tempFile(imageName)).exists());

    // cleanup
    BOOST_CHECK(QFile::remove(_tempFile(imageName)));
}

BOOST_AUTO_TEST_CASE(testUnhandledOpenSignal)
{
    FileReceiver fileReceiver{QDir::tempPath()};

    const auto imageName = "other.png";
    const auto type = http::Header::CONTENT_TYPE;

    // Prepare upload of image
    const auto uploadRequest = _makeFileRequestWithoutPosition(imageName);
    const auto uploadResponse = fileReceiver.prepareUpload(uploadRequest).get();
    BOOST_CHECK_EQUAL(uploadResponse.code, 200);
    BOOST_CHECK_EQUAL(uploadResponse.headers.at(type), "application/json");
    const auto receivedUri = _parseJsonResponse(uploadResponse.body);
    BOOST_CHECK_EQUAL(receivedUri, imageName);

    // Upload image
    const auto dataRequest = _makeDataRequest(receivedUri);
    // Unhandled -> broken promise
    auto dataResponse = fileReceiver.handleUpload(dataRequest);
    BOOST_CHECK_THROW(dataResponse.get(), std::future_error);
    // Cleanup since the FileReceiver could not do it
    QFile uploadedFile(_tempFile(imageName));
    BOOST_CHECK(uploadedFile.exists());
    uploadedFile.remove();
}

BOOST_AUTO_TEST_CASE(testUploadFileWithoutPosition)
{
    FileReceiver fileReceiver{QDir::tempPath()};

    OpenListener listener{fileReceiver};

    const auto imageName = "abc.png";
    const auto type = http::Header::CONTENT_TYPE;

    // Prepare upload of image
    const auto uploadRequest = _makeFileRequestWithoutPosition(imageName);
    const auto uploadResponse = fileReceiver.prepareUpload(uploadRequest).get();
    BOOST_CHECK_EQUAL(listener.open, false);
    BOOST_CHECK_EQUAL(uploadResponse.code, 200);
    BOOST_CHECK_EQUAL(uploadResponse.headers.at(type), "application/json");
    const auto receivedUri = _parseJsonResponse(uploadResponse.body);
    BOOST_CHECK_EQUAL(receivedUri, imageName);

    // Upload image
    const auto dataRequest = _makeDataRequest(receivedUri);
    const auto dataResponse = fileReceiver.handleUpload(dataRequest).get();
    BOOST_CHECK_EQUAL(dataResponse.code, 201);
    BOOST_CHECK_EQUAL(listener.open, true);
    BOOST_CHECK_EQUAL(listener.openUri, _tempFile(imageName));
    BOOST_CHECK_EQUAL(listener.openSurfaceIndex, 0);
    BOOST_CHECK_EQUAL(listener.openPosition, QPointF());
    BOOST_CHECK(QFile(_tempFile(imageName)).exists());

    // cleanup
    BOOST_CHECK(QFile::remove(_tempFile(imageName)));
}

BOOST_AUTO_TEST_CASE(testUploadFileTwice)
{
    FileReceiver fileReceiver{QDir::tempPath()};

    OpenListener listener{fileReceiver};

    const auto imageName = "wall.png";
    const auto type = http::Header::CONTENT_TYPE;

    // Prepare upload of image 1
    const auto uploadResponse1 =
        fileReceiver.prepareUpload(_makeFileRequest(imageName)).get();
    BOOST_CHECK_EQUAL(listener.open, false);
    BOOST_CHECK_EQUAL(uploadResponse1.code, 200);
    BOOST_CHECK_EQUAL(uploadResponse1.headers.at(type), "application/json");
    const auto receivedUri1 = _parseJsonResponse(uploadResponse1.body);
    BOOST_CHECK_EQUAL(receivedUri1, imageName);

    // Upload image 1
    const auto dataRequest1 = _makeDataRequest(receivedUri1);
    const auto dataResponse1 = fileReceiver.handleUpload(dataRequest1).get();
    BOOST_CHECK_EQUAL(dataResponse1.code, 201);
    BOOST_CHECK_EQUAL(listener.open, true);
    BOOST_CHECK_EQUAL(listener.openUri, _tempFile(receivedUri1));
    BOOST_CHECK_EQUAL(listener.openSurfaceIndex, 1);
    BOOST_CHECK_EQUAL(listener.openPosition, expectedPosition);
    BOOST_REQUIRE(QFile(_tempFile(receivedUri1)).exists());

    // reset
    listener = OpenListener();

    // Check can't upload same data again
    const auto response = fileReceiver.handleUpload(dataRequest1).get();
    BOOST_CHECK_EQUAL(response.code, 403);

    // Make extra temp file
    BOOST_REQUIRE(QFile(_tempFile(receivedUri1)).copy(_tempFile("wall_1.png")));

    // Prepare upload of image 2
    const auto uploadResponse2 =
        fileReceiver.prepareUpload(_makeFileRequest(imageName)).get();
    BOOST_CHECK_EQUAL(listener.open, false);
    BOOST_CHECK_EQUAL(uploadResponse2.code, 200);
    BOOST_CHECK_EQUAL(uploadResponse2.headers.at(type), "application/json");
    const auto receivedUri2 = _parseJsonResponse(uploadResponse2.body);
    BOOST_CHECK_EQUAL(receivedUri2, "wall_2.png");

    // Upload image 2
    const auto dataRequest2 = _makeDataRequest(receivedUri2);
    const auto dataResponse2 = fileReceiver.handleUpload(dataRequest2).get();
    BOOST_CHECK_EQUAL(dataResponse2.code, 201);
    BOOST_CHECK_EQUAL(listener.open, true);
    BOOST_CHECK_EQUAL(listener.openUri, _tempFile(receivedUri2));
    BOOST_CHECK_EQUAL(listener.openSurfaceIndex, 1);
    BOOST_CHECK_EQUAL(listener.openPosition, expectedPosition);
    BOOST_CHECK(QFile(_tempFile(receivedUri2)).exists());

    // cleanup
    BOOST_CHECK(QFile::remove(_tempFile(receivedUri1)));
    BOOST_CHECK(QFile::remove(_tempFile("wall_1.png")));
    BOOST_CHECK(QFile::remove(_tempFile(receivedUri2)));
}

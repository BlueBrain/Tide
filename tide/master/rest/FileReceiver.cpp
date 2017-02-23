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

#include "FileReceiver.h"

#include "log.h"

#include "scene/ContentFactory.h"

#include <zeroeq/http/server.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

using namespace zeroeq;

namespace
{
QString _encodeFilename( const QString& filePath )
{
    const auto dstFileName = QFileInfo( filePath ).fileName();
    const auto url = QUrl::fromLocalFile( dstFileName );
    return url.path( QUrl::FullyEncoded );
}

QString _getAvailableFilePath( const QFileInfo& fileInfo )
{
    QString filePath = QDir::tempPath() + "/" + fileInfo.fileName();

    int nSuffix = 0;
    while( QFile( filePath ).exists( ))
    {
        filePath = QString( "%1/%2_%3.%4" ).arg( QDir::tempPath(),
                                                 fileInfo.baseName(),
                                                 QString::number( ++nSuffix ),
                                                 fileInfo.suffix( ));
    }
    return filePath;
}

std::future<http::Response> _makeResponse( const http::Code code,
                                           const QString& key,
                                           const QString& info )
{
    http::Response response{ code };
    response.headers[ http::Header::CONTENT_TYPE ] = "application/json";
    QJsonObject status;
    status[key] = info;
    response.payload = QJsonDocument{ status }.toJson().toStdString();
    return make_ready_future( response );
}

QJsonObject _toJSONObject( const std::string& data )
{
    const auto input = QByteArray::fromRawData( data.c_str(), data.size( ));
    const auto doc = QJsonDocument::fromJson( input );

    if( doc.isNull() || !doc.isObject( ))
    {
        put_flog( LOG_INFO, "Error parsing JSON string: '%s'", data.c_str( ));
        return QJsonObject{};
    }
    return doc.object();
}
}

FileReceiver::FileReceiver( http::Server& server )
    : _server( server )
{
    server.handle( http::Verb::POST, "tide/upload",
                   [this]( const std::string& payload )
    { return _prepareUpload( payload ); } );
}

std::future<http::Response>
FileReceiver::_prepareUpload( const std::string& data )
{
    const auto obj = _toJSONObject( data );
    if( obj.empty( ))
        return make_ready_future( http::Response{ http::Code::BAD_REQUEST } );

    auto fileName = obj["fileName"].toString();
    if( fileName.contains( '/' ))
        return make_ready_future( http::Response{ http::Code::NOT_SUPPORTED } );

    const QFileInfo fileInfo( fileName );
    const QString fileSuffix = fileInfo.suffix();
    if( fileSuffix.isEmpty() || fileInfo.baseName().isEmpty( ))
        return make_ready_future( http::Response{ http::Code::NOT_SUPPORTED } );

    const QStringList& filters = ContentFactory::getSupportedExtensions();
    if( !filters.contains( fileSuffix ))
        return make_ready_future( http::Response{ http::Code::NOT_SUPPORTED } );

    const auto filePath = _getAvailableFilePath( fileInfo );
    const auto encodedFilename = _encodeFilename( filePath );

    const auto path = encodedFilename.toStdString();
    _server.handle( http::Verb::PUT, "tide/upload/" + path,
                    [this, filePath, path]( const std::string& payload )
    {
        return _handleUpload( filePath, path, payload );
    });

    return _makeResponse( http::Code::OK, "url", encodedFilename );
}

std::future<http::Response>
FileReceiver::_handleUpload( const QString& filePath, const std::string& path,
                             const std::string& payload )
{
    QFile file( filePath );
    if( !file.open( QIODevice::WriteOnly ) ||
            !file.write( payload.c_str(), payload.size( )))
    {
        return _makeResponse( http::Code::INTERNAL_SERVER_ERROR, "info",
                              "could not upload"  );
    }
    file.close();
    put_flog ( LOG_INFO, "file created as %s",
               filePath.toLocal8Bit().constData( ));

    auto openPromise = std::make_shared<std::promise<bool>>();
    emit open( filePath, openPromise );
    const bool success = openPromise->get_future().get();

    _server.remove( "tide/upload/" + path );

    if( success )
    {
        put_flog( LOG_INFO, "file uploaded and saved as: %s",
                  filePath.toLocal8Bit().constData( ));
        return _makeResponse( http::Code::CREATED, "info", "OK" );
    }
    else
    {
        file.remove();
        put_flog( LOG_INFO, "file uploaded but could not be opened: %s",
                  filePath.toLocal8Bit().constData( ));
        return _makeResponse( http::Code::NOT_SUPPORTED, "info",
                              "file corrupted" );
    }
}

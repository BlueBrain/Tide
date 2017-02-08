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

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include <QByteArray>

namespace
{
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

FileReceiver::FileReceiver( zeroeq::http::Server& server )
    : _server ( server )
{
    server.handlePUT( "tide/upload", [this]( const std::string& received )
    { return _handleUpload( received ); } );

}

bool FileReceiver::_handleUpload( const std::string& payload )
{
    const auto obj = _toJSONObject( payload );
    if( obj.empty( ))
        return false;
    QString fileName = obj[ "fileName" ].toString();

    const auto url = QUrl::fromLocalFile( fileName );
    const std::string path = url.path( QUrl::FullyEncoded ).toStdString();
    _server.handlePUT( "tide/upload/" + path,
                       [ this, fileName, path ] ( const std::string& data )
    {
        const QString uploadDir = QDir::tempPath() + "/";
        QByteArray ba = QByteArray::fromRawData( data.c_str(), data.size( ));
        QFile file( uploadDir + fileName );
        file.open( QIODevice::WriteOnly );
        file.write( ba );
        file.close();
        emit open( QFileInfo( file ).absoluteFilePath( ));
        _server.remove( "tide/upload/" + path );
        return true;
    });
    return true;
}

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

#include "FileSystemQuery.h"

#include "log.h"
#include "scene/ContentFactory.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUuid>

namespace
{

QString _parseJson( const std::string& data )
{
    const QByteArray input = QByteArray::fromRawData( data.c_str(),
                                                      data.size( ));
    const auto doc = QJsonDocument::fromJson( input );
    if( doc.isNull() || !doc.isObject( ) || !doc.object()["dir"].isString( ))
    {
        put_flog( LOG_INFO, "Error parsing JSON string: '%s'", data.c_str( ));
        return QString();
    }
    return doc.object()["dir"].toString();
}

const auto sessionsFilter = QStringList{ "*.dcx" };

}

FileSystemQuery::FileSystemQuery( zeroeq::http::Server& server,
                                  const QString& contentDirectory,
                                  const Type contentType )
    : _server( server )
    , _contentDirectory( contentDirectory )
    , _contentType( contentType )
{
    _server.handlePUT( _getEndpoint(), [this]( const std::string& received )
    {
        return _handleDirectoryList( received );
    });
}

std::string FileSystemQuery::_getEndpoint() const
{
    switch( _contentType )
    {
    case Type::FILES:
        return "tide/files";
    case Type::SESSIONS:
        return "tide/sessions";
    default:
        throw std::logic_error( "no such type" );
    };
}

const QStringList& FileSystemQuery::_getFilters() const
{
    switch( _contentType )
    {
    case Type::FILES:
        return ContentFactory::getSupportedFilesFilter();
    case Type::SESSIONS:
        return sessionsFilter;
    default:
        throw std::logic_error( "no such type" );
    };
}

bool FileSystemQuery::_handleDirectoryList( const std::string& payload )
{
    const auto path = _parseJson( payload );
    if( path.isEmpty( ))
        return false;

    const QString fullpath = _contentDirectory + "/" + path;

    const QDir absolutePath( fullpath );
    if( !absolutePath.canonicalPath().startsWith( _contentDirectory ))
        return false;

    if( absolutePath.exists( ))
    {
        if( !_fsContentList.count( path ))
        {
            _fsContentList.emplace( std::piecewise_construct,
                                    std::forward_as_tuple( path ),
                                    std::forward_as_tuple( _getEndpoint(),
                                                           _contentDirectory,
                                                           path,
                                                           _getFilters( )));
            _server.handleGET( _fsContentList.at( path ));
        }
        return true;
    }
    else
    {
        auto kv =_fsContentList.find( path );
        if( kv != _fsContentList.end( ))
        {
            _server.remove( kv->second );
            _fsContentList.erase( kv );
        }
    }
    return false;
}

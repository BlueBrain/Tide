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

#include "FileSystemContent.h"

#include "log.h"

#include <QDir>
#include <QFileInfoList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

FileSystemContent::FileSystemContent( const QString& path,
                                      const QString& contentDirectory,
                                      const std::string& contentType )
    : _path ( path )
    , _contentDirectory ( contentDirectory )
    , _contentType ( contentType )
{
}

std::string FileSystemContent::getTypeName() const
{
    if( _path == "/" )
        return "tide/"+_contentType;
    else
        return "tide/"+_contentType + "/" + QUrl::fromLocalFile( _path ).
                path( QUrl::FullyEncoded ).toStdString();
}

std::string FileSystemContent::_toJSON() const
{
    const QDir directory( _contentDirectory + "/" + _path );
    const QFileInfoList content = directory.entryInfoList( QDir::AllEntries,
                                                           QDir::DirsFirst );
    const QDir contentDir( _contentDirectory );

    QJsonArray arr;
    for( const auto& file: content )
    {
        QJsonObject item;
        item["name"] = file.fileName();
        item["path"] = contentDir.relativeFilePath( file.absoluteFilePath( ));
        item["dir"] = file.isDir();
        item["file"] = file.isFile();
        arr.append( item );
    }
    return QJsonDocument{ arr }.toJson().toStdString();
}

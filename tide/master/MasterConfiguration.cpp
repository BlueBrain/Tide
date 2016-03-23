/*********************************************************************/
/* Copyright (c) 2013-2015, EPFL/Blue Brain Project                  */
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

#include "MasterConfiguration.h"

#include "log.h"

#include <QDomElement>
#include <QtXmlPatterns>
#include <stdexcept>

namespace
{
const int DEFAULT_WEBSERVICE_PORT = 8888;
const QRegExp TRIM_REGEX( "[\\n\\t\\r]" );
const QString DEFAULT_URL( "http://www.google.com" );
}

MasterConfiguration::MasterConfiguration( const QString& filename )
    : Configuration( filename )
    , _webServicePort( DEFAULT_WEBSERVICE_PORT )
    , _backgroundColor( Qt::black )
{
    loadMasterSettings();
}

void MasterConfiguration::loadMasterSettings()
{
    QXmlQuery query;
    if( !query.setFocus( QUrl( filename_ )))
        throw std::runtime_error( "Invalid configuration file: " +
                                  filename_.toStdString( ));

    loadDockStartDirectory( query );
    loadSessionsDirectory( query );
    loadWebService( query );
    loadAppLauncher( query );
    loadWebBrowserStartURL( query );
    loadBackgroundProperties( query );
}

void MasterConfiguration::loadDockStartDirectory( QXmlQuery& query )
{
    QString queryResult;

    query.setQuery( "string(/configuration/dock/@directory)" );
    if( query.evaluateTo( &queryResult ))
        _dockStartDir = queryResult.remove( QRegExp( TRIM_REGEX ));
    if( _dockStartDir.isEmpty( ))
        _dockStartDir = QDir::homePath();
}

void MasterConfiguration::loadSessionsDirectory( QXmlQuery& query )
{
    QString queryResult;

    query.setQuery( "string(/configuration/sessions/@directory)" );
    if( query.evaluateTo( &queryResult ))
        _sessionsDir = queryResult.remove( QRegExp( TRIM_REGEX ));
    if( _sessionsDir.isEmpty( ))
        _sessionsDir = QDir::homePath();
}

void MasterConfiguration::loadWebService( QXmlQuery& query )
{
    QString queryResult;
    query.setQuery( "string(/configuration/webservice/@port)" );
    if( query.evaluateTo( &queryResult ))
    {
        if( !queryResult.isEmpty( ))
            _webServicePort = queryResult.toInt();
    }
}

void MasterConfiguration::loadAppLauncher( QXmlQuery& query )
{
    QString queryResult;
    query.setQuery( "string(/configuration/applauncher/@qml)" );
    if( query.evaluateTo( &queryResult ))
        _appLauncherFile = queryResult.remove( QRegExp( TRIM_REGEX ));
}

void MasterConfiguration::loadWebBrowserStartURL( QXmlQuery& query )
{
    QString queryResult;
    query.setQuery( "string(/configuration/webbrowser/@defaultURL)");
    if( query.evaluateTo( &queryResult ))
        _webBrowserDefaultURL = queryResult.remove( QRegExp( TRIM_REGEX ));
    if( _webBrowserDefaultURL.isEmpty( ))
        _webBrowserDefaultURL = DEFAULT_URL;
}

void MasterConfiguration::loadBackgroundProperties( QXmlQuery& query )
{
    QString queryResult;
    query.setQuery( "string(/configuration/background/@uri)" );
    if( query.evaluateTo( &queryResult ))
        _backgroundUri = queryResult.remove( QRegExp( TRIM_REGEX ));

    query.setQuery( "string(/configuration/background/@color)" );
    if( query.evaluateTo( &queryResult ))
    {
        queryResult.remove( QRegExp( TRIM_REGEX ));

        const QColor newColor( queryResult );
        if( newColor.isValid( ))
            _backgroundColor = newColor;
    }
}

const QString& MasterConfiguration::getDockStartDir() const
{
    return _dockStartDir;
}

const QString& MasterConfiguration::getSessionsDir() const
{
    return _sessionsDir;
}

const QString& MasterConfiguration::getAppLauncherFile() const
{
    return _appLauncherFile;
}

int MasterConfiguration::getWebServicePort() const
{
    return _webServicePort;
}

const QString& MasterConfiguration::getWebBrowserDefaultURL() const
{
    return _webBrowserDefaultURL;
}

const QString& MasterConfiguration::getBackgroundUri() const
{
    return _backgroundUri;
}

const QColor& MasterConfiguration::getBackgroundColor() const
{
    return _backgroundColor;
}

void MasterConfiguration::setBackgroundColor( const QColor& color )
{
    _backgroundColor = color;
}

void MasterConfiguration::setBackgroundUri( const QString& uri )
{
    _backgroundUri = uri;
}

bool MasterConfiguration::save() const
{
    return save( filename_ );
}

bool MasterConfiguration::save( const QString& filename ) const
{
    QDomDocument doc( "XmlDoc" );
    QFile infile( filename_ );
    if( !infile.open( QIODevice::ReadOnly ))
    {
        put_flog( LOG_ERROR, "could not open configuration file: '%s'",
                  filename.toLocal8Bit().constData( ));
        return false;
    }
    doc.setContent( &infile );
    infile.close();

    QDomElement root = doc.documentElement();

    QDomElement background = root.firstChildElement( "background" );
    if( background.isNull( ))
    {
        background = doc.createElement( "background" );
        root.appendChild( background );
    }
    background.setAttribute( "uri", _backgroundUri );
    background.setAttribute( "color", _backgroundColor.name( ));

    QFile outfile( filename );
    if( !outfile.open( QIODevice::WriteOnly | QIODevice::Text ))
    {
        put_flog( LOG_ERROR, "could not save configuration file: '%s'",
                  filename.toLocal8Bit().constData( ));
        return false;
    }
    QTextStream out( &outfile );
    out << doc.toString( 4 );
    outfile.close();
    return true;
}

/*********************************************************************/
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
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

#include "WallConfiguration.h"

#include <QtXmlPatterns>
#include <stdexcept>

WallConfiguration::WallConfiguration( const QString& filename,
                                      const int processIndex )
    : Configuration( filename )
    , _processIndex( processIndex )
{
    _loadWallSettings( processIndex );
}

void WallConfiguration::_loadWallSettings( const int processIndex )
{
    assert( processIndex > 0 && "WallConfiguration::loadWallSettings is only"
                                "valid for processes of rank > 0" );

    const int xpathIndex = processIndex; // xpath index also starts from 1

    QXmlQuery query;
    if( !query.setFocus( QUrl( _filename )))
        throw std::runtime_error( "Invalid configuration file: '" +
                                  _filename.toStdString() + "'" );

    QString queryResult;

    // read host
    query.setQuery( QString( "string(//process[%1]/@host)").arg( xpathIndex ));
    if( query.evaluateTo( &queryResult ))
        _host = queryResult.remove( QRegExp("[\\n\\t\\r]" ));

    // read display (optional attribute)
    query.setQuery( QString( "string(//process[%1]/@display)" ).arg( xpathIndex ));
    if( query.evaluateTo( &queryResult ))
        _display = queryResult.remove( QRegExp( "[\\n\\t\\r]" ));
    else
        _display = QString( "default (:0)" ); // the default

    int value = 0;

    // read number of tiles for this process
    query.setQuery( QString( "string(count(//process[%1]/screen))" ).arg( xpathIndex ));
    if( !getInt( query, value ) || value != 1 )
        throw std::runtime_error( "Expect exactly one screen per process" );

    // read number of wall processes on the same host
    query.setQuery( QString( "string(count(//process[@host eq '%1']))" ).arg( _host ));
    if( !getInt( query, value ) || value < 1 )
        throw std::runtime_error( "Could not determine the number of wall processes on that host" );
    _processCountForHost = value;

    query.setQuery( QString( "string(//process[%1]/screen/@x)" ).arg( xpathIndex ));
    if( getInt( query, value ))
        _screenPosition.setX( value );

    query.setQuery( QString( "string(//process[%1]/screen/@y)" ).arg( xpathIndex ));
    if( getInt( query, value ))
        _screenPosition.setY( value );

    query.setQuery( QString( "string(//process[%1]/screen/@i)" ).arg( xpathIndex ));
    if( getInt( query, value ))
        _screenGlobalIndex.setX( value );

    query.setQuery( QString( "string(//process[%1]/screen/@j)" ).arg( xpathIndex ));
    if( getInt( query, value ))
        _screenGlobalIndex.setY( value );

    // read stereo mode
    query.setQuery( QString( "string(//process[%1]/@stereo)" ).arg( xpathIndex ));
    if( getString( query, queryResult ))
    {
        if( queryResult == "left" )
            _stereoMode = deflect::View::left_eye;
        else if( queryResult == "right" )
            _stereoMode = deflect::View::right_eye;
        else
            _stereoMode = deflect::View::mono;
    }
}

int WallConfiguration::getProcessIndex() const
{
    return _processIndex;
}

const QString& WallConfiguration::getHost() const
{
    return _host;
}

const QString& WallConfiguration::getDisplay() const
{
    return _display;
}

int WallConfiguration::getProcessCountForHost() const
{
    return _processCountForHost;
}

const QPoint& WallConfiguration::getGlobalScreenIndex() const
{
    return _screenGlobalIndex;
}

const QPoint& WallConfiguration::getWindowPos() const
{
    return _screenPosition;
}

deflect::View WallConfiguration::getStereoMode() const
{
    return _stereoMode;
}

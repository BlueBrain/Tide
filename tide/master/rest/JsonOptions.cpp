/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#include "JsonOptions.h"

#include "jsonschema.h"
#include "log.h"
#include "scene/Options.h"

#include <QJsonDocument>
#include <QJsonObject>

QJsonObject _makeJsonObject( const Options& options )
{
    QJsonObject obj;
    obj.insert( "alphaBlending", options.isAlphaBlendingEnabled( ));
    obj.insert( "autoFocusStreamers", options.getAutoFocusPixelStreams( ));
    obj.insert( "backgroundColor", options.getBackgroundColor().name( ));
    obj.insert( "background", options.getBackgroundUri( ));
    obj.insert( "clock", options.getShowClock( ));
    obj.insert( "contentTiles", options.getShowContentTiles( ));
    obj.insert( "controlArea", options.getShowControlArea( ));
    obj.insert( "statistics", options.getShowStatistics( ));
    obj.insert( "testPattern", options.getShowTestPattern( ));
    obj.insert( "touchPoints", options.getShowTouchPoints( ));
    obj.insert( "windowBorders", options.getShowWindowBorders( ));
    obj.insert( "windowTitles", options.getShowWindowTitles( ));
    obj.insert( "zoomContext", options.getShowZoomContext( ));
    return obj;
}

JsonOptions::JsonOptions( OptionsPtr options )
    : _options( options )
{}

std::string JsonOptions::getTypeName() const
{
    return "tide/options";
}

std::string JsonOptions::getSchema() const
{
    return jsonschema::create( "Options", _makeJsonObject( *_options ),
                               "Options of the Tide application" );
}

std::string JsonOptions::_toJSON() const
{
    if( !_options )
        return std::string();

    const QJsonObject obj{ _makeJsonObject( *_options ) };
    const QJsonDocument doc{ obj };
    return doc.toJson().constData();
}

bool JsonOptions::_fromJSON( const std::string& string )
{
    if( !_options )
        return false;

    const QByteArray input = QString::fromStdString( string ).toUtf8();
    const QJsonDocument doc = QJsonDocument::fromJson( input );
    if( doc.isNull() || !doc.isObject( ))
    {
        put_flog( LOG_INFO, "Error parsing JSON string: '%s'", string.c_str( ));
        return false;
    }
    const QJsonObject obj = doc.object();
    QJsonValue value;

    value = obj["alphaBlending"];
    _options->enableAlphaBlending( value.toBool( _options->isAlphaBlendingEnabled( )));
    value = obj["autoFocusStreamers"];
    _options->setAutoFocusPixelStreams( value.toBool( _options->getAutoFocusPixelStreams( )));
    value = obj["backgroundColor"];
    _options->setBackgroundColor( QColor( value.toString( _options->getBackgroundColor().name( ))));
    value = obj["background"];
    _options->setBackgroundUri( value.toString( _options->getBackgroundUri( )));
    value = obj["clock"];
    _options->setShowClock( value.toBool( _options->getShowClock( )));
    value = obj["contentTiles"];
    _options->setShowContentTiles( value.toBool( _options->getShowContentTiles( )));
    value = obj["controlArea"];
    _options->setShowControlArea( value.toBool( _options->getShowControlArea( )));
    value = obj["clock"];
    _options->setShowClock( value.toBool( _options->getShowClock( )));
    value = obj["statistics"];
    _options->setShowStatistics( value.toBool( _options->getShowStatistics( )));
    value = obj["testPattern"];
    _options->setShowTestPattern( value.toBool( _options->getShowTestPattern( )));
    value = obj["touchPoints"];
    _options->setShowTouchPoints( value.toBool( _options->getShowTouchPoints( )));
    value = obj["windowBorders"];
    _options->setShowWindowBorders( value.toBool( _options->getShowWindowBorders( )));
    value = obj["windowTitles"];
    _options->setShowWindowTitles( value.toBool( _options->getShowWindowTitles( )));
    value = obj["zoomContext"];
    _options->setShowZoomContext( value.toBool( _options->getShowZoomContext( )));

    return true;
}

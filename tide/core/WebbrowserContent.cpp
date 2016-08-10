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

#include "WebbrowserContent.h"

#include <boost/serialization/export.hpp>
#include <sstream>

BOOST_CLASS_EXPORT_IMPLEMENT( WebbrowserContent )

IMPLEMENT_SERIALIZE_FOR_XML( WebbrowserContent )

namespace
{
const QString WEBBROWSER_CONTROLS( "qrc:///qml/core/WebbrowserControls.qml" );
}

WebbrowserContent::WebbrowserContent( const QString& uri )
    : PixelStreamContent( uri )
{}

CONTENT_TYPE WebbrowserContent::getType() const
{
    return CONTENT_TYPE_WEBBROWSER;
}

bool WebbrowserContent::hasFixedAspectRatio() const
{
    return false;
}

QString WebbrowserContent::getQmlControls() const
{
    return WEBBROWSER_CONTROLS;
}

int WebbrowserContent::getPage() const
{
    return _history.currentItemIndex();
}

int WebbrowserContent::getPageCount() const
{
    return _history.items().size();
}

int WebbrowserContent::getRestPort() const
{
    return _restPort;
}

int WebbrowserContent::getCursorPosition() const
{
    return _cursorPosition;
}

void WebbrowserContent::setCursorPosition( const int arg )
{
    if( _cursorPosition == arg )
        return;

    _cursorPosition = arg;
    emit cursorPositionChanged();
}

int WebbrowserContent::getSelectionStart() const
{
    return _selectionStart;
}

void WebbrowserContent::setSelectionStart( const int pos )
{
    if( _selectionStart == pos )
        return;

    _selectionStart = pos;
    emit selectionStartChanged();
    emit modified();
}

int WebbrowserContent::getSelectionEnd() const
{
    return _selectionEnd;
}

void WebbrowserContent::setSelectionEnd( const int pos )
{
    if( _selectionEnd == pos )
        return;

    _selectionEnd = pos;
    emit selectionEndChanged();
    emit modified();
}

QString WebbrowserContent::getUrl() const
{
    return _addressBarUrl;
}

void WebbrowserContent::setUrl( const QString url )
{
    if( getUrl() == url )
        return;

    _addressBarUrl = url;
    emit urlChanged();
    emit modified();
}

bool WebbrowserContent::isAddressBarFocused() const
{
    return _addressBarFocused;
}

void WebbrowserContent::setAddressBarFocused( const bool set )
{
    if( _addressBarFocused == set )
        return;

    _addressBarFocused = set;
    emit addressBarFocusedChanged();
    emit modified();
}

void WebbrowserContent::parseData( const QByteArray data )
{
    std::istringstream iss{ data.toStdString(), std::istringstream::binary };
    {
        boost::archive::binary_iarchive ia{ iss };
        ia >> _history;
        ia >> _restPort;
    }
    _addressBarUrl = _history.currentItem();

    emit pageChanged();
    emit pageCountChanged();
    emit restPortChanged();
    emit urlChanged();
    emit modified();
}

QByteArray WebbrowserContent::serializeData( const WebbrowserHistory& history,
                                             const int restPort )
{
    std::ostringstream stream{ std::ostringstream::binary };
    {
        boost::archive::binary_oarchive oa{ stream };
        oa << history;
        oa << restPort;
    }
    const auto string = stream.str();
    return QByteArray{ string.data(), (int)string.size( )};
}

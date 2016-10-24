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

#include "AddressBar.h"

AddressBar::AddressBar( QObject* parentObject )
    : QObject( parentObject )
{}

bool AddressBar::isFocused() const
{
    return _focused;
}

void AddressBar::setFocused( const bool set )
{
    if( _focused == set )
        return;

    _focused = set;
    emit focusedChanged();
    emit modified();
}

int AddressBar::getCursorPosition() const
{
    return _cursorPosition;
}

void AddressBar::setCursorPosition( const int arg )
{
    if( _cursorPosition == arg )
        return;

    _cursorPosition = arg;
    emit cursorPositionChanged();
}

int AddressBar::getSelectionStart() const
{
    return _selectionStart;
}

void AddressBar::setSelectionStart( const int pos )
{
    if( _selectionStart == pos )
        return;

    _selectionStart = pos;
    emit selectionStartChanged();
    emit modified();
}

int AddressBar::getSelectionEnd() const
{
    return _selectionEnd;
}

void AddressBar::setSelectionEnd( const int pos )
{
    if( _selectionEnd == pos )
        return;

    _selectionEnd = pos;
    emit selectionEndChanged();
    emit modified();
}

QString AddressBar::getUrl() const
{
    return _url;
}

void AddressBar::setUrl( const QString url )
{
    if( getUrl() == url )
        return;

    _url = url;
    emit urlChanged();
    emit modified();
}

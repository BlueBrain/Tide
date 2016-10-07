/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "PixelStreamContent.h"

BOOST_CLASS_EXPORT_IMPLEMENT( PixelStreamContent )

IMPLEMENT_SERIALIZE_FOR_XML( PixelStreamContent )

namespace
{
const QString ICON_KEYBOARD( "qrc:///img/keyboard.svg" );
}

PixelStreamContent::PixelStreamContent( const QString& uri,
                                        const bool keyboard )
    : Content( uri )
    , _eventReceiversCount( 0 )
{
    if( keyboard )
        _createActions();
}

CONTENT_TYPE PixelStreamContent::getType() const
{
    return CONTENT_TYPE_PIXEL_STREAM;
}

bool PixelStreamContent::readMetadata()
{
    return true;
}

Content::Interaction PixelStreamContent::getInteractionPolicy() const
{
    return hasEventReceivers() ? Content::Interaction::ON :
                                 Content::Interaction::OFF;
}

bool PixelStreamContent::hasEventReceivers() const
{
    return _eventReceiversCount > 0;
}

void PixelStreamContent::incrementEventReceiverCount()
{
    if( ++_eventReceiversCount == 1 )
    {
        emit interactionPolicyChanged();
        emit modified();
    }
}

void PixelStreamContent::_createActions()
{
    ContentAction* keyboardAction = new ContentAction();
    keyboardAction->setCheckable( true );
    keyboardAction->setIcon( ICON_KEYBOARD );
    keyboardAction->setIconChecked( ICON_KEYBOARD );
    connect( keyboardAction, &ContentAction::triggered,
             &_keyboardState, &KeyboardState::setVisible );
    connect( &_keyboardState, &KeyboardState::visibleChanged,
             keyboardAction, &ContentAction::setChecked );
    _actions.add( keyboardAction );
}

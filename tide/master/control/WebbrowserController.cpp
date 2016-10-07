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

#include "WebbrowserController.h"

#include "scene/ContentWindow.h"
#include "scene/WebbrowserContent.h"

WebbrowserController::WebbrowserController( ContentWindow& contentWindow )
    : PixelStreamController( contentWindow )
{}

void WebbrowserController::touchBegin( const QPointF position )
{
    getWebContent().setAddressBarFocused( false );

    PixelStreamController::touchBegin( position );
}

void WebbrowserController::keyPress( const int key, const int modifiers,
                                     const QString text )
{
    if( getWebContent().isAddressBarFocused( ))
        return;

    PixelStreamController::keyPress( key, modifiers, text );
}

void WebbrowserController::keyRelease( const int key, const int modifiers,
                                       const QString text )
{
    if( getWebContent().isAddressBarFocused( ))
    {
        switch( key )
        {
        case Qt::Key_Enter:
            emit enterKeyPressed();
            break;
        case Qt::Key_Backspace:
            emit deleteKeyPressed();
            break;
        default:
            emit keyboardInput( text );
            break;
        }
        return;
    }

    PixelStreamController::keyRelease( key, modifiers, text );
}

WebbrowserContent& WebbrowserController::getWebContent()
{
    return dynamic_cast<WebbrowserContent&>( *_contentWindow.getContent( ));
}

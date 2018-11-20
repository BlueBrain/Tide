/*********************************************************************/
/* Copyright (c) 2011-2012, The University of Texas at Austin.       */
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Raphael.Dumusc@epfl.ch                   */
/*                          Daniel.Nachbaur@epfl.ch                  */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "PixelStreamContent.h"

BOOST_CLASS_EXPORT_IMPLEMENT(PixelStreamContent)

IMPLEMENT_SERIALIZE_FOR_XML(PixelStreamContent)

namespace
{
const QString ICON_KEYBOARD("qrc:///img/keyboard.svg");
const QUuid UUID_NAMESPACE{"{11111111-1111-1111-1111-111111111111}"};
}

QUuid PixelStreamContent::getStreamId(const QString& uri)
{
    return QUuid::createUuidV3(UUID_NAMESPACE, uri);
}

PixelStreamContent::PixelStreamContent(const QString& uri, const QSize& size,
                                       const bool keyboard)
    : MultiChannelContent{uri, getStreamId(uri)}
{
    setDimensions(size);
    if (keyboard)
        _createKeyboard();
}

PixelStreamContent::PixelStreamContent(const bool keyboard)
{
    if (keyboard)
        _createKeyboard();
}

ContentType PixelStreamContent::getType() const
{
    return ContentType::pixel_stream;
}

bool PixelStreamContent::readMetadata()
{
    return true;
}

KeyboardState* PixelStreamContent::getKeyboardState()
{
    return _keyboardState;
}

bool PixelStreamContent::hasEventReceivers() const
{
    return _eventReceiversCount > 0;
}

void PixelStreamContent::incrementEventReceiverCount()
{
    if (++_eventReceiversCount == 1)
    {
        emit interactionPolicyChanged();
        emit modified();
    }
}

Content::Interaction PixelStreamContent::_getInteractionPolicy() const
{
    return hasEventReceivers() ? Content::Interaction::on
                               : Content::Interaction::off;
}

void PixelStreamContent::_createKeyboard()
{
    _keyboardState = new KeyboardState(this); // child QObject
    connect(_keyboardState, &KeyboardState::modified, this, &Content::modified);
}

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

#include "Content.h"

#include "serialization/utils.h"

IMPLEMENT_SERIALIZE_FOR_XML(Content)

qreal Content::_maxScale = 3.0;

Content::Content(const QString& uri)
    : _uri(uri)
    , _zoomRect(UNIT_RECTF)
    , _actions(this)
    , _keyboardState(this)
    , _captureInteraction(false)
{
    _init();
}

Content::Content()
    : _zoomRect(UNIT_RECTF)
    , _actions(this)
    , _keyboardState(this)
    , _captureInteraction(false)
{
    _init();
}

ContentPtr Content::clone() const
{
    return ContentPtr{const_cast<Content*>(serialization::binaryCopy(this))};
}

void Content::_init()
{
    connect(this, &Content::interactionPolicyChanged, this,
            &Content::captureInteractionChanged);

    connect(&_keyboardState, &KeyboardState::modified, this,
            &Content::modified);
}

const QString& Content::getURI() const
{
    return _uri;
}

QString Content::getTitle() const
{
    return getURI().section("/", -1, -1);
}

QSize Content::getDimensions() const
{
    return _size;
}

int Content::width() const
{
    return _size.width();
}

int Content::height() const
{
    return _size.height();
}

QSize Content::getMinDimensions() const
{
    if (_sizeHints.minWidth == deflect::SizeHints::UNSPECIFIED_SIZE ||
        _sizeHints.minHeight == deflect::SizeHints::UNSPECIFIED_SIZE)
    {
        return UNDEFINED_SIZE;
    }
    return QSize(_sizeHints.minWidth, _sizeHints.minHeight);
}

QSize Content::getPreferredDimensions() const
{
    if (_sizeHints.preferredWidth == deflect::SizeHints::UNSPECIFIED_SIZE ||
        _sizeHints.preferredHeight == deflect::SizeHints::UNSPECIFIED_SIZE)
    {
        return getDimensions();
    }
    return QSize(_sizeHints.preferredWidth, _sizeHints.preferredHeight);
}

QSize Content::getMaxDimensions() const
{
    if (_sizeHints.maxWidth == deflect::SizeHints::UNSPECIFIED_SIZE ||
        _sizeHints.maxHeight == deflect::SizeHints::UNSPECIFIED_SIZE)
    {
        return getDimensions().isValid() ? getDimensions() * getMaxScale()
                                         : UNDEFINED_SIZE;
    }
    return QSize(_sizeHints.maxWidth, _sizeHints.maxHeight);
}

void Content::setSizeHints(const deflect::SizeHints& sizeHints)
{
    if (_sizeHints == sizeHints)
        return;
    _sizeHints = sizeHints;
    emit modified();
}

void Content::setMaxScale(const qreal value)
{
    if (value > 0)
        _maxScale = value;
}

qreal Content::getMaxScale()
{
    return _maxScale;
}

Content::Interaction Content::getInteractionPolicy() const
{
    return Content::Interaction::AUTO;
}

QString Content::getQmlControls() const
{
    return QString();
}

bool Content::getCaptureInteraction() const
{
    switch (getInteractionPolicy())
    {
    case Content::Interaction::OFF:
        return false;
    case Content::Interaction::ON:
        return true;
    case Content::Interaction::AUTO:
    default:
        return _captureInteraction;
    }
}

void Content::setCaptureInteraction(const bool enable)
{
    if (_captureInteraction == enable ||
        getInteractionPolicy() != Content::Interaction::AUTO)
    {
        return;
    }

    _captureInteraction = enable;
    emit captureInteractionChanged();
    emit modified();
}

void Content::setDimensions(const QSize& dimensions)
{
    if (_size == dimensions)
        return;

    _size = dimensions;
    emit dimensionsChanged();
    emit modified();
}

qreal Content::getAspectRatio() const
{
    if (_size.height() == 0)
        return 0.0;
    return (qreal)_size.width() / (qreal)_size.height();
}

bool Content::canBeZoomed() const
{
    return false;
}

bool Content::hasFixedAspectRatio() const
{
    return true;
}

const QRectF& Content::getZoomRect() const
{
    return _zoomRect;
}

void Content::setZoomRect(const QRectF& zoomRect)
{
    if (!canBeZoomed() || _zoomRect == zoomRect)
        return;

    _zoomRect = zoomRect;
    emit modified();
}

void Content::resetZoom()
{
    setZoomRect(UNIT_RECTF);
}

ContentActionsModel* Content::getActions()
{
    return &_actions;
}

KeyboardState* Content::getKeyboardState()
{
    return &_keyboardState;
}

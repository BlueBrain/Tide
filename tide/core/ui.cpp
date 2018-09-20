/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#include "ui.h"

namespace ui
{
namespace
{
/** Hardcoded dimensions (currently also defined in style.js). */
const qreal buttonsSize = 106;
const qreal buttonsPadding = buttonsSize / 4.0;
const qreal buttonsSizeLarge = 1.15 * buttonsSize;
const qreal buttonsPaddingLarge = 1.15 * buttonsPadding;

/** Derived UI element sizes. */
const qreal movieBarHeight = buttonsSize;
const qreal controlsWidth = buttonsSize + 2 * buttonsPadding;
const qreal controlsDistanceFromWindow = buttonsPadding;

/** Constraints. */
const qreal minWindowSize = 300;
const qreal minWindowSpacingInFocusMode = 40;

inline bool hasMovieControls(const Content& content)
{
#if TIDE_ENABLE_MOVIE_SUPPORT
    return content.getType() == ContentType::movie;
#else
    Q_UNUSED(content);
    return false;
#endif
}
}

qreal getButtonSize()
{
    return buttonsSize;
}

qreal getWindowControlsMargin()
{
    return controlsWidth + controlsDistanceFromWindow;
}

qreal getMinWindowSpacing()
{
    return minWindowSpacingInFocusMode;
}

qreal getFocusedWindowControlBarHeight(const Content& content)
{
    return hasMovieControls(content) ? movieBarHeight : 0.0;
}

QMarginsF getFocusedWindowControlsMargins(const Content& content)
{
    const auto leftMargin = getWindowControlsMargin();
    const auto topMargin = getFocusedWindowControlBarHeight(content);
    return QMarginsF{leftMargin, topMargin, 0.0, 0.0};
}

qreal getSideControlWidth()
{
    return buttonsSizeLarge + 2.0 * buttonsPaddingLarge;
}

QRectF getFocusSurface(const DisplayGroup& group)
{
    const auto margin = buttonsSize;
    const auto rightMargin = margin;
    const auto leftMargin = margin + getSideControlWidth();
    const auto topMargin = margin;
    const auto bottomMargin = margin;
    return QRectF{leftMargin, topMargin,
                  group.width() - leftMargin - rightMargin,
                  group.height() - topMargin - bottomMargin};
}

qreal getMinWindowSize()
{
    return minWindowSize;
}
}

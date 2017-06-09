/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Nataniel Hofer <nataniel.hofer@epfl.ch>       */
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

#ifndef LAYOUTPOLICY_H
#define LAYOUTPOLICY_H

#include "types.h"

// Hardcoded variables which are also defined in style.js
namespace controlSpecifications
{
const qreal INSIDE_MARGIN_RELATIVE = 0.02;
const qreal SIDEBAR_WIDTH_REL_TO_DISPLAYGROUP_HEIGHT = 0.3 * 0.3;
const qreal WINDOW_CONTROLS_MARGIN_PX = 200.0;
const qreal WINDOW_SPACING_PX = 80.0;
const qreal WINDOW_TITLE_HEIGHT = 100.0;
const qreal MOVIE_BAR_HEIGHT = 100.0;
}

/**
 * Abstract class whose derived classes must deal with the layout in focus
 * coordinates.
 */
class LayoutPolicy
{
public:
    LayoutPolicy(const DisplayGroup& group);

    /** @return the focused coordinates for the window. */
    virtual QRectF getFocusedCoord(const ContentWindow& window) const = 0;

    /** Update the focused coordinates for the set of windows. */
    virtual void updateFocusedCoord(const ContentWindowSet& windows) const = 0;
    virtual ~LayoutPolicy(){};

protected:
    const DisplayGroup& _group;

    /**
     * returns the rect of available space on the display group
     */
    QRectF _getAvailableSpace() const;
};

#endif

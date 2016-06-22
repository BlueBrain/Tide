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

#ifndef DISPLAYGROUPCONTROLLER_H
#define DISPLAYGROUPCONTROLLER_H

#include "types.h"

/** Controller for rescaling and adjusting DisplayGroup. */
class DisplayGroupController
{
public:
    /** Constructor */
    DisplayGroupController( DisplayGroup& group );

    /** Scale the DisplayGroup and its windows by the given x and y factors. */
    void scale( const QSizeF& factor );

    /** Rescale to fit inside the given size, preserving aspect ratio. */
    void adjust( const QSizeF& maxGroupSize );

    /** Reshape to fit inside the given size, scaling and centering windows. */
    void reshape( const QSizeF& newSize );

    /** Transform from normalized coordinates to pixel coordinates. */
    void denormalize( const QSizeF& targetSize );

    /** Resize windows in place so that their aspect ratio matches content's. */
    void adjustWindowsAspectRatioToContent();

    /** Estimate the surface covered by the windows in the group. */
    QRectF estimateSurface() const;

private:
    DisplayGroup& _group;

    /** Extend the DisplayGroup surface, keeping the windows centered. */
    void _extend( const QSizeF& newSize );

    qreal _estimateAspectRatio() const;
};

#endif // DISPLAYGROUPCONTROLLER_H

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

#include "SurfaceConfigValidator.h"

#include <cmath>

namespace
{
const auto maxError = 0.01;
const auto errorMessage = QString(
    "The aspect ratio of the surface in pixels (%1 x %2) -> %3 "
    "does not match that of the dimensions in meters (%4 x %5) -> %6");

inline double _getAspectRatio(const QSizeF& size)
{
    return size.width() / size.height();
}
}

SurfaceConfigValidator::SurfaceConfigValidator(const SurfaceConfig& surface)
    : _surface{surface}
{
}

void SurfaceConfigValidator::validateDimensions() const
{
    if (_surface.dimensions.isEmpty())
        return;

    const auto dimensionsAR = _getAspectRatio(_surface.dimensions);
    const auto pixelsAR = _surface.getAspectRatio();

    if (std::abs(dimensionsAR - pixelsAR) > maxError)
    {
        throw dimensions_mismatch(errorMessage.arg(_surface.getTotalWidth())
                                      .arg(_surface.getTotalHeight())
                                      .arg(pixelsAR)
                                      .arg(_surface.dimensions.width())
                                      .arg(_surface.dimensions.height())
                                      .arg(dimensionsAR)
                                      .toStdString());
    }
}

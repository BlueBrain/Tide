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

#include "SurfaceConfig.h"

uint SurfaceConfig::getScreenWidth() const
{
    return displayWidth * displaysPerScreenX +
           ((displaysPerScreenX - 1) * bezelWidth);
}

uint SurfaceConfig::getScreenHeight() const
{
    return displayHeight * displaysPerScreenY +
           ((displaysPerScreenY - 1) * bezelHeight);
}

QRect SurfaceConfig::getScreenRect(const QPoint& tileIndex) const
{
    if (tileIndex.x() < 0 || tileIndex.x() >= (int)screenCountX ||
        tileIndex.y() < 0 || tileIndex.y() >= (int)screenCountY)
    {
        throw std::invalid_argument("tile index does not exist");
    }

    const int xPos = tileIndex.x() * (getScreenWidth() + bezelWidth);
    const int yPos = tileIndex.y() * (getScreenHeight() + bezelHeight);

    return QRect(xPos, yPos, getScreenWidth(), getScreenHeight());
}

uint SurfaceConfig::getTotalWidth() const
{
    return screenCountX * getScreenWidth() + (screenCountX - 1) * bezelWidth;
}

uint SurfaceConfig::getTotalHeight() const
{
    return screenCountY * getScreenHeight() + (screenCountY - 1) * bezelHeight;
}

QSize SurfaceConfig::getTotalSize() const
{
    return QSize(getTotalWidth(), getTotalHeight());
}

double SurfaceConfig::getAspectRatio() const
{
    if (getTotalHeight() == 0)
        return 0.0;
    return double(getTotalWidth()) / getTotalHeight();
}

/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#include "StateThumbnailGenerator.h"

#include "StatePreview.h"

#include <QPainter>

StateThumbnailGenerator::StateThumbnailGenerator(const QSize& size)
    : ThumbnailGenerator(size)
{
}

QImage StateThumbnailGenerator::generate(const QString& filename) const
{
    QImage thumbnail = createGradientImage(Qt::darkCyan, Qt::cyan);

    StatePreview filePreview(filename);
    if (filePreview.loadFromFile())
    {
        const QImage preview = filePreview.getImage();
        const QSize newSize =
            preview.size().scaled(thumbnail.size(), Qt::KeepAspectRatio);
        thumbnail = thumbnail.scaled(newSize);
        QPainter painter(&thumbnail);
        const QRect rect = _scaleRectAroundCenter(thumbnail.rect(), 0.9f);
        painter.drawImage(rect, preview);
    }
    else
        paintText(thumbnail, "session");

    return thumbnail;
}

QRect StateThumbnailGenerator::_scaleRectAroundCenter(
    const QRect& rect, const float scaleFactor) const
{
    const float topLeftFactor = 0.5f * (1.0f - scaleFactor);

    return QRect(topLeftFactor * rect.width(), topLeftFactor * rect.height(),
                 scaleFactor * rect.width(), scaleFactor * rect.height());
}

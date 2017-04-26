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

#include "StatePreview.h"

#include <QRectF>

#include "log.h"
#include "scene/ContentWindow.h"
#include "thumbnail/thumbnail.h"

#include <QFileInfo>
#include <QImageReader>
#include <QPainter>

namespace
{
const QSize PREVIEW_IMAGE_SIZE(512, 512);
}

StatePreview::StatePreview(const QString& dcxFileName)
    : _dcxFileName(dcxFileName)
{
}

QString StatePreview::getFileExtension()
{
    return QString(".dcxpreview");
}

QImage StatePreview::getImage() const
{
    return _previewImage;
}

QString StatePreview::previewFilename() const
{
    QFileInfo fileinfo(_dcxFileName);

    const QString extension = fileinfo.suffix().toLower();
    if (extension != "dcx")
    {
        put_flog(LOG_WARN,
                 "wrong state file extension: '%s' for session: '%s'"
                 "(expected: .dcx)",
                 extension.toLocal8Bit().constData(),
                 _dcxFileName.toLocal8Bit().constData());
        return QString();
    }
    return fileinfo.path() + "/" + fileinfo.completeBaseName() +
           getFileExtension();
}

void StatePreview::generateImage(const QSize& wallDimensions,
                                 const ContentWindowPtrs& contentWindows)
{
    const auto previewDimension =
        wallDimensions.scaled(PREVIEW_IMAGE_SIZE, Qt::KeepAspectRatio);
    // Transparent image
    QImage preview(previewDimension, QImage::Format_ARGB32);
    preview.fill(qRgba(0, 0, 0, 0));

    // Paint all Contents at their correct location
    QPainter painter{&preview};
    for (const auto& window : contentWindows)
    {
        const auto ratio =
            (qreal)previewDimension.width() / (qreal)wallDimensions.width();
        const auto area = QRectF{window->getCoordinates().topLeft() * ratio,
                                 window->size() * ratio};
        const auto thumbnail =
            thumbnail::create(*window->getContent(), area.size().toSize());
        painter.drawImage(area, thumbnail);
    }
    painter.end();

    _previewImage = preview;
}

bool StatePreview::saveToFile() const
{
    const bool success = _previewImage.save(previewFilename(), "PNG");

    if (!success)
        put_flog(LOG_ERROR, "Saving StatePreview image failed: '%s'",
                 previewFilename().toLocal8Bit().constData());

    return success;
}

bool StatePreview::loadFromFile()
{
    QImageReader reader(previewFilename());
    return reader.canRead() && reader.read(&_previewImage);
}

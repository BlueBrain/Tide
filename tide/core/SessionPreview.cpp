/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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

#include "SessionPreview.h"

#include "utils/log.h"

#include <QFileInfo>

namespace
{
const auto previewFileExtension = ".dcxpreview";
const auto previewFileFormat = "PNG";
}

SessionPreview::SessionPreview(const QString& sessionFile)
    : _sessionFile{sessionFile}
{
}

QImage SessionPreview::loadFromFile() const
{
    return QImage{_getPreviewFilename()};
}

bool SessionPreview::saveToFile(const QImage& image) const
{
    const auto success = image.save(_getPreviewFilename(), previewFileFormat);
    if (!success)
        print_log(LOG_ERROR, LOG_CONTENT,
                  "Saving SessionPreview image failed: '%s'",
                  _getPreviewFilename().toLocal8Bit().constData());
    return success;
}

QString SessionPreview::_getPreviewFilename() const
{
    const auto fileinfo = QFileInfo{_sessionFile};
    const auto extension = fileinfo.suffix().toLower();
    if (extension != "dcx")
    {
        print_log(LOG_WARN, LOG_CONTENT,
                  "wrong file extension: '%s' for session: '%s'"
                  "(expected: .dcx)",
                  extension.toLocal8Bit().constData(),
                  _sessionFile.toLocal8Bit().constData());
        return QString();
    }
    return fileinfo.path() + "/" + fileinfo.completeBaseName() +
           previewFileExtension;
}

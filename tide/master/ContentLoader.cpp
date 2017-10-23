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

#include "ContentLoader.h"

#include "control/ContentWindowController.h"
#include "log.h"
#include "scene/ContentFactory.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"

#include <QDir>
#include <cmath>

#include <cmath>

ContentLoader::ContentLoader(DisplayGroupPtr displayGroup)
    : _displayGroup(displayGroup)
{
}

bool ContentLoader::load(const QString& filename,
                         const QPointF& windowCenterPosition,
                         const QSizeF& windowSize)
{
    put_facility_flog(LOG_INFO, LOG_CONTENT, "opening: '%s'",
                      filename.toLocal8Bit().constData());

    if (isAlreadyOpen(filename))
    {
        put_facility_flog(LOG_INFO, LOG_CONTENT, "file already opened: '%s'",
                          filename.toLocal8Bit().constData());
        return false;
    }

    ContentPtr content = ContentFactory::getContent(filename);
    if (!content)
    {
        put_facility_flog(LOG_WARN, LOG_CONTENT,
                          "ignoring unsupported file: '%s'",
                          filename.toLocal8Bit().constData());
        return false;
    }

    ContentWindowPtr contentWindow(new ContentWindow(content));
    ContentWindowController controller(*contentWindow, *_displayGroup);

    if (windowSize.isValid())
        controller.resize(windowSize);
    else
        controller.adjustSize(SIZE_1TO1_FITTING);

    if (windowCenterPosition.isNull())
        controller.moveCenterTo(_displayGroup->getCoordinates().center());
    else
        controller.moveCenterTo(windowCenterPosition);

    _displayGroup->addContentWindow(contentWindow);

    return true;
}

QSize _estimateGridSize(const int numElem)
{
    if (numElem <= 0)
        return QSize();

    const auto w = int(ceil(sqrt(numElem)));
    return {w, (w * (w - 1) >= numElem) ? w - 1 : w};
}

size_t ContentLoader::loadDir(const QString& dirName, QSize gridSize)
{
    put_facility_flog(LOG_INFO, LOG_CONTENT, "opening directory: '%s'",
                      dirName.toLocal8Bit().constData());

    QDir directory(dirName);
    directory.setFilter(QDir::Files);
    directory.setNameFilters(ContentFactory::getSupportedFilesFilter());

    const auto list = directory.entryInfoList();
    if (list.empty())
        return 0;

    if (gridSize.isEmpty())
        gridSize = _estimateGridSize(list.size());

    const QSizeF win(_displayGroup->width() / (qreal)gridSize.width(),
                     _displayGroup->height() / (qreal)gridSize.height());

    int contentIndex = 0;
    for (const auto& fileinfo : list)
    {
        const auto filename = fileinfo.absoluteFilePath();
        const auto x = contentIndex % gridSize.width();
        const auto y = contentIndex / gridSize.width();
        const auto position = QPointF{x * win.width() + 0.5 * win.width(),
                                      y * win.height() + 0.5 * win.height()};

        if (load(filename, position, win))
            ++contentIndex;

        if (contentIndex >= gridSize.width() * gridSize.height())
            break; // should not happen if grid size is correct
    }

    put_facility_flog(LOG_INFO, LOG_CONTENT,
                      "done opening %d contents from directory: '%s'",
                      contentIndex, dirName.toLocal8Bit().constData());

    return contentIndex;
}

bool ContentLoader::isAlreadyOpen(const QString& filename) const
{
    for (const auto& window : _displayGroup->getContentWindows())
    {
        if (window->getContent()->getURI() == filename)
            return true;
    }
    return false;
}

ContentWindowPtr ContentLoader::findWindow(const QString& filename) const
{
    for (const auto& window : _displayGroup->getContentWindows())
    {
        if (window->getContent()->getURI() == filename)
            return window;
    }
    return {};
}

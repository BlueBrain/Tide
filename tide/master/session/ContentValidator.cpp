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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "ContentValidator.h"

#include "scene/ContentFactory.h"
#include "scene/Scene.h"
#include "utils/log.h"

#include <QtConcurrent>

namespace
{
bool _canBeRestored(const ContentType type)
{
    // PixelStreams are external applications and can't be restored.
    return type != ContentType::pixel_stream;
}

bool _validateContent(const WindowPtr& window)
{
    auto content = window->getContentPtr();
    if (!content)
    {
        print_log(LOG_WARN, LOG_CONTENT, "Window '%s' does not have a Content!",
                  window->getID().toString().toLocal8Bit().constData());
        return false;
    }

    if (!_canBeRestored(content->getType()))
        return false;

    // Some regular textures were saved as DynamicTexture type before the
    // migration to qml2 rendering
    if (content->getType() == ContentType::dynamic_texture)
    {
        const auto& uri = content->getUri();
        if (ContentFactory::isValidImageFile(uri))
        {
            print_log(LOG_DEBUG, LOG_CONTENT,
                      "Restoring legacy DynamicTexture as "
                      "a regular image: '%s'",
                      content->getUri().toLocal8Bit().constData());

            auto newContent = ContentFactory::createContent(uri);
            newContent->moveToThread(window->thread());
            window->setContent(std::move(newContent));
            content = window->getContentPtr();
        }
        else
        {
            print_log(LOG_INFO, LOG_CONTENT,
                      "DynamicTexture are no longer supported. Please"
                      "convert the source image to a tiff pyramid: "
                      "'%s'",
                      content->getUri().toLocal8Bit().constData());
        }
    }

    // Refresh content information, files can have been modified or removed
    // since the session was saved.
    if (window->getContent().readMetadata())
    {
        print_log(LOG_DEBUG, LOG_CONTENT, "Restoring content: '%s'",
                  content->getUri().toLocal8Bit().constData());
    }
    else
    {
        print_log(LOG_WARN, LOG_CONTENT, "'%s' could not be restored!",
                  content->getUri().toLocal8Bit().constData());
        auto errorContent = ContentFactory::createErrorContent(*content);
        errorContent->moveToThread(window->thread());
        window->setContent(std::move(errorContent));
    }
    return true;
}

void _validateContents(DisplayGroup& group)
{
    auto windows = QVector<WindowPtr>::fromStdVector(group.getWindows());

    QtConcurrent::blockingFilter(windows, _validateContent);

    group.replaceWindows(windows.toStdVector());
}
}

void ContentValidator::validateContents(Scene& scene)
{
    for (auto&& surface : scene.getSurfaces())
        _validateContents(surface.getGroup());
}

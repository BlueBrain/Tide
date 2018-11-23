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

#ifndef CONTENTFACTORY_H
#define CONTENTFACTORY_H

#include "scene/Content.h"
#include "scene/ContentType.h"
#include "types.h"

#include <QStringList>

class ContentFactory
{
public:
    /**
     * Create a Content of the appropriate type based on the given URI.
     * @throw load_error if the content can't be opened.
     */
    static ContentPtr createContent(const QString& uri);

    /** Special case: PixelStreamContent type cannot be derived from its uri. */
    static ContentPtr createPixelStreamContent(
        const QString& uri, const QSize& size,
        StreamType stream = StreamType::EXTERNAL);

    /** Create a Content representing a loading error. */
    static ContentPtr createErrorContent(const Content& content);

    /** @name For legacy sessions; deprecated */
    //@{
    static bool isValidImageFile(const QString& uri);
    static ContentPtr createErrorContent(const QString& uri);
    //@}

    /** Get all the supported file extensions. */
    static const QStringList& getSupportedExtensions();

    /**
     * Get all the supported file extensions prefixed with '.' for use in Qt
     * file filters.
     */
    static const QStringList& getSupportedFilesFilter();

    /** Get all the file extensions prefixed with '.' in one string. */
    static QString getSupportedFilesFilterAsString();
};

#endif

/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#ifndef ASYNIMAGELOADER_H
#define ASYNIMAGELOADER_H

#include <QtCore/QObject>
#include <QtCore/QCache>
#include <QtGui/QImage>

/**
 * Load image thumbnails for supported content types.
 *
 * The AsyncImageLoader is designed to be moved to a worker thread.
 * It maintain a cache with the 100 latest images for performance.
 */
class AsyncImageLoader : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param defaultSize The desired size for the thumbnails.
     */
    AsyncImageLoader( const QSize& defaultSize );

public slots:
    /**
     * Load an image thumbnail.
     *
     * This method is blocking; it designed to be call via a QSignal after the
     * AsyncImageLoader has been moved to a worker thread.
     * @param filename The path to the content file.
     * @param index A user-defined index that will be passed back with
     *        imageLoaded(). Used by DockPixelStreamer.
     */
    void loadImage( const QString& filename, int index );

signals:
    /**
     * Emitted when an image could be successfully loaded by loadImage().
     *
     * @param index The user-defined index passed in loadImage().
     * @param image The thumbnail image.
     */
    void imageLoaded( int index, QImage image );

    /** Emitted whenever loadImage() has finished (successful or not). */
    void imageLoadingFinished();

private:
    Q_DISABLE_COPY( AsyncImageLoader )

    QSize _defaultSize;
    QCache<QString, QImage> _cache;

    bool imageInCache( const QString& filename ) const;
};


#endif // ASYNIMAGELOADER_H

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

#ifndef DOCKPIXELSTREAMER_H
#define DOCKPIXELSTREAMER_H

#include "PixelStreamer.h"

#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QHash>
#include <QtCore/QVector>
#include <QtCore/QLinkedList>
#include <QtGui/QImage>

class PictureFlow;
class AsyncImageLoader;
class DockToolbar;

class DockPixelStreamer : public PixelStreamer
{
    Q_OBJECT

public:
    static QString getUniqueURI();
    static float getDefaultAspectRatio();

    DockPixelStreamer(const QSize& desiredDockSize, const QString& rootDir);
    ~DockPixelStreamer();

    QSize size() const override;

    bool setRootDir(const QString& dir);

    static QSize constrainSize(const QSize& targetSize);

public slots:
    void processEvent(deflect::Event evt) override;

private slots:
    void update(const QImage &image);
    void loadThumbnails(int newCenterIndex);
    void loadNextThumbnailInList();

signals:
    void renderPreview( const QString& fileName, const int index );

private:
    PictureFlow* flow_;
    AsyncImageLoader* loader_;
    DockToolbar* toolbar_;

    QThread loadThread_;

    QString rootDir_;
    QDir currentDir_;
    QHash< QString, int > slideIndex_;

    typedef QPair<bool, QString> SlideImageLoadingStatus;
    QVector<SlideImageLoadingStatus> slideImagesLoaded_;
    QLinkedList<int> slideImagesToLoad_;

    void createFlow(const QSize& dockSize);
    void createToolbar(const QSize& toolbarSize);
    void createImageLoader();

    void processClickEvent(const deflect::Event& clickEvent);
    void onItem();
    void changeDirectory( const QString& dir );
    void addRootDirToFlow();
    void addFilesToFlow();
    void addFoldersToFlow();

    static QSize getMinSize();
    static QSize getMaxSize();
};

#endif // DOCKPIXELSTREAMER_H

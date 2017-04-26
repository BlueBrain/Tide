/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include "types.h"

#include <QFutureWatcher>
#include <QList>
#include <QObject>

/**
 * Load image data in parallel.
 */
class DataProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DataProvider)

public:
    /**
     * Construct a data provider.
     * @param view to use for stereo contents.
     */
    explicit DataProvider(deflect::View view);

    /** Destructor. */
    ~DataProvider();

    /** Get the data source for the given stream uri. */
    PixelStreamUpdaterSharedPtr getStreamDataSource(const QString& uri);

public slots:
    /** Load an image asynchronously. */
    void loadAsync(ContentSynchronizerSharedPtr source, TileWeakPtr tile);

    /** Add a new frame. */
    void setNewFrame(deflect::FramePtr frame);

signals:
    /** Notify that loadAsync has completed for the given tile. */
    void imageLoaded(ImagePtr image, TileWeakPtr tile);

    /** Emitted to request a new frame after a successful swap. */
    void requestFrame(QString uri);

private:
    const deflect::View _view;

    typedef QFutureWatcher<void> Watcher;
    QList<Watcher*> _watchers;

    std::map<QString, PixelStreamUpdaterWeakPtr> _streamUpdaters;

    void _load(ContentSynchronizerSharedPtr source, TileWeakPtr tile);
    void _handleFinished();
};

#endif

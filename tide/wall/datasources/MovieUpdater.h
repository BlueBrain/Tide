/*********************************************************************/
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project                  */
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

#ifndef MOVIEUPDATER_H
#define MOVIEUPDATER_H

#include "datasources/DataSource.h"
#include "tools/ElapsedTimer.h"
#include "tools/FpsCounter.h"
#include "types.h"

#include <QMutex>
#include <QObject>

/**
 * Updates Movies synchronously across different processes.
 *
 * A single movie is designed to provide images to multiple windows on each
 * process.
 */
class MovieUpdater : public QObject, public DataSource
{
    Q_OBJECT
    Q_DISABLE_COPY(MovieUpdater)

public:
    explicit MovieUpdater(const QString& uri);
    ~MovieUpdater();

    /** @copydoc DataSource::isDynamic */
    bool isDynamic() const final { return true; }
    /** Update this datasource according to visibility and movie content. */
    void update(const Content& content) final;

    /**
     * @copydoc DataSource::getTileImage
     * threadsafe
     */
    ImagePtr getTileImage(uint tileIndex, deflect::View view) const final;

    /** @copydoc DataSource::getTileRect */
    QRect getTileRect(uint tileIndex) const final;

    /** @copydoc DataSource::getTilesArea */
    QSize getTilesArea(uint lod, uint channel) const final;

    /** @copydoc DataSource::computeVisibleSet */
    Indices computeVisibleSet(const QRectF& visibleTilesArea, uint lod,
                              uint channel) const final;

    /** @copydoc DataSource::getMaxLod */
    uint getMaxLod() const final;

    /** @copydoc DataSource::allowNextFrame */
    void allowNextFrame() final;

    /** @copydoc DataSource::synchronizeFrameAdvance */
    void synchronizeFrameAdvance(WallToWallChannel& channel) final;

    /** @return current / max fps, movie position in percentage. */
    QString getStatistics() const;

    /** @return current position of the movie, normalized between [0.0, 1.0]. */
    qreal getPosition() const;

    /** @return true if the user is currently skipping the movie. */
    bool isSkipping() const;

    /** @return true if the movie is paused. */
    bool isPaused() const;

    /** @return skip position of the movie, normalized between [0.0, 1.0]. */
    qreal getSkipPosition() const;

signals:
    /** Emitted when a new picture has become available. */
    void pictureUpdated();

private:
    std::unique_ptr<FFMPEGMovie> _ffmpegMovie;

    bool _paused = false;
    bool _loop = true;
    bool _skipping = false;
    double _skipPosition = 0.0;

    bool _readyForNextFrame = true;

    ElapsedTimer _timer;
    double _elapsedTime = 0.0;

    mutable QMutex _mutex;
    mutable double _sharedTimestamp = 0.0;
    mutable double _currentPosition = -1.0;
    mutable bool _loopedBack = false;

    mutable PicturePtr _pictureLeftOrMono;
    mutable PicturePtr _pictureRight;

    mutable QMutex _getImageMutex;

    void _triggerFrameUpdate();
    void _exchangeSharedTimestamp(WallToWallChannel& channel, bool isCandidate);
};

#endif

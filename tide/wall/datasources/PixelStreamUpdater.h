/*********************************************************************/
/* Copyright (c) 2015-2018, EPFL/Blue Brain Project                  */
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

#ifndef PIXELSTREAMUPDATER_H
#define PIXELSTREAMUPDATER_H

#include "types.h"

#include "DataSource.h"
#include "tools/SwapSyncObject.h"

#include <QObject>
#include <QReadWriteLock>

class PixelStreamProcessor;

/**
 * Synchronize the update of PixelStreams and send new frame requests.
 */
class PixelStreamUpdater : public QObject, public DataSource
{
    Q_OBJECT
    Q_DISABLE_COPY(PixelStreamUpdater)

public:
    /** Constructor. */
    PixelStreamUpdater(const QString& uri);

    /** Destructor. */
    ~PixelStreamUpdater();

    /** @return the uri of the stream that was passed to the constructor. */
    QString getUri() const final;

    /** @copydoc DataSource::isDynamic */
    bool isDynamic() const final { return true; }
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

    /** Set the frame to be rendered next. */
    void setNextFrame(deflect::server::FramePtr frame);

signals:
    /** Emitted when a new picture has become available. */
    void pictureUpdated();

    /** Emitted to request a new frame after a successful swap. */
    void requestFrame(QString uri);

private:
    QString _uri;
    SwapSyncObject<deflect::server::FramePtr> _swapSyncFrame;
    deflect::server::FramePtr _frameLeftOrMono;
    deflect::server::FramePtr _frameRight;
    std::unique_ptr<PixelStreamProcessor> _processorLeft;
    std::unique_ptr<PixelStreamProcessor> _processRight;
    mutable QReadWriteLock _frameMutex;
    mutable std::unique_ptr<std::vector<std::mutex>> _perTileLock;
    bool _readyToSwap = true;

    void _onFrameSwapped(deflect::server::FramePtr frame);
    void _createFrameProcessors();
    void _createPerTileMutexes();
};

#endif

/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#ifndef PIXELSTREAMUPDATER_H
#define PIXELSTREAMUPDATER_H

#include "types.h"

#include "DataSource.h"
#include "SwapSyncObject.h"

#include <QObject>
#include <QReadWriteLock>

/**
 * Synchronize the update of PixelStreams and send new frame requests.
 */
class PixelStreamUpdater : public QObject, public DataSource
{
    Q_OBJECT
    Q_DISABLE_COPY( PixelStreamUpdater )

public:
    /**
     * Constructor.
     * @param view which the data source provides. Left and right views also
     *        include mono contents.
     */
    PixelStreamUpdater( deflect::View view );

    /**
     * @copydoc DataSource::getTileImage
     * threadsafe
     */
    ImagePtr getTileImage( uint tileIndex ) const final;

    /** @copydoc DataSource::getTileRect */
    QRect getTileRect( uint tileIndex ) const final;

    /** @copydoc DataSource::getTilesArea */
    QSize getTilesArea( uint lod ) const final;

    /** @copydoc DataSource::computeVisibleSet */
    Indices computeVisibleSet( const QRectF& visibleTilesArea,
                               uint lod ) const final;

    /** @copydoc DataSource::getMaxLod */
    uint getMaxLod() const final;

    /** Synchronize the update of the PixelStreams. */
    void synchronizeFramesSwap( WallToWallChannel& channel );

    /** Allow the updater to request next frame (flow control). */
    void getNextFrame();

public slots:
    /** Update the appropriate PixelStream with the given frame. */
    void updatePixelStream( deflect::FramePtr frame );

signals:
    /** Emitted when a new picture has become available. */
    void pictureUpdated();

    /** Emitted to request a new frame after a successful swap. */
    void requestFrame( QString uri );

private:
    const deflect::View _view;
    SwapSyncObject<deflect::FramePtr> _swapSyncFrame;
    deflect::FramePtr _currentFrame;
    mutable QReadWriteLock _mutex;
    bool _readyToSwap = true;

    void _onFrameSwapped( deflect::FramePtr frame );
};

#endif

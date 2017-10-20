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

#ifndef PIXELSTREAMSYNCHRONIZER_H
#define PIXELSTREAMSYNCHRONIZER_H

#include "FpsCounter.h"
#include "TiledSynchronizer.h"

#include <QObject>

/**
 * Synchronizes a PixelStream between different QML windows.
 */
class PixelStreamSynchronizer : public TiledSynchronizer
{
    Q_OBJECT
    Q_DISABLE_COPY(PixelStreamSynchronizer)

public:
    /**
     * Construct a synchronizer for a stream.
     * @param updater The shared pixel stream data source.
     * @param view which the data source provides. Left and right views also
     *        include mono contents.
     */
    PixelStreamSynchronizer(std::shared_ptr<PixelStreamUpdater> updater,
                            deflect::View view);
    ~PixelStreamSynchronizer();

    /** @copydoc ContentSynchronizer::update */
    void update(const ContentWindow& window,
                const QRectF& visibleArea) override;

    /** @copydoc ContentSynchronizer::updateTiles */
    void updateTiles() final;

    /** @copydoc ContentSynchronizer::swapTiles */
    void swapTiles() final;

    /** @copydoc ContentSynchronizer::getTilesArea */
    QSize getTilesArea() const override;

    /** @copydoc ContentSynchronizer::getStatistics */
    QString getStatistics() const override;

    /** @copydoc ContentSynchronizer::getView */
    deflect::View getView() const final;

    /** @copydoc ContentSynchronizer::getDataSource */
    const DataSource& getDataSource() const final;

private:
    std::shared_ptr<PixelStreamUpdater> _updater;
    deflect::View _view;
    FpsCounter _fpsCounter;

    bool _tilesDirty = true;
    QSize _tilesArea;

    void _onPictureUpdated();
};

#endif

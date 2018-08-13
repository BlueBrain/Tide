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

#ifndef BASICSYNCHRONIZER_H
#define BASICSYNCHRONIZER_H

#include "synchronizers/ContentSynchronizer.h"

/**
 * A basic synchronizer used for static content types.
 */
class BasicSynchronizer : public ContentSynchronizer
{
    Q_OBJECT
    Q_DISABLE_COPY(BasicSynchronizer)

public:
    /** Constructor */
    BasicSynchronizer(DataSourceSharedPtr source);
    ~BasicSynchronizer();

    /** @copydoc ContentSynchronizer::update */
    void update(const Window& window, const QRectF& visibleArea) override;

    /** @copydoc ContentSynchronizer::updateTiles */
    void updateTiles() override;

    /** @copydoc ContentSynchronizer::canSwapTiles */
    bool canSwapTiles() const override;

    /** @copydoc ContentSynchronizer::swapTiles */
    void swapTiles() override;

    /** @copydoc ContentSynchronizer::getTilesArea */
    QSize getTilesArea() const override;

    /** @copydoc ContentSynchronizer::getStatistics */
    QString getStatistics() const override;

    /** @copydoc ContentSynchronizer::onSwapReady */
    void onSwapReady(TilePtr tile) override;

    /** @copydoc ContentSynchronizer::createZoomContextTile */
    TilePtr createZoomContextTile() const override;

    /** @copydoc ContentSynchronizer::hasVisibleTiles */
    bool hasVisibleTiles() const override;

private:
    DataSourceSharedPtr _dataSource;
    bool _tileAdded = false;
    bool _addTile = false;
    bool _zoomContextTileDirty = false;

    /** @copydoc ContentSynchronizer::getDataSource */
    const DataSource& getDataSource() const final;

    /** Create this content's unique tile. */
    void _createTile();
};

#endif

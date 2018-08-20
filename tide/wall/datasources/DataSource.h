/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include "synchronizers/ContentSynchronizers.h"
#include "types.h"

/**
 * Base interface for shared data sources.
 *
 * The DataProvider expects derived classes to always be constructible (no-throw
 * constructor) and the public functions to behave correctly even if the source
 * is unavailable for some reason (file access problem, etc...).
 */
class DataSource
{
public:
    virtual ~DataSource() = default;

    /** @return true if the data source produces dynamic contents. */
    virtual bool isDynamic() const { return false; }
    /** Update the state of the data source. */
    virtual void update(const Content& content) { Q_UNUSED(content); }
    /**
     * Get a tile image by its id for a given view.
     * Called asynchronously but not concurrently. threadsafe.
     * Implementations can return nullptr or throw an exception on error.
     */
    virtual ImagePtr getTileImage(uint tileId, deflect::View view) const = 0;

    /** @return the coordinates of a tile. */
    virtual QRect getTileRect(uint tileId) const = 0;

    /** @return the image size for the requested lod and channel. */
    virtual QSize getTilesArea(uint lod, uint channel) const = 0;

    /** @return the indices of the tiles which are visible in the given area. */
    virtual Indices computeVisibleSet(const QRectF& visibleTilesArea, uint lod,
                                      uint channel) const = 0;

    /** @return the max LOD level (top of pyramid, lowest resolution). */
    virtual uint getMaxLod() const = 0;

    /** Allow advancing to the next frame (synchronization / flow control). */
    virtual void allowNextFrame() {}
    /** Synchronize the advance to the next frame of the data. */
    virtual void synchronizeFrameAdvance(WallToWallChannel& channel)
    {
        Q_UNUSED(channel);
    }

    /** The synchronizers linked to this shared data source. */
    ContentSynchronizers synchronizers;
};

#endif

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

#include "SVGSynchronizer.h"

#include "SVGGpuImage.h"
#include "SVGTiler.h"
#include "data/SVG.h"

struct SVGSynchronizer::Impl
{
    Impl(const QString& uri)
        : svg(uri)
        , dataSource(svg)
    {
    }
    SVG svg;
    SVGTiler dataSource;
};

SVGSynchronizer::SVGSynchronizer(const QString& uri)
    : LodSynchronizer(TileSwapPolicy::SwapTilesIndependently)
    , _impl(new Impl(uri))
{
}

SVGSynchronizer::~SVGSynchronizer()
{
}

void SVGSynchronizer::synchronize(WallToWallChannel& channel)
{
    Q_UNUSED(channel);
}

ImagePtr SVGSynchronizer::getTileImage(const uint tileId) const
{
#if !(TIDE_USE_CAIRO && TIDE_USE_RSVG)
    if (!_impl->dataSource.contains(tileId))
        return std::make_shared<SVGGpuImage>(_impl->dataSource, tileId);
#endif
    return LodSynchronizer::getTileImage(tileId);
}

const DataSource& SVGSynchronizer::getDataSource() const
{
    return _impl->dataSource;
}

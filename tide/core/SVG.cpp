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

#include "SVG.h"

#if TIDE_USE_CAIRO && TIDE_USE_RSVG
#include "SVGCairoRSVGBackend.h"
#else
#include "SVGQtGpuBackend.h"
#endif
#include "log.h"

#include <QFile>

std::unique_ptr<SVGBackend> _createSvgBackend( const QByteArray& svgData )
{
#if TIDE_USE_CAIRO && TIDE_USE_RSVG
    return make_unique<SVGCairoRSVGBackend>( svgData );
#else
    return make_unique<SVGQtGpuBackend>( svgData );
#endif
}

struct SVG::Impl
{
    QString filename;
    QByteArray svgData;
    std::unique_ptr<SVGBackend> svg;
};

SVG::SVG( const QString& uri )
    : _impl( new Impl )
{
    _impl->filename = uri;
    try
    {
        QFile file( uri );
        if( !file.open( QIODevice::ReadOnly ))
            throw std::runtime_error( "invalid file" );
        _impl->svgData = file.readAll();
        _impl->svg = _createSvgBackend( _impl->svgData );
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_DEBUG, "Could not open document '%s': '%s'",
                  uri.toLocal8Bit().constData( ), e.what( ));
    }
}

SVG::SVG( const QByteArray& svgData )
    : _impl( new Impl )
{
    _impl->svgData = svgData;
    _impl->svg = _createSvgBackend( _impl->svgData );
}

SVG::~SVG() {}

const QString& SVG::getFilename() const
{
    return _impl->filename;
}

bool SVG::isValid() const
{
    return bool(_impl->svg);
}

QSize SVG::getSize() const
{
    return isValid() ? _impl->svg->getSize() : QSize();
}

const QByteArray& SVG::getData() const
{
    return _impl->svgData;
}

QImage SVG::renderToImage( const QSize& imageSize, const QRectF& region ) const
{
    return isValid() ? _impl->svg->renderToImage( imageSize, region ) : QImage();
}

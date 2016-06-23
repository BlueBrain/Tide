/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#include "ThumbnailGenerator.h"

#include <QPainter>

#define GRADIENT_START_X   0.4
#define GRADIENT_END_X     0.6
#define GRADIENT_START_Y   0
#define GRADIENT_END_Y     1

#define THUMBNAIL_FONT_SIZE 30

ThumbnailGenerator::ThumbnailGenerator( const QSize& size )
    : _size( size )
    , _aspectRatioMode( Qt::KeepAspectRatio )
{
}

QImage ThumbnailGenerator::createErrorImage( const QString& message) const
{
    QImage img = createGradientImage( Qt::red, Qt::darkRed );
    paintText( img, message );
    return img;
}

QImage ThumbnailGenerator::createGradientImage( const QColor& bgcolor1,
                                                const QColor& bgcolor2 ) const
{
    QImage img( _size, QImage::Format_RGB32 );

    QPainter painter( &img );
    const QPoint p1( GRADIENT_START_X * img.width(),
                     GRADIENT_START_Y * img.height( ));
    const QPoint p2( GRADIENT_END_X   * img.width(),
                     GRADIENT_END_Y   * img.height( ));
    QLinearGradient linearGrad( p1, p2 );
    linearGrad.setColorAt( 0, bgcolor1 );
    linearGrad.setColorAt( 1, bgcolor2 );
    painter.setBrush( linearGrad );
    painter.fillRect( 0, 0, img.width(), img.height(), QBrush( linearGrad ));
    painter.end();

    return img;
}

void ThumbnailGenerator::paintText( QImage& img, const QString& text ) const
{
    QFont font;
    font.setStyleHint( QFont::Times, QFont::PreferAntialias );
    font.setPointSize( THUMBNAIL_FONT_SIZE );

    QPainter painter( &img );
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setBrush( Qt::black );
    painter.setFont( font );
    int flags = Qt::AlignVCenter | Qt::AlignHCenter | Qt::TextWrapAnywhere;
    painter.drawText( img.rect(), flags, text );
    painter.end();
}

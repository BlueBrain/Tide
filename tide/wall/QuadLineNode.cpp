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

#include "QuadLineNode.h"

#include <QSGFlatColorMaterial>

QuadLineNode::QuadLineNode( const QRectF& rect, const qreal lineWidth )
{
    setGeometry( new QSGGeometry( QSGGeometry::defaultAttributes_Point2D(), 4));
    geometry()->setDrawingMode( GL_LINE_LOOP );
    geometry()->setLineWidth( lineWidth );
    setFlag( QSGNode::OwnsGeometry );

    setMaterial( new QSGFlatColorMaterial );
    setFlag( QSGNode::OwnsMaterial );

    setRect( rect );
}

void QuadLineNode::setRect( const QRectF& rect )
{
    QSGGeometry::Point2D* points = geometry()->vertexDataAsPoint2D();
    points[0].set( rect.left(), rect.top( ));
    points[1].set( rect.left(), rect.bottom( ));
    points[2].set( rect.right(), rect.bottom( ));
    points[3].set( rect.right(), rect.top( ));
    markDirty( DirtyGeometry );
}

void QuadLineNode::setLineWidth( const qreal width )
{
    geometry()->setLineWidth( width );
}

void QuadLineNode::setColor( const QColor& color )
{
    dynamic_cast<QSGFlatColorMaterial&>( *material( )).setColor( color );
}

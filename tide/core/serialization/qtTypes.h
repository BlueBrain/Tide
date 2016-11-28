/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#ifndef SERIALIZATION_QTTYPES_H
#define SERIALIZATION_QTTYPES_H

#include <QColor>
#include <QImage>
#include <QRectF>
#include <QString>
#include <QUuid>

#include <boost/serialization/array.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost
{
namespace serialization
{

template< class Archive >
void serialize( Archive& ar, QColor& color, const unsigned int )
{
    unsigned char t;
    t = color.red(); ar & t; color.setRed( t );
    t = color.green(); ar & t; color.setGreen( t );
    t = color.blue(); ar & t; color.setBlue( t );
    t = color.alpha(); ar & t; color.setAlpha( t );
}

template< class Archive >
void save( Archive& ar, const QString& s, const unsigned int )
{
    std::string stdStr = s.toStdString();
    ar << make_nvp( "value", stdStr );
}

template< class Archive >
void load( Archive& ar, QString& s, const unsigned int )
{
    std::string stdStr;
    ar >> make_nvp( "value", stdStr );
    s = QString::fromStdString( stdStr );
}

template< class Archive >
void serialize( Archive& ar, QString& s, const unsigned int version )
{
    split_free( ar, s, version );
}

template< class Archive >
void serialize( Archive& ar, QUuid& uuid, const unsigned int /*version*/ )
{
    ar & make_nvp( "data1", uuid.data1 );
    ar & make_nvp( "data2", uuid.data2 );
    ar & make_nvp( "data3", uuid.data3 );
    ar & make_nvp( "data40", uuid.data4[0] );
    ar & make_nvp( "data41", uuid.data4[1] );
    ar & make_nvp( "data42", uuid.data4[2] );
    ar & make_nvp( "data43", uuid.data4[3] );
    ar & make_nvp( "data44", uuid.data4[4] );
    ar & make_nvp( "data45", uuid.data4[5] );
    ar & make_nvp( "data46", uuid.data4[6] );
    ar & make_nvp( "data47", uuid.data4[7] );
}

template< class Archive >
void serialize( Archive& ar, QPointF& point, const unsigned int )
{
    ar & make_nvp( "x", point.rx( ));
    ar & make_nvp( "y", point.ry( ));
}

template< class Archive >
void serialize( Archive& ar, QSize& size, const unsigned int )
{
    ar & make_nvp( "w", size.rwidth( ));
    ar & make_nvp( "h", size.rheight( ));
}

template< class Archive >
void serialize( Archive& ar, QRectF& rect, const unsigned int )
{
    qreal t;
    t = rect.x(); ar & make_nvp( "x", t ); rect.setX( t );
    t = rect.y(); ar & make_nvp( "y", t ); rect.setY( t );
    t = rect.width(); ar & make_nvp( "w", t ); rect.setWidth( t );
    t = rect.height(); ar & make_nvp( "h", t ); rect.setHeight( t );
}

template< class Archive >
void serialize( Archive& ar, QImage& image, const unsigned int )
{
    auto size = image.size();
    ar & make_nvp( "size", size );

    auto format = image.format();
    ar & make_nvp( "format", format );

    if( Archive::is_loading::value )
        image = QImage{ size, format };

    const size_t count = image.width() * image.height() * 4;
    ar & make_nvp( "data", make_array( image.bits(), count ));
}

}
}

#endif

/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>*/
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

#ifndef SERIALIZATION_DEFLECTTYPES_H
#define SERIALIZATION_DEFLECTTYPES_H

#include <deflect/server/Frame.h>

#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost
{
namespace serialization
{
template <class Archive>
void serialize(Archive& ar, deflect::server::Frame& frame, const unsigned int)
{
    // clang-format off
    ar & frame.tiles;
    ar & frame.uri;
    // clang-format on
}

template <class Archive>
void save(Archive& ar, const deflect::server::Tile& tile, const unsigned int)
{
    // clang-format off
    ar & tile.x;
    ar & tile.y;
    ar & tile.width;
    ar & tile.height;
    ar & tile.format;
    ar & tile.rowOrder;
    ar & tile.view;
    ar & tile.channel;

    int size = tile.imageData.size();
    ar & size;

    ar & make_binary_object((void*)tile.imageData.data(),
                            tile.imageData.size());
    // clang-format on
}

template <class Archive>
void load(Archive& ar, deflect::server::Tile& tile, const unsigned int)
{
    // clang-format off
    ar & tile.x;
    ar & tile.y;
    ar & tile.width;
    ar & tile.height;
    ar & tile.format;
    ar & tile.rowOrder;
    ar & tile.view;
    ar & tile.channel;

    int size = 0;
    ar & size;
    tile.imageData.resize(size);

    ar & make_binary_object((void*)tile.imageData.data(), size);
    // clang-format on
}

template <class Archive>
void serialize(Archive& ar, deflect::server::Tile& tile,
               const unsigned int version)
{
    split_free(ar, tile, version);
}
}
}

#endif

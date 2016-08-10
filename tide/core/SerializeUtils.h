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

#ifndef SERIALIZEUTILS_H
#define SERIALIZEUTILS_H

#include "log.h"

#include <fstream>
#include <sstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/xml_archive_exception.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>

/**
 * A set of serialization utility functions.
 */
struct SerializeUtils
{
    template <typename T>
    static T binaryCopy( const T& source )
    {
        std::stringstream oss;
        {
            boost::archive::binary_oarchive oa( oss );
            oa << source;
        }
        T copy;
        {
            boost::archive::binary_iarchive ia( oss );
            ia >> copy;
        }
        return copy;
    }

    template <typename T>
    static T xmlCopy( const T& source )
    {
        std::stringstream oss;
        {
            boost::archive::xml_oarchive oa( oss );
            oa << BOOST_SERIALIZATION_NVP( source );
        }
        T copy;
        {
            boost::archive::xml_iarchive ia( oss );
            ia >> BOOST_SERIALIZATION_NVP( copy );
        }
        return copy;
    }

    template <typename T>
    static bool fromXmlFile( T& object, const std::string& filename )
    {
        std::ifstream ifs( filename );
        if( !ifs.good( ))
            return false;
        try
        {
            boost::archive::xml_iarchive ia( ifs );
            ia >> BOOST_SERIALIZATION_NVP( object );
        }
        catch( const boost::archive::archive_exception& e )
        {
            put_flog( LOG_ERROR, "Could not restore from file: '%s': %s",
                      filename.c_str(), e.what( ));
            return false;
        }
        catch( const std::exception& e )
        {
            put_flog( LOG_ERROR, "Could not restore from file '%s'',"
                                 "wrong file format: %s",
                      filename.c_str(), e.what( ));
            return false;
        }
        return true;
    }

    template <typename T>
    static bool toXmlFile( const T& object, const std::string& filename )
    {
        std::ofstream ofs( filename );
        if( !ofs.good( ))
            return false;

        boost::archive::xml_oarchive oa( ofs );
        oa << BOOST_SERIALIZATION_NVP( object );
        return true;
    }
};

#endif

/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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

#ifndef SERIALIZATION_UTILS_H
#define SERIALIZATION_UTILS_H

#include "log.h"
#include "serialization/includes.h"

#include <fstream>
#include <sstream>

/**
 * Free utility functions for serializing objects using boost::serialization.
 */
namespace serialization
{
/** Implementation details (template metaprograming). */
namespace detail
{
/** Recursive method to serialize n arguments into an archive. */
template <uint N>
struct serialize
{
    template <class Archive, typename Arg0, typename... Args>
    serialize(Archive& ar, Arg0& arg0, Args&... args)
    {
        // clang-format off
        ar & arg0;
        // clang-format on
        serialize<N - 1u>(ar, args...);
    }
};
/** End of recursion. */
template <>
struct serialize<0>
{
    template <class Archive>
    serialize(Archive&)
    {
    }
};
}

/**
 * Get an object of type T, stored in a container in binary serialized form.
 */
template <typename T, typename Container>
T get(const Container& container)
{
    std::istringstream iss{std::istringstream::binary};
#ifdef __APPLE__
    // pubsetbuf() does nothing on OSX (and probably on Windows too).
    // so an intermediate copy of the data is required.
    const std::string dataStr(container.data(), container.size());
    iss.str(dataStr);
#else
    iss.rdbuf()->pubsetbuf(const_cast<char*>(container.data()),
                           container.size());
#endif
    T object;
    {
        boost::archive::binary_iarchive ia{iss};
        ia >> object;
    }
    return object;
}

/**
 * Deserialize object(s) from a string of binary serialized data.
 */
template <typename Container, typename... Args>
void fromBinary(const Container& container, Args&... args)
{
    std::istringstream iss{std::istringstream::binary};
#ifdef __APPLE__
    // pubsetbuf() does nothing on OSX (and probably on Windows too).
    // so an intermediate copy of the data is required.
    const std::string dataStr(container.data(), container.size());
    iss.str(dataStr);
#else
    iss.rdbuf()->pubsetbuf(const_cast<char*>(container.data()),
                           container.size());
#endif
    boost::archive::binary_iarchive ia{iss};
    detail::serialize<sizeof...(Args)>(ia, args...);
}

/**
 * Serialize object(s) to a string of binary serialized data.
 */
template <typename... Args>
std::string toBinary(Args&... args)
{
    std::ostringstream stream{std::ostringstream::binary};
    {
        boost::archive::binary_oarchive oa{stream};
        detail::serialize<sizeof...(Args)>(oa, args...);
    }
    return stream.str();
}

/**
 * Copy an object using its binary serialization + deserialization methods.
 */
template <typename T>
T binaryCopy(const T& source)
{
    std::stringstream oss;
    {
        boost::archive::binary_oarchive oa(oss);
        oa << source;
    }
    T copy;
    {
        boost::archive::binary_iarchive ia(oss);
        ia >> copy;
    }
    return copy;
}

/**
 * Copy an object using its xml serialization + deserialization methods.
 */
template <typename T>
T xmlCopy(const T& source)
{
    std::stringstream oss;
    {
        boost::archive::xml_oarchive oa(oss);
        oa << BOOST_SERIALIZATION_NVP(source);
    }
    T copy;
    {
        boost::archive::xml_iarchive ia(oss);
        ia >> BOOST_SERIALIZATION_NVP(copy);
    }
    return copy;
}

/**
 * Restore an object that was previously serialized to the given xml file.
 */
template <typename T>
bool fromXmlFile(T& object, const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs.good())
        return false;
    try
    {
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(object);
    }
    catch (const boost::archive::archive_exception& e)
    {
        print_log(LOG_ERROR, LOG_CONTENT,
                  "Could not restore from file: '%s': %s", filename.c_str(),
                  e.what());
        return false;
    }
    catch (const std::exception& e)
    {
        print_log(LOG_ERROR, LOG_CONTENT,
                  "Could not restore from file '%s'',"
                  "wrong file format: %s",
                  filename.c_str(), e.what());
        return false;
    }
    return true;
}

/**
 * Store an object to a target xml file.
 */
template <typename T>
bool toXmlFile(const T& object, const std::string& filename)
{
    std::ofstream ofs(filename);
    if (!ofs.good())
        return false;

    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(object);
    return true;
}
}

#endif

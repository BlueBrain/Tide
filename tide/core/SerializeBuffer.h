/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#ifndef SERIALIZEBUFFER_H
#define SERIALIZEBUFFER_H

#include <vector>
#include <sstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>

/**
 * Utility class to ease and optimize (de)serialization of any object using
 * boost.serialization.
 */
class SerializeBuffer : boost::noncopyable
{
public:
    /**
     * Construct any empty serialization buffer
     */
    SerializeBuffer()
        : size_( 0 )
    {}

    /**
     * Set the new size of the buffer and grow the storage if necessary
     * @param minSize the minimum size required for a following (de)serialization
     */
    void setSize(const size_t minSize)
    {
        if (buffer_.size() < minSize)
            buffer_.resize(minSize);
        size_ = minSize;
    }

    /** @return the current size of the buffer */
    size_t size() const
    {
        return size_;
    }

    /** Direct write access to the buffer, don't write beyond size() */
    char* data()
    {
        return buffer_.data();
    }

    /**
     * Serialize the given object using a binary archive to a string
     * @param object the object which should be serialized
     */
    template <typename T>
    static std::string serialize(const T& object)
    {
        std::ostringstream oss(std::ostringstream::binary);
        {
            boost::archive::binary_oarchive oa(oss);
            oa << object;
        }
        return oss.str();
    }

    /**
     * Deserialize the current buffer into the given object
     * @param object the target object for deserializing the current buffer
     */
    template <typename T>
    void deserialize(T& object)
    {
        std::istringstream iss(std::istringstream::binary);
#ifdef __APPLE__
        // pubsetbuf() does nothing on OSX (and probably on Windows too).
        // so an intermediate copy of the data is required.
        std::string dataStr(buffer_.data(), buffer_.size());
        iss.str(dataStr);
#else
        iss.rdbuf()->pubsetbuf(buffer_.data(), size_);
#endif
        boost::archive::binary_iarchive ia(iss);
        ia >> object;
    }

private:
    std::vector<char> buffer_;
    size_t size_;
};

#endif // SERIALIZEBUFFER_H

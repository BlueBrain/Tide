/*********************************************************************/
/* Copyright (c) 2014-2017, EPFL/Blue Brain Project                  */
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

#ifndef RECEIVEBUFFER_H
#define RECEIVEBUFFER_H

#include <boost/noncopyable.hpp>
#include <vector>

/**
 * Utility class to optimize the reception of any type of serialized data.
 *
 * The buffer avoids memory reallocations for successive receive operations,
 * which would otherwise be expensive for large data (such as image streaming).
 *
 * The serialization::get<T>() function can be used with this buffer to directly
 * extract the expected (copyable/movable) object stored in it.
 */
class ReceiveBuffer : boost::noncopyable
{
public:
    /** Construct an empty buffer. */
    ReceiveBuffer() = default;

    /**
     * Set the new size of the buffer and grow the storage if necessary.
     * @param minSize the minimum size required for a following deserialization.
     */
    void setSize(const size_t minSize)
    {
        if (_buffer.size() < minSize)
            _buffer.resize(minSize);
        _size = minSize;
    }

    /** @return the current size of the buffer. */
    size_t size() const { return _size; }
    /** Direct write access to the buffer, don't write beyond size(). */
    char* data() { return _buffer.data(); }
    /** Direct read access to the buffer, don't read beyond size(). */
    const char* data() const { return _buffer.data(); }
private:
    std::vector<char> _buffer;
    size_t _size = 0;
};

#endif

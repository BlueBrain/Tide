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

#ifndef BOOST_SERIALIZATION_CHRONO_HPP
#define BOOST_SERIALIZATION_CHRONO_HPP

// serialization for std::chrono templates
// adapted from: http://lists.boost.org/boost-users/att-82112/chrono.hpp

#include <chrono>

#include <boost/config.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/tracking.hpp>

namespace boost
{
namespace serialization
{
/// serialization for std::chrono::duration

template <class Archive, class Rep, class Period>
inline void serialize(Archive& ar, std::chrono::duration<Rep, Period>& t,
                      const unsigned int file_version)
{
    boost::serialization::split_free(ar, t, file_version);
}

template <class Archive, class Rep, class Period>
inline void save(Archive& ar, std::chrono::duration<Rep, Period> const& t,
                 const unsigned int /* file_version */
                 )
{
    const auto count = t.count();
    ar << count;
}

template <class Archive, class Rep, class Period>
inline void load(Archive& ar, std::chrono::duration<Rep, Period>& t,
                 const unsigned int /* file_version */
                 )
{
    Rep rep;
    ar >> rep;
    t = std::chrono::duration<Rep, Period>(rep);
}

// specialization of serialization traits for std::chrono::duration
template <class Rep, class Period>
struct is_bitwise_serializable<std::chrono::duration<Rep, Period>>
    : public is_bitwise_serializable<Rep>
{
};

template <class Rep, class Period>
struct implementation_level<std::chrono::duration<Rep, Period>>
    : mpl::int_<object_serializable>
{
};

// treat just like builtin arithmetic types for tracking
template <class Rep, class Period>
struct tracking_level<std::chrono::duration<Rep, Period>>
    : mpl::int_<track_never>
{
};

/// serialization for std::chrono::time_point

template <class Archive, class C, class D> // = typename C::duration>
inline void serialize(Archive& ar, std::chrono::time_point<C, D>& t,
                      const unsigned int file_version)
{
    boost::serialization::split_free(ar, t, file_version);
}

template <class Archive, class C, class D> // = typename C::duration>
inline void save(Archive& ar, std::chrono::time_point<C, D> const& t,
                 const unsigned int /* file_version */
                 )
{
    const auto duration = t.time_since_epoch();
    ar << duration;
}

template <class Archive, class C, class D> // = typename C::duration>
inline void load(Archive& ar, std::chrono::time_point<C, D>& t,
                 const unsigned int /* file_version */
                 )
{
    D dur;
    ar >> dur;
    t = std::chrono::time_point<C, D>(dur);
}

// specialization of serialization traits for std::chrono::time_point
template <class C, class D> // = typename C::duration>
struct is_bitwise_serializable<std::chrono::time_point<C, D>>
    : public is_bitwise_serializable<D>
{
};

template <class C, class D> // = typename C::duration>
struct implementation_level<std::chrono::time_point<C, D>>
    : mpl::int_<object_serializable>
{
};

// treat just like builtin arithmetic types for tracking
template <class C, class D> // = typename C::duration>
struct tracking_level<std::chrono::time_point<C, D>> : mpl::int_<track_never>
{
};
}
}

#endif

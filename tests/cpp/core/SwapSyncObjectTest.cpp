/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur<daniel.nachbaur@epfl.ch>      */
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


#define BOOST_TEST_MODULE SwapSyncObjectTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "SwapSyncObject.h"

#include <boost/bind.hpp>


BOOST_AUTO_TEST_CASE( testSwapSyncObjectDefaultConstruction )
{
    typedef boost::shared_ptr< int > IntPtr;

    const SwapSyncObject< IntPtr > syncObject;
    BOOST_CHECK_EQUAL( syncObject.get(), IntPtr( ));
}

BOOST_AUTO_TEST_CASE( testSwapSyncObjectValueConstruction )
{
    typedef boost::shared_ptr< int > IntPtr;
    IntPtr ptr( new int );
    *ptr = 5;

    const SwapSyncObject< IntPtr > syncObject( ptr );
    BOOST_CHECK_EQUAL( syncObject.get(), ptr );
}

namespace
{
bool oddNumberSync( const uint64_t version )
{
    return version % 2 != 0;
}

bool alwaysSync( const uint64_t )
{
    return true;
}

template< typename T >
struct CallbackResult
{
    void callMe( T object )
    {
        result = object;
    }
    T getResult() const
    {
        return result;
    }

private:
    T result;
};
}

BOOST_AUTO_TEST_CASE( testUpdateAndSyncOddNumber )
{
    typedef boost::shared_ptr< int > IntPtr;
    IntPtr ptr( new int );
    *ptr = 5;

    SwapSyncObject< IntPtr > syncObject( ptr );

    IntPtr otherPtr( new int );
    *otherPtr = 42;
    syncObject.update( otherPtr );
    BOOST_CHECK_EQUAL( syncObject.get(), ptr );

    BOOST_CHECK( syncObject.sync( boost::bind( &oddNumberSync, _1 )));
    BOOST_CHECK( !syncObject.sync( boost::bind( &oddNumberSync, _1 )));
    BOOST_CHECK_EQUAL( syncObject.get(), otherPtr );

    *ptr = 12345;
    syncObject.update( ptr );
    BOOST_CHECK_EQUAL( syncObject.get(), otherPtr );

    BOOST_CHECK( !syncObject.sync( boost::bind( &oddNumberSync, _1 )));
    BOOST_CHECK_EQUAL( syncObject.get(), otherPtr );
}

BOOST_AUTO_TEST_CASE( testUpdateAndSyncAlways )
{
    typedef boost::shared_ptr< int > IntPtr;
    IntPtr ptr( new int );
    *ptr = 5;

    SwapSyncObject< IntPtr > syncObject( ptr );

    IntPtr otherPtr( new int );
    *otherPtr = 42;
    syncObject.update( otherPtr );
    BOOST_CHECK_EQUAL( syncObject.get(), ptr );

    BOOST_CHECK( syncObject.sync( boost::bind( &alwaysSync, _1 )));
    BOOST_CHECK( !syncObject.sync( boost::bind( &alwaysSync, _1 )));
    BOOST_CHECK_EQUAL( syncObject.get(), otherPtr );

    *ptr = 12345;
    syncObject.update( ptr );
    BOOST_CHECK_EQUAL( syncObject.get(), otherPtr );

    BOOST_CHECK( syncObject.sync( boost::bind( &alwaysSync, _1 )));
    BOOST_CHECK( !syncObject.sync( boost::bind( &alwaysSync, _1 )));
    BOOST_CHECK_EQUAL( syncObject.get(), ptr );
}

BOOST_AUTO_TEST_CASE( testUpdateAndSyncAlwaysWithCallback )
{
    typedef boost::shared_ptr< int > IntPtr;
    IntPtr ptr( new int );
    *ptr = 5;

    typedef CallbackResult< IntPtr > CallbackResultT;
    CallbackResultT result;
    SwapSyncObject< IntPtr > syncObject;
    syncObject.setCallback( boost::bind( &CallbackResultT::callMe, boost::ref(result), _1 ));
    syncObject.update( ptr );
    BOOST_REQUIRE( syncObject.sync( boost::bind( &alwaysSync, _1 )));
    BOOST_CHECK_EQUAL( *result.getResult(), *ptr );
}

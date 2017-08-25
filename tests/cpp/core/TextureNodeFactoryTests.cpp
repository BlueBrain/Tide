/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE TextureNodeFactoryTests

#include <boost/test/unit_test.hpp>

#include "TextureNodeFactory.h"
#include "TextureNodeRGBA.h"
#include "TextureNodeYUV.h"

#include "QGuiAppFixture.h"

BOOST_FIXTURE_TEST_CASE(need_to_change_texture_node_type_between_rgba_and_yuv,
                        QQuickWindowFixture)
{
    if (!window)
        return;

    TextureNodeFactoryImpl f{*window, TextureType::Static};
    using TF = TextureFormat;

    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::rgba, TF::rgba), false);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::rgba, TF::yuv420), true);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::rgba, TF::yuv422), true);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::rgba, TF::yuv444), true);

    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv420, TF::rgba), true);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv420, TF::yuv420), false);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv420, TF::yuv422), false);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv420, TF::yuv444), false);

    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv422, TF::rgba), true);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv422, TF::yuv420), false);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv422, TF::yuv422), false);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv422, TF::yuv444), false);

    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv444, TF::rgba), true);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv444, TF::yuv420), false);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv444, TF::yuv422), false);
    BOOST_CHECK_EQUAL(f.needToChangeNodeType(TF::yuv444, TF::yuv444), false);
}

BOOST_FIXTURE_TEST_CASE(create_static_texture_nodes, QQuickWindowFixture)
{
    if (!window)
        return;

    TextureNodeFactoryImpl f{*window, TextureType::Static};
    using TF = TextureFormat;

    BOOST_CHECK(dynamic_cast<TextureNodeRGBA*>(f.create(TF::rgba).get()));
    BOOST_CHECK(dynamic_cast<TextureNodeYUV*>(f.create(TF::yuv420).get()));
    BOOST_CHECK(dynamic_cast<TextureNodeYUV*>(f.create(TF::yuv422).get()));
    BOOST_CHECK(dynamic_cast<TextureNodeYUV*>(f.create(TF::yuv444).get()));
}

BOOST_FIXTURE_TEST_CASE(create_dynamic_texture_nodes, QQuickWindowFixture)
{
    if (!window)
        return;

    TextureNodeFactoryImpl f{*window, TextureType::Dynamic};
    using TF = TextureFormat;

    BOOST_CHECK(dynamic_cast<TextureNodeRGBA*>(f.create(TF::rgba).get()));
    BOOST_CHECK(dynamic_cast<TextureNodeYUV*>(f.create(TF::yuv420).get()));
    BOOST_CHECK(dynamic_cast<TextureNodeYUV*>(f.create(TF::yuv422).get()));
    BOOST_CHECK(dynamic_cast<TextureNodeYUV*>(f.create(TF::yuv444).get()));
}

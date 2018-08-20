/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE DataSourceTests

#include <boost/test/unit_test.hpp>

#include "datasources/DataSource.h"
#include "datasources/DataSourceFactory.h"

#include "DummyContent.h"

namespace
{
const QSize contentSize(800, 600);
const QRectF contentArea(QRectF(QPointF(), contentSize));
const std::vector<ContentType> contentTypes{ContentType::movie,
                                            ContentType::pixel_stream,
                                            ContentType::svg,
                                            ContentType::image,
                                            ContentType::pdf,
                                            ContentType::webbrowser,
                                            ContentType::image_pyramid};
const std::vector<ContentType> unsupportedContentTypes{
    ContentType::invalid, ContentType::dynamic_texture};

ContentPtr make_dummy_content(const ContentType type)
{
    auto content = std::make_unique<DummyContent>(contentSize, "/not/a/file");
    content->type = type;
    return content;
}
}

BOOST_AUTO_TEST_CASE(datasources_handle_files_access_problems_gracefully)
{
    for (const auto& type : contentTypes)
    {
        auto content = make_dummy_content(type);
        try
        {
            auto datasource = DataSourceFactory::create(*content);

            BOOST_CHECK(datasource->getTileRect(0).isEmpty());
            BOOST_CHECK(datasource->getTilesArea(0, 0).isEmpty());
            BOOST_CHECK_EQUAL(datasource->getMaxLod(), 0);
            BOOST_CHECK(
                datasource->computeVisibleSet(contentArea, 0, 0).empty());
            BOOST_CHECK(!datasource->getTileImage(0, deflect::View::mono));
        }
        catch (...)
        {
            BOOST_CHECK(false);
        }
    }
}

BOOST_AUTO_TEST_CASE(datasource_factory_throws_for_unsupported_contents)
{
    for (const auto& type : unsupportedContentTypes)
    {
        auto content = make_dummy_content(type);
        BOOST_CHECK_THROW(DataSourceFactory::create(*content),
                          std::logic_error);
    }
}

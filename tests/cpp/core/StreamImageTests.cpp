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

#define BOOST_TEST_MODULE StreamImageTests
#include <boost/test/unit_test.hpp>

#include "StreamImage.h"

#include <deflect/Frame.h>
#include <deflect/SegmentDecoder.h>

namespace
{
const std::vector<uint8_t> expectedRGBA(8 * 8 * 4, 17);
const std::vector<uint8_t> expectedY(8 * 8, 92);
const std::vector<uint8_t> expectedU(8 * 8, 28);
const std::vector<uint8_t> expectedV(8 * 8, 79);
}

deflect::FramePtr createRgbaTestFrame(const QSize& size)
{
    deflect::FramePtr frame(new deflect::Frame);
    deflect::Segment segment;

    segment.parameters.x = 10;
    segment.parameters.y = -20;
    segment.parameters.width = size.width();
    segment.parameters.height = size.height();

    segment.parameters.dataType = deflect::DataType::rgba;
    segment.imageData.append(QByteArray(size.width() * size.height() * 4, 17));

    frame->segments.push_back(segment);
    return frame;
}

deflect::FramePtr createYuvTestFrame(const QSize& size, const int subsamp)
{
    deflect::FramePtr frame(new deflect::Frame);
    deflect::Segment segment;

    segment.parameters.x = 10;
    segment.parameters.y = -20;
    segment.parameters.width = size.width();
    segment.parameters.height = size.height();

    segment.parameters.dataType = deflect::DataType::yuv444;
    if (subsamp == 1)
        segment.parameters.dataType = deflect::DataType::yuv422;
    else if (subsamp == 2)
        segment.parameters.dataType = deflect::DataType::yuv420;

    const auto ySize = size.width() * size.height();
    const auto uvSize = ySize >> subsamp;
    segment.imageData.append(QByteArray(ySize, 92));  // Y
    segment.imageData.append(QByteArray(uvSize, 28)); // U
    segment.imageData.append(QByteArray(uvSize, 79)); // V

    frame->segments.push_back(segment);
    return frame;
}

BOOST_AUTO_TEST_CASE(testStreamImageRGBA)
{
    StreamImage image(createRgbaTestFrame({8, 8}), 0);

    BOOST_CHECK_EQUAL(image.getPosition(), QPoint(10, -20));
    BOOST_CHECK_EQUAL(image.getWidth(), 8);
    BOOST_CHECK_EQUAL(image.getHeight(), 8);

    const auto data = image.getData(0);
    const auto size = image.getDataSize(0);

    BOOST_CHECK(image.getColorSpace() == ColorSpace::undefined);
    BOOST_CHECK(image.getRowOrder() == deflect::RowOrder::top_down);

    BOOST_CHECK_EQUAL(size, 8 * 8 * 4);
    BOOST_CHECK_EQUAL_COLLECTIONS(data, data + size, expectedRGBA.data(),
                                  expectedRGBA.data() + size);
    BOOST_CHECK_EQUAL(image.getTextureSize(0), QSize(8, 8));
}

BOOST_AUTO_TEST_CASE(testStreamImageYUV)
{
    for (int subsamp = 0; subsamp <= 2; ++subsamp)
    {
        StreamImage image(createYuvTestFrame({8, 8}, subsamp), 0);

        const auto y = image.getData(0);
        const auto u = image.getData(1);
        const auto v = image.getData(2);
        const auto imageSizeY = image.getTextureSize(0);
        const auto imageSizeU = image.getTextureSize(1);
        const auto imageSizeV = image.getTextureSize(2);
        const auto ySize = imageSizeY.width() * imageSizeY.height();
        const auto uSize = imageSizeU.width() * imageSizeU.height();
        const auto vSize = imageSizeV.width() * imageSizeV.height();

        BOOST_CHECK_EQUAL(image.getPosition(), QPoint(10, -20));
        BOOST_CHECK_EQUAL(image.getWidth(), 8);
        BOOST_CHECK_EQUAL(image.getHeight(), 8);

        BOOST_CHECK(image.getColorSpace() == ColorSpace::yCbCrJpeg);
        BOOST_CHECK(image.getRowOrder() == deflect::RowOrder::top_down);

        BOOST_CHECK_EQUAL(image.getDataSize(0), 8 * 8);
        BOOST_CHECK_EQUAL(image.getDataSize(1), 8 * 8 >> subsamp);
        BOOST_CHECK_EQUAL(image.getDataSize(2), 8 * 8 >> subsamp);

        BOOST_CHECK_EQUAL(ySize, 8 * 8);
        BOOST_CHECK_EQUAL(uSize, 8 * 8 >> subsamp);
        BOOST_CHECK_EQUAL(vSize, 8 * 8 >> subsamp);

        BOOST_CHECK_EQUAL_COLLECTIONS(y, y + ySize, expectedY.data(),
                                      expectedY.data() + ySize);
        BOOST_CHECK_EQUAL_COLLECTIONS(u, u + uSize, expectedU.data(),
                                      expectedU.data() + uSize);
        BOOST_CHECK_EQUAL_COLLECTIONS(v, v + vSize, expectedV.data(),
                                      expectedV.data() + vSize);
    }
}

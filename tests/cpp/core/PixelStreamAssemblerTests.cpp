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

#define BOOST_TEST_MODULE PixelStreamAssemblerTests
#include <boost/test/unit_test.hpp>

#include "tide/core/data/Image.h"
#include "tide/wall/PixelStreamAssembler.h"

#include <deflect/Frame.h>
#include <deflect/SegmentDecoder.h>

#include <cmath> //std::ceil

namespace
{
const int SEGMENT_SIZE = 64;

QByteArray createBuffer(const int size, const int seed)
{
    QByteArray buffer;
    assert(size % 8 == 0);
    for (int i = 0; i < size; i += 8)
    {
        buffer.append(seed + 0);
        buffer.append(seed + 1);
        buffer.append(seed + 2);
        buffer.append(seed + 3);
        buffer.append(seed + 4);
        buffer.append(seed + 5);
        buffer.append(seed + 6);
        buffer.append(seed + 7);
    }
    return buffer;
}

const QByteArray expectedY = createBuffer(512 * 512, 92);
const QByteArray expectedU = createBuffer(512 * 512, 28);
const QByteArray expectedV = createBuffer(512 * 512, 79);
const QByteArray expectedRGBA = createBuffer(512 * 512 * 4, 112);

deflect::DataType getDataType(const int subsamp)
{
    switch (subsamp)
    {
    case -1:
        return deflect::DataType::rgba;
    case 0:
        return deflect::DataType::yuv444;
    case 1:
        return deflect::DataType::yuv422;
    case 2:
        return deflect::DataType::yuv420;
    default:
        throw std::runtime_error("Invalid subsampling");
    }
}

TextureFormat getTextureFormat(const int subsamp)
{
    switch (subsamp)
    {
    case -1:
        return TextureFormat::rgba;
    case 0:
        return TextureFormat::yuv444;
    case 1:
        return TextureFormat::yuv422;
    case 2:
        return TextureFormat::yuv420;
    default:
        throw std::runtime_error("Invalid subsampling");
    }
}

deflect::Segment createSegment(const QPoint& pos, const QSize& size,
                               const int subsamp)
{
    deflect::Segment segment;
    segment.parameters.dataType = getDataType(subsamp);
    segment.parameters.x = pos.x();
    segment.parameters.y = pos.y();
    segment.parameters.width = size.width();
    segment.parameters.height = size.height();

    if (subsamp == -1)
    {
        const auto rgbaSize = size.width() * size.height() * 4;
        segment.imageData.append(createBuffer(rgbaSize, 112)); // RGBA
    }
    else
    {
        const auto ySize = size.width() * size.height();
        const auto uvSize = ySize >> subsamp;
        segment.imageData.append(createBuffer(ySize, 92));  // Y
        segment.imageData.append(createBuffer(uvSize, 28)); // U
        segment.imageData.append(createBuffer(uvSize, 79)); // V
    }
    return segment;
}

deflect::FramePtr createTestFrame(const QSize& size, const int subsamp)
{
    deflect::FramePtr frame(new deflect::Frame);

    const int segmentsX = std::ceil(float(size.width()) / SEGMENT_SIZE);
    const int segmentsY = std::ceil(float(size.height()) / SEGMENT_SIZE);

    auto borderX = size.width() % SEGMENT_SIZE;
    if (borderX == 0)
        borderX = SEGMENT_SIZE;

    auto borderY = size.height() % SEGMENT_SIZE;
    if (borderY == 0)
        borderY = SEGMENT_SIZE;

    for (int y = 0; y < segmentsY; ++y)
    {
        for (int x = 0; x < segmentsX; ++x)
        {
            const QPoint pos{x * SEGMENT_SIZE, y * SEGMENT_SIZE};
            const QSize segSize{x < segmentsX - 1 ? SEGMENT_SIZE : borderX,
                                y < segmentsY - 1 ? SEGMENT_SIZE : borderY};
            frame->segments.push_back(createSegment(pos, segSize, subsamp));
        }
    }
    return frame;
}

inline void checkData(const Image& image, const int subsamp)
{
    if (subsamp == -1) // RGBA
    {
        const auto size = image.getWidth() * image.getHeight() * 4;
        const auto data = image.getData(0);
        BOOST_CHECK_EQUAL_COLLECTIONS(data, data + size, expectedRGBA.data(),
                                      expectedRGBA.data() + size);
    }
    else // YUV
    {
        const auto sizeY = image.getWidth() * image.getHeight();
        const auto sizeUV = sizeY / (1 << subsamp);
        const auto dataY = image.getData(0);
        const auto dataU = image.getData(1);
        const auto dataV = image.getData(2);

        BOOST_CHECK_EQUAL_COLLECTIONS(dataY, dataY + sizeY, expectedY.data(),
                                      expectedY.data() + sizeY);
        BOOST_CHECK_EQUAL_COLLECTIONS(dataU, dataU + sizeUV, expectedU.data(),
                                      expectedU.data() + sizeUV);
        BOOST_CHECK_EQUAL_COLLECTIONS(dataV, dataV + sizeUV, expectedV.data(),
                                      expectedV.data() + sizeUV);
    }
}
}

BOOST_AUTO_TEST_CASE(testAssembleStreamImageYUVandRGBA)
{
#ifndef DEFLECT_USE_LEGACY_LIBJPEGTURBO
    for (auto subsamp = -1; subsamp <= 2; ++subsamp)
#else
    const int subsamp = -1; // Only RGBA can be tested
#endif
    {
        const auto frame = createTestFrame({640, 900}, subsamp);
        const auto textureFormat = getTextureFormat(subsamp);

        BOOST_REQUIRE_EQUAL(frame->segments.size(), 10 * 15);
        BOOST_REQUIRE_EQUAL(frame->computeDimensions(), QSize(640, 900));
        BOOST_REQUIRE_NO_THROW(PixelStreamAssembler{frame});

        // Check indices
        PixelStreamAssembler assembler{frame};
        auto indices = assembler.computeVisibleSet(QRectF{0, 0, 0, 0});
        BOOST_CHECK_EQUAL(indices.size(), 0);
        indices = assembler.computeVisibleSet(QRectF{64, 64, 576, 512 - 64});
        BOOST_CHECK_EQUAL(indices.size(), 2);
        indices = assembler.computeVisibleSet(QRectF{0, 0, 640, 900});
        BOOST_CHECK_EQUAL(indices.size(), 4);

        // Check format
        deflect::SegmentDecoder decoder;
        for (auto tileIndex : indices)
        {
            BOOST_CHECK_EQUAL((int)assembler.getTileFormat(tileIndex, decoder),
                              (int)textureFormat);
        }

        // Check rectangles
        BOOST_CHECK_EQUAL(assembler.getTileRect(0), QRect(0, 0, 512, 512));
        BOOST_CHECK_EQUAL(assembler.getTileRect(1),
                          QRect(512, 0, 640 - 512, 512));
        BOOST_CHECK_EQUAL(assembler.getTileRect(2),
                          QRect(0, 512, 512, 900 - 512));
        BOOST_CHECK_EQUAL(assembler.getTileRect(3),
                          QRect(512, 512, 640 - 512, 900 - 512));

        // Check images
        const auto image0 = assembler.getTileImage(0, decoder);
        BOOST_CHECK_EQUAL(image0->getWidth(), 512);
        BOOST_CHECK_EQUAL(image0->getHeight(), 512);
        BOOST_CHECK_EQUAL((int)image0->getFormat(), (int)textureFormat);
        checkData(*image0, subsamp);

        const auto image1 = assembler.getTileImage(1, decoder);
        BOOST_CHECK_EQUAL(image1->getWidth(), 640 - 512);
        BOOST_CHECK_EQUAL(image1->getHeight(), 512);
        BOOST_CHECK_EQUAL((int)image1->getFormat(), (int)textureFormat);
        checkData(*image1, subsamp);

        const auto image2 = assembler.getTileImage(2, decoder);
        BOOST_CHECK_EQUAL(image2->getWidth(), 512);
        BOOST_CHECK_EQUAL(image2->getHeight(), 900 - 512);
        BOOST_CHECK_EQUAL((int)image2->getFormat(), (int)textureFormat);
        checkData(*image2, subsamp);

        const auto image3 = assembler.getTileImage(3, decoder);
        BOOST_CHECK_EQUAL(image3->getWidth(), 640 - 512);
        BOOST_CHECK_EQUAL(image3->getHeight(), 900 - 512);
        BOOST_CHECK_EQUAL((int)image3->getFormat(), (int)textureFormat);
        checkData(*image3, subsamp);
    }
}

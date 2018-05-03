/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE PixelStreamAssemblerTests
#include <boost/test/unit_test.hpp>

#include "tide/core/data/Image.h"
#include "tide/wall/PixelStreamAssembler.h"

#include <deflect/server/Frame.h>
#include <deflect/server/TileDecoder.h>

#include <cmath> //std::ceil

namespace
{
const int TILE_SIZE = 64;
const int IMAGE_WIDTH = 642;
const int IMAGE_HEIGHT = 914;

deflect::Format getFormat(const int subsamp)
{
    switch (subsamp)
    {
    case -1:
        return deflect::Format::rgba;
    case 0:
        return deflect::Format::yuv444;
    case 1:
        return deflect::Format::yuv422;
    case 2:
        return deflect::Format::yuv420;
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

QSize getUVImageSize(const QSize& ySize, const int subsamp)
{
    switch (subsamp)
    {
    case 0:
        return ySize;
    case 1:
        return {ySize.width() >> 1, ySize.height()};
    case 2:
        return ySize / 2;
    default:
        throw std::runtime_error("Invalid subsampling");
    }
}

QPoint getUVImagePos(const QPoint& yPos, const int subsamp)
{
    switch (subsamp)
    {
    case 0:
        return yPos;
    case 1:
        return {yPos.x() >> 1, yPos.y()};
    case 2:
        return yPos / 2;
    default:
        throw std::runtime_error("Invalid subsampling");
    }
}

deflect::RowOrder getRowOrder(const int orientation)
{
    switch (orientation)
    {
    case 0:
        return deflect::RowOrder::top_down;
    case 1:
        return deflect::RowOrder::bottom_up;
    default:
        throw std::runtime_error("Invalid orientation");
    }
}

QByteArray createBuffer(const QSize& size, const int bpp, const int seed)
{
    QByteArray buffer;
    for (int y = 0; y < size.height(); ++y)
    {
        for (int x = 0; x < size.width(); ++x)
        {
            for (int p = 0; p < bpp; ++p)
                buffer.append(char((seed + y + x) % 256));
        }
    }
    assert(buffer.size() == size.width() * size.height() * bpp);
    return buffer;
}

struct TestImage
{
    TestImage(const int subsamp_)
        : subsamp(subsamp_)
    {
        if (subsamp == -1)
            rgba = createBuffer({IMAGE_WIDTH, IMAGE_HEIGHT}, 4, 'r');
        else
        {
            const auto ySize = QSize{IMAGE_WIDTH, IMAGE_HEIGHT};
            uvSize = getUVImageSize(ySize, subsamp);
            y = createBuffer(ySize, 1, 'y');
            u = createBuffer(uvSize, 1, 'u');
            v = createBuffer(uvSize, 1, 'v');
        }
    }

    const int subsamp;
    QByteArray rgba;
    QByteArray y;
    QByteArray u;
    QByteArray v;
    QSize uvSize;
};

QByteArray copyRegion(const QByteArray& src, const int srcWidth,
                      const QRect& region, const int bpp)
{
    QByteArray copy;
    const auto in = src.data();
    const auto srcStride = srcWidth * bpp;
    const auto dstStride = region.width() * bpp;

    for (int y = region.y(); y < region.y() + region.height(); ++y)
    {
        const auto index = y * srcStride + region.x() * bpp;
        copy.append(&in[index], dstStride);
        assert(&in[index] < in + src.size());
    }

    assert(copy.size() == region.height() * region.width() * bpp);
    return copy;
}

void flipRowOrder(QByteArray& buffer, const QSize& size, const int bpp)
{
    QByteArray copy;
    const auto in = buffer.data();
    const auto stride = size.width() * bpp;
    for (int y = size.height() - 1; y >= 0; --y)
        copy.append(&in[y * stride], stride);
    buffer = copy;
}

deflect::server::Tile createTile(const QRect& region,
                                 const TestImage& testImage,
                                 const deflect::RowOrder rowOrder)
{
    deflect::server::Tile tile;
    tile.format = getFormat(testImage.subsamp);
    tile.x = region.x();
    tile.y = region.y();
    tile.width = region.width();
    tile.height = region.height();
    tile.rowOrder = rowOrder;

    if (testImage.subsamp == -1)
    {
        tile.imageData = copyRegion(testImage.rgba, IMAGE_WIDTH, region, 4);
        if (rowOrder == deflect::RowOrder::bottom_up)
            flipRowOrder(tile.imageData, region.size(), 4);
    }
    else
    {
        const auto uvSize = getUVImageSize(region.size(), testImage.subsamp);
        const auto uvPos = getUVImagePos(region.topLeft(), testImage.subsamp);
        const auto uvRegion = QRect{uvPos, uvSize};
        auto yBuffer = copyRegion(testImage.y, IMAGE_WIDTH, region, 1);
        auto uBuffer =
            copyRegion(testImage.u, testImage.uvSize.width(), uvRegion, 1);
        auto vBuffer =
            copyRegion(testImage.v, testImage.uvSize.width(), uvRegion, 1);

        if (rowOrder == deflect::RowOrder::bottom_up)
        {
            flipRowOrder(yBuffer, region.size(), 1);
            flipRowOrder(uBuffer, uvSize, 1);
            flipRowOrder(vBuffer, uvSize, 1);
        }
        tile.imageData.append(yBuffer);
        tile.imageData.append(uBuffer);
        tile.imageData.append(vBuffer);
    }

    return tile;
}

deflect::server::FramePtr createTestFrame(const QSize& size,
                                          const TestImage& testImage,
                                          const deflect::RowOrder rowOrder)
{
    deflect::server::FramePtr frame(new deflect::server::Frame);

    const int tilesX = std::ceil(float(size.width()) / TILE_SIZE);
    const int tilesY = std::ceil(float(size.height()) / TILE_SIZE);

    auto borderX = size.width() % TILE_SIZE;
    if (borderX == 0)
        borderX = TILE_SIZE;

    auto borderY = size.height() % TILE_SIZE;
    if (borderY == 0)
        borderY = TILE_SIZE;

    for (int y = 0; y < tilesY; ++y)
    {
        for (int x = 0; x < tilesX; ++x)
        {
            const QPoint pos{x * TILE_SIZE, y * TILE_SIZE};
            const QSize segSize{x < tilesX - 1 ? TILE_SIZE : borderX,
                                y < tilesY - 1 ? TILE_SIZE : borderY};
            const QRect region{pos, segSize};
            frame->tiles.push_back(createTile(region, testImage, rowOrder));
        }
    }
    return frame;
}

void checkRGBAData(const Image& image, const QRect& region,
                   const TestImage& testImage)
{
    const auto size = image.getWidth() * image.getHeight() * 4;
    const auto data = image.getData(0);
    const auto expectedData =
        copyRegion(testImage.rgba, IMAGE_WIDTH, region, 4);

    BOOST_CHECK_EQUAL(size, expectedData.size());
    // ptr type need to be identical otherwise boost shows false errors
    const auto ptr = reinterpret_cast<const uint8_t*>(expectedData.data());
    BOOST_CHECK_EQUAL_COLLECTIONS(data, data + size, ptr, ptr + size);
}

void checkYUVData(const Image& image, const QRect& region,
                  const TestImage& testImage)
{
    const auto sizeY = image.getWidth() * image.getHeight();
    const auto sizeUV = sizeY / (1 << testImage.subsamp);
    const auto dataY = image.getData(0);
    const auto dataU = image.getData(1);
    const auto dataV = image.getData(2);

    const auto uvSize = getUVImageSize(region.size(), testImage.subsamp);
    const auto uvPos = getUVImagePos(region.topLeft(), testImage.subsamp);
    const auto uvRegion = QRect{uvPos, uvSize};

    const auto expectedDataY = copyRegion(testImage.y, IMAGE_WIDTH, region, 1);
    const auto expectedDataU =
        copyRegion(testImage.u, testImage.uvSize.width(), uvRegion, 1);
    const auto expectedDataV =
        copyRegion(testImage.v, testImage.uvSize.width(), uvRegion, 1);

    BOOST_CHECK_EQUAL(sizeY, expectedDataY.size());
    BOOST_CHECK_EQUAL(sizeUV, expectedDataU.size());
    BOOST_CHECK_EQUAL(sizeUV, expectedDataV.size());

    // ptr type need to be identical otherwise boost shows false errors
    const auto pY = reinterpret_cast<const uint8_t*>(expectedDataY.data());
    const auto pU = reinterpret_cast<const uint8_t*>(expectedDataU.data());
    const auto pV = reinterpret_cast<const uint8_t*>(expectedDataV.data());

    BOOST_CHECK_EQUAL_COLLECTIONS(dataY, dataY + sizeY, pY, pY + sizeY);
    BOOST_CHECK_EQUAL_COLLECTIONS(dataU, dataU + sizeUV, pU, pU + sizeUV);
    BOOST_CHECK_EQUAL_COLLECTIONS(dataV, dataV + sizeUV, pV, pV + sizeUV);
}

void checkData(const Image& image, const QRect& region,
               const TestImage& testImage)
{
    if (testImage.subsamp == -1)
        checkRGBAData(image, region, testImage);
    else
        checkYUVData(image, region, testImage);
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
        const TestImage image(subsamp);

        for (auto orientation = 0; orientation <= 1; ++orientation)
        {
            const auto rowOrder = getRowOrder(orientation);
            const auto frame =
                createTestFrame({IMAGE_WIDTH, IMAGE_HEIGHT}, image, rowOrder);
            const auto textureFormat = getTextureFormat(subsamp);

            BOOST_REQUIRE_EQUAL(frame->tiles.size(), 11 * 15);
            BOOST_REQUIRE_EQUAL(frame->computeDimensions(),
                                QSize(IMAGE_WIDTH, IMAGE_HEIGHT));
            BOOST_REQUIRE_NO_THROW(PixelStreamAssembler{frame});

            // Check indices
            PixelStreamAssembler assembler{frame};
            auto indices = assembler.computeVisibleSet(QRectF{0, 0, 0, 0});
            BOOST_CHECK_EQUAL(indices.size(), 0);
            indices =
                assembler.computeVisibleSet(QRectF{64, 64, 576, 512 - 64});
            BOOST_CHECK_EQUAL(indices.size(), 2);
            indices = assembler.computeVisibleSet(
                QRectF{0, 0, IMAGE_WIDTH, IMAGE_HEIGHT});
            BOOST_CHECK_EQUAL(indices.size(), 4);

            // Check rectangles
            const auto rect0 = assembler.getTileRect(0);
            const auto rect1 = assembler.getTileRect(1);
            const auto rect2 = assembler.getTileRect(2);
            const auto rect3 = assembler.getTileRect(3);
            BOOST_CHECK_EQUAL(rect0, QRect(0, 0, 512, 512));
            BOOST_CHECK_EQUAL(rect1, QRect(512, 0, IMAGE_WIDTH - 512, 512));
            BOOST_CHECK_EQUAL(rect2, QRect(0, 512, 512, IMAGE_HEIGHT - 512));
            BOOST_CHECK_EQUAL(rect3, QRect(512, 512, IMAGE_WIDTH - 512,
                                           IMAGE_HEIGHT - 512));

            // Check images
            deflect::server::TileDecoder decoder;
            const auto image0 = assembler.getTileImage(0, decoder);
            BOOST_CHECK_EQUAL(image0->getWidth(), 512);
            BOOST_CHECK_EQUAL(image0->getHeight(), 512);
            BOOST_CHECK_EQUAL((int)image0->getFormat(), (int)textureFormat);
            checkData(*image0, rect0, image);

            const auto image1 = assembler.getTileImage(1, decoder);
            BOOST_CHECK_EQUAL(image1->getWidth(), IMAGE_WIDTH - 512);
            BOOST_CHECK_EQUAL(image1->getHeight(), 512);
            BOOST_CHECK_EQUAL((int)image1->getFormat(), (int)textureFormat);
            checkData(*image1, rect1, image);

            const auto image2 = assembler.getTileImage(2, decoder);
            BOOST_CHECK_EQUAL(image2->getWidth(), 512);
            BOOST_CHECK_EQUAL(image2->getHeight(), IMAGE_HEIGHT - 512);
            BOOST_CHECK_EQUAL((int)image2->getFormat(), (int)textureFormat);
            checkData(*image2, rect2, image);

            const auto image3 = assembler.getTileImage(3, decoder);
            BOOST_CHECK_EQUAL(image3->getWidth(), IMAGE_WIDTH - 512);
            BOOST_CHECK_EQUAL(image3->getHeight(), IMAGE_HEIGHT - 512);
            BOOST_CHECK_EQUAL((int)image3->getFormat(), (int)textureFormat);
            checkData(*image3, rect3, image);
        }
    }
}

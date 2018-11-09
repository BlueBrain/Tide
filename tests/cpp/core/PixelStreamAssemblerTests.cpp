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

#define BOOST_TEST_MODULE PixelStreamAssemblerTests
#include <boost/test/unit_test.hpp>

#include "tide/core/data/Image.h"
#include "tide/wall/tools/PixelStreamAssembler.h"

#include <deflect/server/Frame.h>
#include <deflect/server/TileDecoder.h>

#include <cmath> //std::ceil

#ifndef DEFLECT_USE_LEGACY_LIBJPEGTURBO
#define SUBSAMP_LOOP for (auto subsamp = -1; subsamp <= 2; ++subsamp)
#else
#define SUBSAMP_LOOP const int subsamp = -1; // Only RGBA can be tested
#endif

namespace
{
const int TILE_SIZE = 64;
const int IMAGE_WIDTH = 644;
const int IMAGE_HEIGHT = 916;
const QSize REF_IMAGE_SIZE = {IMAGE_WIDTH, IMAGE_HEIGHT};
const int TILES_CHANNEL_0 = 11 * 15;
const int TILES_CHANNEL_1 = 6 * 8;
const int TILES_CHANNEL_2 = 11 * 15;

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
    TestImage(const QSize& size_, const int subsamp_)
        : size{size_}
        , subsamp(subsamp_)
    {
        if (subsamp == -1)
            rgba = createBuffer(size, 4, 'r');
        else
        {
            uvSize = getUVImageSize(size, subsamp);
            y = createBuffer(size, 1, 'y');
            u = createBuffer(uvSize, 1, 'u');
            v = createBuffer(uvSize, 1, 'v');
        }
    }

    QSize size;
    const int subsamp;
    QByteArray rgba;
    QByteArray y;
    QByteArray u;
    QByteArray v;
    QSize uvSize;
};

size_t expectedTilesCount(const TestImage& image)
{
    const auto tilesX = int(std::ceil(float(image.size.width()) / 512));
    const auto tilesY = int(std::ceil(float(image.size.height()) / 512));
    return tilesX * tilesY;
}

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
        tile.imageData =
            copyRegion(testImage.rgba, testImage.size.width(), region, 4);
        if (rowOrder == deflect::RowOrder::bottom_up)
            flipRowOrder(tile.imageData, region.size(), 4);
    }
    else
    {
        const auto uvSize = getUVImageSize(region.size(), testImage.subsamp);
        const auto uvPos = getUVImagePos(region.topLeft(), testImage.subsamp);
        const auto uvRegion = QRect{uvPos, uvSize};
        auto yBuffer =
            copyRegion(testImage.y, testImage.size.width(), region, 1);
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

deflect::server::FramePtr createTestFrame(const TestImage& testImage,
                                          const deflect::RowOrder rowOrder,
                                          const int tileSize = TILE_SIZE)
{
    deflect::server::FramePtr frame(new deflect::server::Frame);

    const auto& size = testImage.size;

    const int tilesX = std::ceil(float(size.width()) / tileSize);
    const int tilesY = std::ceil(float(size.height()) / tileSize);

    auto borderX = size.width() % tileSize;
    if (borderX == 0)
        borderX = tileSize;

    auto borderY = size.height() % tileSize;
    if (borderY == 0)
        borderY = tileSize;

    for (int y = 0; y < tilesY; ++y)
    {
        for (int x = 0; x < tilesX; ++x)
        {
            const QPoint pos{x * tileSize, y * tileSize};
            const QSize segSize{x < tilesX - 1 ? tileSize : borderX,
                                y < tilesY - 1 ? tileSize : borderY};
            const QRect region{pos, segSize};
            frame->tiles.push_back(createTile(region, testImage, rowOrder));
        }
    }
    return frame;
}

deflect::server::FramePtr createTestFrame(const TestImage& testImage0,
                                          const TestImage& testImage1,
                                          const TestImage& testImage2,
                                          const deflect::RowOrder rowOrder)
{
    auto frame = createTestFrame(testImage0, rowOrder);

    auto channel1 = createTestFrame(testImage1, rowOrder);
    for (auto& tile : channel1->tiles)
    {
        tile.channel = 1;
        frame->tiles.push_back(tile);
    }

    auto channel2 = createTestFrame(testImage2, rowOrder);
    for (auto& tile : channel2->tiles)
    {
        tile.channel = 2;
        frame->tiles.push_back(tile);
    }

    return frame;
}

void checkRGBAData(const Image& image, const QRect& region,
                   const TestImage& testImage)
{
    const auto size = image.getWidth() * image.getHeight() * 4;
    const auto data = image.getData(0);
    const auto expectedData =
        copyRegion(testImage.rgba, testImage.size.width(), region, 4);

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

    const auto expectedDataY =
        copyRegion(testImage.y, testImage.size.width(), region, 1);
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

void checkIndices(PixelStreamProcessor& processor, const TestImage& image,
                  const uint channel)
{
    auto indices = processor.computeVisibleSet(QRectF{0, 0, 0, 0}, channel);
    BOOST_CHECK_EQUAL(indices.size(), 0);
    indices =
        processor.computeVisibleSet(QRectF{64, 64, 576, 512 - 64}, channel);
    BOOST_CHECK_EQUAL(indices.size(), image.size.width() > 512 ? 2 : 1);

    indices = processor.computeVisibleSet(QRectF{{0, 0}, image.size}, channel);
    BOOST_CHECK_EQUAL(indices.size(), expectedTilesCount(image));
}

void checkRefImageTiles(PixelStreamProcessor& processor, const TestImage& image,
                        const int subsamp, const uint tileIndexOffset = 0)
{
    const auto rect0 = processor.getTileRect(0 + tileIndexOffset);
    const auto rect1 = processor.getTileRect(1 + tileIndexOffset);
    const auto rect2 = processor.getTileRect(2 + tileIndexOffset);
    const auto rect3 = processor.getTileRect(3 + tileIndexOffset);
    BOOST_CHECK_EQUAL(rect0, QRect(0, 0, 512, 512));
    BOOST_CHECK_EQUAL(rect1, QRect(512, 0, IMAGE_WIDTH - 512, 512));
    BOOST_CHECK_EQUAL(rect2, QRect(0, 512, 512, IMAGE_HEIGHT - 512));
    BOOST_CHECK_EQUAL(rect3,
                      QRect(512, 512, IMAGE_WIDTH - 512, IMAGE_HEIGHT - 512));

    const auto textureFormat = getTextureFormat(subsamp);

    deflect::server::TileDecoder decoder;
    const auto image0 = processor.getTileImage(0 + tileIndexOffset, decoder);
    BOOST_CHECK_EQUAL(image0->getWidth(), 512);
    BOOST_CHECK_EQUAL(image0->getHeight(), 512);
    BOOST_CHECK_EQUAL((int)image0->getFormat(), (int)textureFormat);
    checkData(*image0, rect0, image);

    const auto image1 = processor.getTileImage(1 + tileIndexOffset, decoder);
    BOOST_CHECK_EQUAL(image1->getWidth(), IMAGE_WIDTH - 512);
    BOOST_CHECK_EQUAL(image1->getHeight(), 512);
    BOOST_CHECK_EQUAL((int)image1->getFormat(), (int)textureFormat);
    checkData(*image1, rect1, image);

    const auto image2 = processor.getTileImage(2 + tileIndexOffset, decoder);
    BOOST_CHECK_EQUAL(image2->getWidth(), 512);
    BOOST_CHECK_EQUAL(image2->getHeight(), IMAGE_HEIGHT - 512);
    BOOST_CHECK_EQUAL((int)image2->getFormat(), (int)textureFormat);
    checkData(*image2, rect2, image);

    const auto image3 = processor.getTileImage(3 + tileIndexOffset, decoder);
    BOOST_CHECK_EQUAL(image3->getWidth(), IMAGE_WIDTH - 512);
    BOOST_CHECK_EQUAL(image3->getHeight(), IMAGE_HEIGHT - 512);
    BOOST_CHECK_EQUAL((int)image3->getFormat(), (int)textureFormat);
    checkData(*image3, rect3, image);
}

void checkHalfRefImageTiles(PixelStreamProcessor& processor,
                            const TestImage& image, const int subsamp,
                            const uint tileIndexOffset = 0)
{
    const auto rect0 = processor.getTileRect(0 + tileIndexOffset);
    BOOST_CHECK_EQUAL(rect0, QRect(0, 0, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2));

    const auto textureFormat = getTextureFormat(subsamp);

    deflect::server::TileDecoder decoder;
    const auto image0 = processor.getTileImage(0 + tileIndexOffset, decoder);
    BOOST_CHECK_EQUAL(image0->getWidth(), IMAGE_WIDTH / 2);
    BOOST_CHECK_EQUAL(image0->getHeight(), IMAGE_HEIGHT / 2);
    BOOST_CHECK_EQUAL((int)image0->getFormat(), (int)textureFormat);
    checkData(*image0, rect0, image);
}

void checkConstructorThrows(deflect::server::FramePtr frame)
{
    BOOST_CHECK_THROW(PixelStreamChannelAssembler(frame, 0),
                      std::runtime_error);
    BOOST_CHECK_THROW(PixelStreamAssembler{frame}, std::runtime_error);
}
void checkConstructorDoesNotThrow(deflect::server::FramePtr frame)
{
    BOOST_CHECK_NO_THROW(PixelStreamChannelAssembler(frame, 0));
    BOOST_CHECK_NO_THROW(PixelStreamAssembler{frame});
}
}

BOOST_AUTO_TEST_CASE(testAssembleSingleChannelStreamImageYUVandRGBA)
{
    SUBSAMP_LOOP
    {
        const auto image = TestImage(REF_IMAGE_SIZE, subsamp);

        for (auto orientation = 0; orientation <= 1; ++orientation)
        {
            const auto rowOrder = getRowOrder(orientation);
            const auto frame = createTestFrame(image, rowOrder);

            BOOST_REQUIRE_EQUAL(frame->tiles.size(), TILES_CHANNEL_0);
            BOOST_REQUIRE_EQUAL(frame->computeDimensions(), REF_IMAGE_SIZE);

            BOOST_REQUIRE_NO_THROW(PixelStreamChannelAssembler(frame, 0));
            PixelStreamChannelAssembler assembler{frame, 0};
            checkIndices(assembler, image, 0);
            checkRefImageTiles(assembler, image, subsamp);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAssembleMultiChannelStreamImageYUVandRGBA)
{
    SUBSAMP_LOOP
    {
        const auto image0 = TestImage(REF_IMAGE_SIZE, subsamp);
        const auto image1 = TestImage(REF_IMAGE_SIZE / 2, subsamp);
        const auto image2 = TestImage(REF_IMAGE_SIZE, subsamp);

        for (auto orientation = 0; orientation <= 1; ++orientation)
        {
            const auto rowOrder = getRowOrder(orientation);
            const auto frame =
                createTestFrame(image0, image1, image2, rowOrder);

            const auto totalTiles =
                TILES_CHANNEL_0 + TILES_CHANNEL_1 + TILES_CHANNEL_2;
            BOOST_REQUIRE_EQUAL(frame->tiles.size(), totalTiles);
            BOOST_REQUIRE_EQUAL(frame->computeDimensions(), REF_IMAGE_SIZE);

            BOOST_REQUIRE_NO_THROW(PixelStreamAssembler{frame});
            PixelStreamAssembler assembler{frame};
            checkIndices(assembler, image0, 0);
            checkIndices(assembler, image1, 1);
            checkIndices(assembler, image2, 2);
            checkRefImageTiles(assembler, image0, subsamp);
            auto tileIndexOffset = expectedTilesCount(image0);
            checkHalfRefImageTiles(assembler, image1, subsamp, tileIndexOffset);
            tileIndexOffset += expectedTilesCount(image1);
            checkRefImageTiles(assembler, image2, subsamp, tileIndexOffset);
        }
    }
}

BOOST_AUTO_TEST_CASE(testConstructorForImageTooSmallThrows)
{
    SUBSAMP_LOOP
    {
        const auto imageTooSmall = TestImage({54, 36}, subsamp);
        const auto image = TestImage(REF_IMAGE_SIZE, subsamp);

        for (auto orientation = 0; orientation <= 1; ++orientation)
        {
            const auto rowOrder = getRowOrder(orientation);
            checkConstructorThrows(createTestFrame(imageTooSmall, rowOrder));
        }
    }
}

BOOST_AUTO_TEST_CASE(testTileSizeIsDivisorOf512)
{
    SUBSAMP_LOOP
    {
        const auto image = TestImage(REF_IMAGE_SIZE, subsamp);

        for (auto orientation = 0; orientation <= 1; ++orientation)
        {
            const auto rowOrder = getRowOrder(orientation);
            checkConstructorDoesNotThrow(createTestFrame(image, rowOrder, 16));
            checkConstructorDoesNotThrow(createTestFrame(image, rowOrder, 32));
            checkConstructorDoesNotThrow(createTestFrame(image, rowOrder, 128));
            checkConstructorDoesNotThrow(createTestFrame(image, rowOrder, 256));
            checkConstructorThrows(createTestFrame(image, rowOrder, 66));
            checkConstructorThrows(createTestFrame(image, rowOrder, 137));
            checkConstructorThrows(createTestFrame(image, rowOrder, 512));
            checkConstructorThrows(createTestFrame(image, rowOrder, 1024));
        }
    }
}

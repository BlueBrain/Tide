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

#define BOOST_TEST_MODULE TextureSwitcherTests

#include <boost/test/unit_test.hpp>

#include "TextureSwitcher.h"
#include "data/Image.h"

class MockTextureNode : public TextureNode
{
public:
    explicit MockTextureNode(const TextureFormat f)
        : format{f}
    {
    }
    virtual QRectF getCoord() const { return coord; }
    virtual void setCoord(const QRectF& rect) { coord = rect; }
    virtual void uploadTexture(const Image& im) { image = &im; }
    virtual void swap() { swapped = true; }
    TextureFormat format;
    QRectF coord;
    const Image* image = nullptr;
    bool swapped = false;
};

class MockTextureNodeFactory : public TextureNodeFactory
{
public:
    std::unique_ptr<TextureNode> create(TextureFormat format) final
    {
        return std::make_unique<MockTextureNode>(format);
    }
    bool needToChangeNodeType(TextureFormat a, TextureFormat b) const final
    {
        return a != b;
    }
};

class MockImage : public Image
{
public:
    MockImage() = default;
    MockImage(TextureFormat format)
        : _format{format}
    {
    }
    int getWidth() const { return 0; }
    int getHeight() const { return 0; }
    const uint8_t* getData(uint) const { return nullptr; }
    TextureFormat getFormat() const { return _format; }
    uint getGLPixelFormat() const { return 0; }
private:
    TextureFormat _format = TextureFormat::rgba;
};

class MockTextureSwitcher : public TextureSwitcher
{
public:
    void aboutToSwitch(TextureNode&, TextureNode&) final
    {
        switchedTextureNodes = true;
    }
    bool switchedTextureNodes = false;
};

inline std::ostream& operator<<(std::ostream& str, const TextureFormat t)
{
    switch (t)
    {
    case TextureFormat::rgba:
        str << "rgba";
        break;
    case TextureFormat::yuv420:
        str << "yuv420";
        break;
    case TextureFormat::yuv422:
        str << "yuv422";
        break;
    case TextureFormat::yuv444:
        str << "yuv444";
        break;
    default:
        str << "INVALID";
        break;
    }
    return str;
}

struct Fixture
{
    MockTextureNodeFactory factory;
    MockTextureSwitcher switcher;
    std::unique_ptr<TextureNode> textureNode;
};

BOOST_FIXTURE_TEST_CASE(update_null_texture_with_no_image_returns_null, Fixture)
{
    switcher.update(textureNode, factory);
    BOOST_CHECK(textureNode == nullptr);
}

BOOST_FIXTURE_TEST_CASE(update_null_texture_with_image_returns_texture, Fixture)
{
    auto image = std::make_shared<MockImage>(TextureFormat::yuv422);
    switcher.setNextImage(image);

    switcher.update(textureNode, factory);
    BOOST_REQUIRE(textureNode != nullptr);

    auto node = dynamic_cast<MockTextureNode*>(textureNode.get());
    BOOST_REQUIRE(node != nullptr);
    BOOST_CHECK_EQUAL(node->format, image->getFormat());

    BOOST_CHECK(!switcher.switchedTextureNodes);
}

BOOST_FIXTURE_TEST_CASE(updating_texture_multiple_times_returns_input, Fixture)
{
    auto image = std::make_shared<MockImage>(TextureFormat::yuv422);
    switcher.setNextImage(image);

    switcher.update(textureNode, factory);
    BOOST_REQUIRE(textureNode != nullptr);

    const auto firstTextureNode = textureNode.get();
    for (int i = 0; i < 10; ++i)
    {
        switcher.update(textureNode, factory);
        BOOST_CHECK_EQUAL(textureNode.get(), firstTextureNode);
    }
    BOOST_CHECK(!switcher.switchedTextureNodes);
}

struct ImagesFixture : public Fixture
{
    std::vector<ImagePtr> images;
    ImagesFixture()
    {
        for (int i = 0; i < 10; ++i)
            images.emplace_back(std::make_shared<MockImage>());
    }

    void updateSwitcher(ImagePtr image)
    {
        switcher.setNextImage(image);
        switcher.requestSwap();
        switcher.update(textureNode, factory);
    }
};

BOOST_FIXTURE_TEST_CASE(swap_without_new_images_does_not_work, ImagesFixture)
{
    updateSwitcher(images[0]);
    switcher.update(textureNode, factory);
    auto& node = dynamic_cast<MockTextureNode&>(*textureNode);
    BOOST_REQUIRE(node.swapped);
    BOOST_REQUIRE_EQUAL(node.image, images[0].get());
    node.swapped = false;

    for (int i = 0; i < 10; ++i)
    {
        switcher.requestSwap();
        switcher.update(textureNode, factory);

        node = dynamic_cast<MockTextureNode&>(*textureNode);
        BOOST_CHECK_EQUAL(node.image, images[0].get());
        BOOST_CHECK(!node.swapped);
    }
    BOOST_CHECK(!switcher.switchedTextureNodes);
}

BOOST_FIXTURE_TEST_CASE(update_image_multiple_times_without_swap, ImagesFixture)
{
    for (int i = 0; i < 10; ++i)
    {
        switcher.setNextImage(images[i]);
        switcher.update(textureNode, factory);

        auto& node = dynamic_cast<MockTextureNode&>(*textureNode);
        BOOST_CHECK_EQUAL(node.image, images[i].get());
        BOOST_CHECK(!node.swapped);
    }
    BOOST_CHECK(!switcher.switchedTextureNodes);
}

BOOST_FIXTURE_TEST_CASE(update_and_swap_texture_multiple_times, ImagesFixture)
{
    for (int i = 0; i < 10; ++i)
    {
        updateSwitcher(images[i]);
        auto& node = dynamic_cast<MockTextureNode&>(*textureNode);

        BOOST_CHECK_EQUAL(node.image, images[i].get());
        BOOST_CHECK(node.swapped || i == 0);
        node.swapped = false;
    }
    BOOST_CHECK(!switcher.switchedTextureNodes);
}

BOOST_FIXTURE_TEST_CASE(switch_texture_type_once, ImagesFixture)
{
    for (int i = 0; i < 5; ++i)
        images[i] = std::make_shared<MockImage>(TextureFormat::yuv444);

    for (int i = 0; i < 10; ++i)
    {
        updateSwitcher(images[i]);
        auto& node = dynamic_cast<MockTextureNode&>(*textureNode);

        // When switching texture type, the image is uploaded to an internal
        // texture node and the previous texture node is swapped + returned.
        // The new image is "seen" here after the next swap (one frame delay).
        BOOST_CHECK_EQUAL(node.image, images[i].get());
        BOOST_CHECK_EQUAL(node.format, images[i]->getFormat());
        BOOST_CHECK(node.swapped || i == 0);
        node.swapped = false;

        BOOST_CHECK(!switcher.switchedTextureNodes || i == 0 || i == 5);
        switcher.switchedTextureNodes = false;
    }
}

BOOST_FIXTURE_TEST_CASE(switch_texture_type_twice, ImagesFixture)
{
    for (int i = 0; i < 5; ++i)
        images[i] = std::make_shared<MockImage>(TextureFormat::yuv444);
    images[5] = std::make_shared<MockImage>(TextureFormat::yuv420);

    for (int i = 0; i < 10; ++i)
    {
        updateSwitcher(images[i]);
        auto& node = dynamic_cast<MockTextureNode&>(*textureNode);

        BOOST_CHECK_EQUAL(node.image, images[i].get());
        BOOST_CHECK_EQUAL(node.format, images[i]->getFormat());
        BOOST_CHECK(node.swapped || i == 0);
        node.swapped = false;

        BOOST_CHECK(!switcher.switchedTextureNodes || i == 0 || i == 5 ||
                    i == 6);
        switcher.switchedTextureNodes = false;
    }
}

BOOST_FIXTURE_TEST_CASE(switch_texture_type_repeatedly, ImagesFixture)
{
    for (int i = 0; i < 10; ++i)
        images[i] = std::make_shared<MockImage>(TextureFormat(i % 4));

    for (int i = 0; i < 10; ++i)
    {
        updateSwitcher(images[i]);
        auto& node = dynamic_cast<MockTextureNode&>(*textureNode);

        BOOST_CHECK_EQUAL(node.image, images[i].get());
        BOOST_CHECK_EQUAL(node.format, images[i]->getFormat());
        BOOST_CHECK(node.swapped || i == 0);
        node.swapped = false;

        BOOST_CHECK(switcher.switchedTextureNodes || i == 0);
        switcher.switchedTextureNodes = false;
    }
}

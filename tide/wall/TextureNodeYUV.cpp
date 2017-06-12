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

#include "TextureNodeYUV.h"

#include "data/Image.h"
#include "textureUtils.h"
#include "yuv.h"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QSGSimpleMaterialShader>
#include <QSGTexture>

namespace
{
const char* vertShader =
    R"(
#version 120
uniform highp mat4 qt_Matrix;
attribute highp vec4 aVertex;
attribute highp vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    gl_Position = qt_Matrix * aVertex;
    vTexCoord = aTexCoord;
}
)";

const char* fragShader =
    R"(
#version 120
uniform lowp float qt_Opacity;
uniform lowp sampler2D y_tex;
uniform lowp sampler2D u_tex;
uniform lowp sampler2D v_tex;
varying vec2 vTexCoord;
const vec3 offset = vec3(-0.0625, -0.5, -0.5);
const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);
const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);
const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);
void main() {
  float y = texture2D(y_tex, vTexCoord).r;
  float u = texture2D(u_tex, vTexCoord).r;
  float v = texture2D(v_tex, vTexCoord).r;
  vec3 yuv = vec3(y, u, v);
  yuv += offset;
  float r = dot(yuv, R_cf);
  float g = dot(yuv, G_cf);
  float b = dot(yuv, B_cf);
  gl_FragColor = vec4(r, g, b, qt_Opacity);
}
)";
}

/**
 * The state of the QSGSimpleMaterialShader.
 */
struct YUVState
{
    std::unique_ptr<QSGTexture> textureY;
    std::unique_ptr<QSGTexture> textureU;
    std::unique_ptr<QSGTexture> textureV;
    TextureFormat textureFormat;

    std::unique_ptr<QOpenGLBuffer> frontPboY;
    std::unique_ptr<QOpenGLBuffer> frontPboU;
    std::unique_ptr<QOpenGLBuffer> frontPboV;

    std::unique_ptr<QOpenGLBuffer> backPboY;
    std::unique_ptr<QOpenGLBuffer> backPboU;
    std::unique_ptr<QOpenGLBuffer> backPboV;
};

/**
 * Material to render a YUV texture with OpenGL in a QSGNode.
 */
class YUVShader : public QSGSimpleMaterialShader<YUVState>
{
    QSG_DECLARE_SIMPLE_SHADER(YUVShader, YUVState)

public:
    QList<QByteArray> attributes() const final
    {
        return QList<QByteArray>() << "aVertex"
                                   << "aTexCoord";
    }

    const char* vertexShader() const final { return vertShader; }
    const char* fragmentShader() const final { return fragShader; }
    void updateState(const YUVState* newState, const YUVState*) final
    {
        auto gl = QOpenGLContext::currentContext()->functions();
        // We bind the textures in inverse order so that we leave the
        // updateState function with GL_TEXTURE0 as the active texture unit.
        // This is maintain the "contract" that updateState should not mess up
        // the GL state beyond what is needed for this material.
        gl->glActiveTexture(GL_TEXTURE2);
        newState->textureV->bind();

        gl->glActiveTexture(GL_TEXTURE1);
        newState->textureU->bind();

        gl->glActiveTexture(GL_TEXTURE0);
        newState->textureY->bind();
    }

    void resolveUniforms() final
    {
        program()->setUniformValue("y_tex", 0); // GL_TEXTURE0
        program()->setUniformValue("u_tex", 1); // GL_TEXTURE1
        program()->setUniformValue("v_tex", 2); // GL_TEXTURE2
    }
};

YUVState* _getMaterialState(QSGGeometryNode& node)
{
    using YUVShaderMaterial = QSGSimpleMaterial<YUVState>;
    return static_cast<YUVShaderMaterial*>(node.material())->state();
}

const YUVState* _getMaterialState(const QSGGeometryNode& node)
{
    using YUVShaderMaterial = QSGSimpleMaterial<YUVState>;
    return static_cast<const YUVShaderMaterial*>(node.material())->state();
}

TextureNodeYUV::TextureNodeYUV(QQuickWindow* window, bool dynamic)
    : _window(window)
    , _dynamicTexture(dynamic)
{
    // Set up geometry, actual vertices will be initialized in updatePaintNode
    const auto& attr = QSGGeometry::defaultAttributes_TexturedPoint2D();
    _node.setGeometry(new QSGGeometry(attr, 4));
    _node.setFlag(QSGNode::OwnsGeometry);

    _node.setMaterial(YUVShader::createMaterial());
    _node.setFlag(QSGNode::OwnsMaterial);

    auto state = _getMaterialState(_node);
    state->textureY.reset(_window->createTextureFromId(0, QSize(1, 1)));
    state->textureU.reset(_window->createTextureFromId(0, QSize(1, 1)));
    state->textureV.reset(_window->createTextureFromId(0, QSize(1, 1)));

    appendChildNode(&_node);
}

const QRectF& TextureNodeYUV::rect() const
{
    return _rect;
}

void TextureNodeYUV::setRect(const QRectF& rect_)
{
    if (_rect == rect_)
        return;

    _rect = rect_;
    QSGGeometry::updateTexturedRectGeometry(_node.geometry(), _rect,
                                            UNIT_RECTF);
    _node.markDirty(QSGNode::DirtyGeometry);
}

void TextureNodeYUV::updateBackTexture(const Image& image)
{
    if (!image.getTextureSize().isValid())
        throw std::runtime_error("image texture has invalid size");
    if (image.getGLPixelFormat() != GL_RED)
        throw std::runtime_error("TextureNodeYUV image format must be GL_RED");

    auto state = _getMaterialState(_node);
    if (!state->backPboY)
        _createBackPbos();

    _uploadToBackPbos(image);

    _nextTextureSize = image.getTextureSize();
    _nextFormat = image.getFormat();
}

void TextureNodeYUV::swap()
{
    if (_needTextureChange())
        _createTextures(_nextTextureSize, _nextFormat);

    _swapPbos();
    _copyFrontPbosToTextures();
    markDirty(DirtyMaterial);

    if (!_dynamicTexture)
        _deletePbos();
}

bool TextureNodeYUV::_needTextureChange() const
{
    auto state = _getMaterialState(_node);
    return state->textureY->textureSize() != _nextTextureSize ||
           state->textureFormat != _nextFormat;
}

void TextureNodeYUV::_createTextures(const QSize& size,
                                     const TextureFormat format)
{
    auto state = _getMaterialState(_node);
    const auto uvSize = yuv::getUVSize(size, format);
    state->textureY = _createTexture(size);
    state->textureU = _createTexture(uvSize);
    state->textureV = _createTexture(uvSize);
    state->textureFormat = format;
}

std::unique_ptr<QSGTexture> TextureNodeYUV::_createTexture(
    const QSize& size) const
{
    auto texture = textureUtils::createTexture(size, *_window);
    texture->setFiltering(QSGTexture::Linear);
    texture->setMipmapFiltering(QSGTexture::Linear);
    return texture;
}

void TextureNodeYUV::_createBackPbos()
{
    auto state = _getMaterialState(_node);
    state->backPboY = textureUtils::createPbo(_dynamicTexture);
    state->backPboU = textureUtils::createPbo(_dynamicTexture);
    state->backPboV = textureUtils::createPbo(_dynamicTexture);
}

void TextureNodeYUV::_deletePbos()
{
    auto state = _getMaterialState(_node);
    state->frontPboY.reset();
    state->frontPboU.reset();
    state->frontPboV.reset();
    state->backPboY.reset();
    state->backPboU.reset();
    state->backPboV.reset();
}

void TextureNodeYUV::_uploadToBackPbos(const Image& image)
{
    auto state = _getMaterialState(_node);
    textureUtils::upload(image, 0, *state->backPboY);
    textureUtils::upload(image, 1, *state->backPboU);
    textureUtils::upload(image, 2, *state->backPboV);
}

void TextureNodeYUV::_copyFrontPbosToTextures()
{
    auto state = _getMaterialState(_node);
    textureUtils::copy(*state->frontPboY, *state->textureY, GL_RED);
    textureUtils::copy(*state->frontPboU, *state->textureU, GL_RED);
    textureUtils::copy(*state->frontPboV, *state->textureV, GL_RED);
}

void TextureNodeYUV::_swapPbos()
{
    auto state = _getMaterialState(_node);
    std::swap(state->frontPboY, state->backPboY);
    std::swap(state->frontPboU, state->backPboU);
    std::swap(state->frontPboV, state->backPboV);
}

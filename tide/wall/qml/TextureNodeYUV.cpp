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

#include "TextureNodeYUV.h"

#include "data/Image.h"
#include "textureUtils.h"
#include "utils/yuv.h"

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
uniform lowp int color_space;
uniform float tex_offset_x;
uniform float tex_offset_y;
uniform float tex_scale_x;
uniform float tex_scale_y;
uniform lowp bool reverse_orientation;
varying vec2 vTexCoord;
// https://en.wikipedia.org/wiki/YCbCr JPEG conversion
const vec3 R_cf_jpeg = vec3(1.0,  0.0,  1.402);
const vec3 offset_jpeg = vec3(0.0, -0.5, -0.5);
const vec3 G_cf_jpeg = vec3(1.0, -0.344136, -0.714136);
const vec3 B_cf_jpeg = vec3(1.0,  1.772,  0.0);
// https://en.wikipedia.org/wiki/YCbCr ITU-R BT.601 conversion
const vec3 offset_video = vec3(-0.0625, -0.5, -0.5);
const vec3 R_cf_video = vec3(1.164383,  0.000000,  1.596027);
const vec3 G_cf_video = vec3(1.164383, -0.391762, -0.812968);
const vec3 B_cf_video = vec3(1.164383,  2.017232,  0.000000);
void main() {
  vec2 texCoord = vTexCoord;
  if(reverse_orientation)
     texCoord.y = 1.0 - texCoord.y;
  texCoord.x = (texCoord.x - tex_offset_x) * tex_scale_x;
  texCoord.y = (texCoord.y - tex_offset_y) * tex_scale_y;
  float y = texture2D(y_tex, texCoord).r;
  float u = texture2D(u_tex, texCoord).r;
  float v = texture2D(v_tex, texCoord).r;
  vec3 yuv = vec3(y, u, v);
  if (color_space == 1) {
    yuv += offset_jpeg;
    float r = dot(yuv, R_cf_jpeg);
    float g = dot(yuv, G_cf_jpeg);
    float b = dot(yuv, B_cf_jpeg);
    gl_FragColor = vec4(r, g, b, qt_Opacity);
  } else if (color_space == 2) {
    yuv += offset_video;
    float r = dot(yuv, R_cf_video);
    float g = dot(yuv, G_cf_video);
    float b = dot(yuv, B_cf_video);
    gl_FragColor = vec4(r, g, b, qt_Opacity);
  } else {
    gl_FragColor = vec4(0, 0.0, 1.0, qt_Opacity);
  }
}
)";
} // namespace

/**
 * The state of the QSGSimpleMaterialShader.
 */
struct YUVState
{
    std::unique_ptr<QSGTexture> textureY;
    std::unique_ptr<QSGTexture> textureU;
    std::unique_ptr<QSGTexture> textureV;
    TextureFormat textureFormat;
    bool reverseOrientation = false;
    ColorSpace colorSpace = ColorSpace::undefined;

    std::unique_ptr<QOpenGLBuffer> pboY;
    std::unique_ptr<QOpenGLBuffer> pboU;
    std::unique_ptr<QOpenGLBuffer> pboV;

    float texOffsetX = 0.f;
    float texOffsetY = 0.f;
    float texScaleX = 1.f;
    float texScaleY = 1.f;
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

        program()->setUniformValue("tex_offset_x", newState->texOffsetX);
        program()->setUniformValue("tex_offset_y", newState->texOffsetY);
        program()->setUniformValue("tex_scale_x", newState->texScaleX);
        program()->setUniformValue("tex_scale_y", newState->texScaleY);

        program()->setUniformValue("color_space", (int)newState->colorSpace);
        program()->setUniformValue("reverse_orientation",
                                   (int)newState->reverseOrientation);
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

TextureNodeYUV::TextureNodeYUV(QQuickWindow& window, const bool dynamic)
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
    state->textureY.reset(_window.createTextureFromId(0, QSize(1, 1)));
    state->textureU.reset(_window.createTextureFromId(0, QSize(1, 1)));
    state->textureV.reset(_window.createTextureFromId(0, QSize(1, 1)));

    appendChildNode(&_node);
}

QRectF TextureNodeYUV::getCoord() const
{
    return _rect;
}

void TextureNodeYUV::setCoord(const QRectF& rect)
{
    if (_rect == rect)
        return;

    _rect = rect;
    QSGGeometry::updateTexturedRectGeometry(_node.geometry(), _rect,
                                            UNIT_RECTF);
    _node.markDirty(QSGNode::DirtyGeometry);
}

void TextureNodeYUV::uploadTexture(const Image& image)
{
    if (!image.getTextureSize().isValid())
        throw std::runtime_error("image texture has invalid size");
    if (image.getGLPixelFormat() != GL_RED)
        throw std::runtime_error("TextureNodeYUV image format must be GL_RED");

    auto state = _getMaterialState(_node);
    if (!state->pboY)
        _createPbos();

    _uploadToPbos(image);

    _nextTextureSize = image.getTextureSize();
    _nextFormat = image.getFormat();

    {
        // Calculate viewport clipping
        const auto viewPort = image.getViewPort();
        const double imageWidth = image.getWidth();
        const double imageHeight = image.getHeight();

        state->texOffsetX = viewPort.x() / imageWidth;
        state->texOffsetY = viewPort.y() / imageHeight;
        state->texScaleX = viewPort.width() / imageWidth;
        state->texScaleY = viewPort.height() / imageHeight;
    }

    state->reverseOrientation =
        image.getRowOrder() == deflect::RowOrder::bottom_up;
    state->colorSpace = image.getColorSpace();
}

void TextureNodeYUV::swap()
{
    if (_needTextureChange())
        _createTextures(_nextTextureSize, _nextFormat);

    _copyPbosToTextures();
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
    auto texture = textureUtils::createTexture(size, _window);
    texture->setFiltering(QSGTexture::Linear);
    texture->setMipmapFiltering(QSGTexture::Linear);
    return texture;
}

void TextureNodeYUV::_createPbos()
{
    auto state = _getMaterialState(_node);
    state->pboY = textureUtils::createPbo(_dynamicTexture);
    state->pboU = textureUtils::createPbo(_dynamicTexture);
    state->pboV = textureUtils::createPbo(_dynamicTexture);
}

void TextureNodeYUV::_deletePbos()
{
    auto state = _getMaterialState(_node);
    state->pboY.reset();
    state->pboU.reset();
    state->pboV.reset();
}

void TextureNodeYUV::_uploadToPbos(const Image& image)
{
    auto state = _getMaterialState(_node);
    textureUtils::upload(image, 0, *state->pboY);
    textureUtils::upload(image, 1, *state->pboU);
    textureUtils::upload(image, 2, *state->pboV);
}

void TextureNodeYUV::_copyPbosToTextures()
{
    auto state = _getMaterialState(_node);
    textureUtils::copy(*state->pboY, *state->textureY, GL_RED);
    textureUtils::copy(*state->pboU, *state->textureU, GL_RED);
    textureUtils::copy(*state->pboV, *state->textureV, GL_RED);
}

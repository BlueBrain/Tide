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

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QSGSimpleMaterialShader>

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
    std::unique_ptr<QSGTexture> frontY;
    std::unique_ptr<QSGTexture> frontU;
    std::unique_ptr<QSGTexture> frontV;
    TextureFormat frontFormat;

    std::unique_ptr<QSGTexture> backY;
    std::unique_ptr<QSGTexture> backU;
    std::unique_ptr<QSGTexture> backV;
    TextureFormat backFormat;
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
        return QList<QByteArray>() << "aVertex" << "aTexCoord";
    }

    const char* vertexShader() const final { return vertShader; }
    const char* fragmentShader() const final { return fragShader; }

    void updateState( const YUVState* newState, const YUVState* ) final
    {
        auto gl = QOpenGLContext::currentContext()->functions();
        // We bind the textures in inverse order so that we leave the
        // updateState function with GL_TEXTURE0 as the active texture unit.
        // This is maintain the "contract" that updateState should not mess up
        // the GL state beyond what is needed for this material.
        gl->glActiveTexture( GL_TEXTURE2 );
        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                             GL_LINEAR_MIPMAP_LINEAR );
        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                             GL_LINEAR );
        newState->frontV->bind();

        gl->glActiveTexture( GL_TEXTURE1 );
        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                             GL_LINEAR_MIPMAP_LINEAR );
        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                             GL_LINEAR );
        newState->frontU->bind();

        gl->glActiveTexture( GL_TEXTURE0 );
        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                             GL_LINEAR_MIPMAP_LINEAR );
        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                             GL_LINEAR );
        newState->frontY->bind();
    }

    void resolveUniforms() final
    {
        program()->setUniformValue( "y_tex", 0 ); // GL_TEXTURE0
        program()->setUniformValue( "u_tex", 1 ); // GL_TEXTURE1
        program()->setUniformValue( "v_tex", 2 ); // GL_TEXTURE2
    }
};

YUVState* _getMaterialState( QSGGeometryNode& node )
{
    using YUVShaderMaterial = QSGSimpleMaterial<YUVState>;
    return static_cast<YUVShaderMaterial*>( node.material( ))->state();
}

const YUVState* _getMaterialState( const QSGGeometryNode& node )
{
    using YUVShaderMaterial = QSGSimpleMaterial<YUVState>;
    return static_cast<const YUVShaderMaterial*>( node.material( ))->state();
}

TextureNodeYUV::TextureNodeYUV( const QSize& size, QQuickWindow* window,
                                const TextureFormat format )
    : _window( window )
{
    // Set up geometry, actual vertices will be initialized in updatePaintNode
    const auto& attr = QSGGeometry::defaultAttributes_TexturedPoint2D();
    _node.setGeometry( new QSGGeometry( attr, 4 ));
    _node.setFlag( QSGNode::OwnsGeometry );

    _node.setMaterial( YUVShader::createMaterial( ));
    _node.setFlag( QSGNode::OwnsMaterial );

    auto state = _getMaterialState( _node );
    state->frontY.reset( _window->createTextureFromId( 0, QSize( 1, 1 )));
    state->frontU.reset( _window->createTextureFromId( 0, QSize( 1, 1 )));
    state->frontV.reset( _window->createTextureFromId( 0, QSize( 1, 1 )));

    _createBackTextures( size, format );
    appendChildNode( &_node );
}

const QRectF& TextureNodeYUV::rect() const
{
    return _rect;
}

void TextureNodeYUV::setRect( const QRectF& rect_ )
{
    if( _rect == rect_ )
        return;

    _rect = rect_;
    QSGGeometry::updateTexturedRectGeometry( _node.geometry(), _rect,
                                             UNIT_RECTF );
    _node.markDirty( QSGNode::DirtyGeometry );
}

YUVTexture TextureNodeYUV::getBackGlTexture() const
{
    const auto state = _getMaterialState( _node );

    return YUVTexture{ state->backY->textureId(), state->backU->textureId(),
                       state->backV->textureId() };
}

void TextureNodeYUV::swap()
{
    auto state = _getMaterialState( _node );

    std::swap( state->frontY, state->backY );
    std::swap( state->frontU, state->backU );
    std::swap( state->frontV, state->backV );
    std::swap( state->frontFormat, state->backFormat );

    markDirty( DirtyMaterial );
}

void TextureNodeYUV::prepareBackTexture( const QSize& size,
                                         const TextureFormat format )
{
    auto state = _getMaterialState( _node );
    if( state->backY->textureSize() != size || state->backFormat != format )
        _createBackTextures( size, format );
}

void TextureNodeYUV::_createBackTextures( const QSize& size,
                                          const TextureFormat format )
{
    const auto uvSize = yuv::getUVSize( size, format );
    auto state = _getMaterialState( _node );

    state->backY = _createTexture( size );
    state->backU = _createTexture( uvSize );
    state->backV = _createTexture( uvSize );
    state->backFormat = format;
}

TextureNodeYUV::QSGTexturePtr
TextureNodeYUV::_createTexture( const QSize& size ) const
{
    uint textureID = 0;
    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glGenTextures( 1, &textureID );
    gl->glBindTexture( GL_TEXTURE_2D, textureID );
    gl->glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, size.width(), size.height(), 0,
                      GL_RED, GL_UNSIGNED_BYTE, nullptr );
    return _createWrapper( textureID, size );
}

TextureNodeYUV::QSGTexturePtr
TextureNodeYUV::_createWrapper( const uint textureID, const QSize& size ) const
{
    const auto textureFlags = QQuickWindow::CreateTextureOptions(
                                  QQuickWindow::TextureOwnsGLTexture );
    return QSGTexturePtr( _window->createTextureFromId( textureID, size,
                                                        textureFlags ));
}

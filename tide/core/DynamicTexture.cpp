/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "DynamicTexture.h"

#include "LodTools.h"
#include "log.h"

#include <fstream>
#include <boost/tokenizer.hpp>
#include <cmath>

#include <QDir>
#include <QImageReader>
#include <QtConcurrent>

#define TEXTURE_SIZE 512

namespace
{
const QString PYRAMID_METADATA_FILE_NAME( "pyramid.pyr" );
QRectF imageBounds[] = { QRectF( 0.0, 0.0, 0.5, 0.5 ),
                         QRectF( 0.5, 0.0, 0.5, 0.5 ),
                         QRectF( 0.5, 0.5, 0.5, 0.5 ),
                         QRectF( 0.0, 0.5, 0.5, 0.5 ) };
}

const QString DynamicTexture::pyramidFileExtension = QString( "pyr" );
const QString DynamicTexture::pyramidFolderSuffix = QString( ".pyramid/" );

DynamicTexture::DynamicTexture( const QString& uri )
    : _uri( uri )
    , _useImagePyramid( false )
    , _depth( 0 )
{
    // this is the top-level object, so its path is 0
    _treePath.push_back( 0 );

    const QString extension = QString( "." ).append( pyramidFileExtension );
    if( _uri.endsWith( extension ))
        readPyramidMetadataFromFile( _uri );
    else
        readFullImageMetadata( _uri );
}

DynamicTexture::DynamicTexture( DynamicTexturePtr parent_,
                                const int childIndex )
    : _imageExtension( parent_->_imageExtension )
    , _useImagePyramid( false )
    , _parent( parent_ )
    , _imageCoordsInParentImage( imageBounds[childIndex] )
    , _treePath( parent_->_treePath )
    , _depth( parent_->_depth + 1 )
{
    // append childIndex to parent's path to form this object's path
    _treePath.push_back( childIndex );
}

bool DynamicTexture::isRoot() const
{
    return _depth == 0;
}

bool DynamicTexture::readFullImageMetadata( const QString& uri )
{
    const QImageReader imageReader(uri);
    if( !imageReader.canRead( ))
        return false;

    _imageExtension = QString( imageReader.format( ));
    _imageSize = imageReader.size();
    return true;
}

bool DynamicTexture::readPyramidMetadataFromFile( const QString& uri )
{
    std::ifstream ifs(uri.toLatin1());

    // read the whole line
    std::string lineString;
    getline(ifs, lineString);

    // parse the arguments, allowing escaped characters, quotes, etc., and assign them to a vector
    std::string separator1("\\"); // allow escaped characters
    std::string separator2(" "); // split on spaces
    std::string separator3("\"\'"); // allow quoted arguments

    boost::escaped_list_separator<char> els(separator1, separator2, separator3);
    boost::tokenizer<boost::escaped_list_separator<char> > tokenizer(lineString, els);

    std::vector<std::string> tokens;
    tokens.assign(tokenizer.begin(), tokenizer.end());

    if( tokens.size() != 3 )
    {
        put_flog( LOG_ERROR, "requires 3 arguments, got %i", tokens.size( ));
        return false;
    }

    _imagePyramidPath = QString(tokens[0].c_str());
    if( !determineImageExtension( _imagePyramidPath ))
        return false;

    _imageSize.setWidth(atoi(tokens[1].c_str()));
    _imageSize.setHeight(atoi(tokens[2].c_str()));

    _useImagePyramid = true;

    put_flog( LOG_VERBOSE, "read pyramid file: '%s'', width: %i, height: %i",
              _imagePyramidPath.toLocal8Bit().constData(), _imageSize.width(),
              _imageSize.height( ));

    return true;
}

bool DynamicTexture::determineImageExtension( const QString& imagePyramidPath )
{
    QString extension;
    const QFileInfoList pyramidRootFiles =
            QDir( imagePyramidPath ).entryInfoList( QStringList( "0.*" ));
    if( !pyramidRootFiles.empty( ))
        extension = pyramidRootFiles.first().suffix();
    else
    {
        QString path( imagePyramidPath );
        path.remove( pyramidFolderSuffix );
        extension = path.split(".").last();
    }

    if( !QImageReader().supportedImageFormats().contains( extension.toLatin1( )))
        return false;

    _imageExtension = extension;
    return true;
}

bool DynamicTexture::writeMetadataFile( const QString& pyramidFolder,
                                        const QString& filename ) const
{
    std::ofstream ofs( filename.toStdString().c_str( ));
    if( !ofs.good( ))
    {
        put_flog( LOG_ERROR, "can't write second metadata file: '%s'",
                  filename.toStdString().c_str( ));
        return false;
    }

    ofs << "\"" << pyramidFolder.toStdString() << "\" " << _imageSize.width()
        << " " << _imageSize.height();
    return true;
}

bool DynamicTexture::writePyramidMetadataFiles(const QString& pyramidFolder) const
{
    // First metadata file inside the pyramid folder
    const QString metadataFilename = pyramidFolder + PYRAMID_METADATA_FILE_NAME;

    // Second (more conveniently named) metadata file outside the pyramid folder
    QString secondMetadataFilename = pyramidFolder;
    const int lastIndex = secondMetadataFilename.lastIndexOf(pyramidFolderSuffix);
    secondMetadataFilename.truncate(lastIndex);
    secondMetadataFilename.append(".").append(pyramidFileExtension);

    return writeMetadataFile(pyramidFolder, metadataFilename) &&
           writeMetadataFile(pyramidFolder, secondMetadataFilename);
}

QString DynamicTexture::getPyramidImageFilename() const
{
    QString filename;

    for( unsigned int i=0; i<_treePath.size(); ++i )
    {
        filename.append( QString::number( _treePath[i] ));

        if( i != _treePath.size() - 1 )
            filename.append( "-" );
    }

    filename.append( "." ).append( _imageExtension );

    return filename;
}

bool DynamicTexture::writePyramidImagesRecursive( const QString& pyramidFolder )
{
    _loadImage(); // load this object's scaledImage_

    const QString filename = pyramidFolder + getPyramidImageFilename();
    put_flog( LOG_DEBUG, "saving: '%s'", filename.toLocal8Bit().constData( ));

    if( !_scaledImage.save( filename ))
        return false;
    _scaledImage = QImage(); // no longer need scaled image

    // recursively generate and save children images
    if( _canHaveChildren( ))
    {
        QList<DynamicTexturePtr> children;
        for( unsigned int i=0; i<4; ++i )
            children.push_back( DynamicTexturePtr(
                                  new DynamicTexture( shared_from_this(), i )));

        QtConcurrent::blockingMap( children, [&]( DynamicTexturePtr child ) {
            child->writePyramidImagesRecursive( pyramidFolder );
        });
    }
    return true;
}

bool DynamicTexture::loadFullResImage()
{
    if( !_fullscaleImage.load( _uri ))
    {
        put_flog( LOG_ERROR, "error loading: '%s'",
                  _uri.toLocal8Bit().constData( ));
        return false;
    }
    _imageSize = _fullscaleImage.size();
    return true;
}

void DynamicTexture::_loadImage()
{
    if(isRoot())
    {
        if(_useImagePyramid)
        {
            _scaledImage.load(_imagePyramidPath+'/'+getPyramidImageFilename());
        }
        else
        {
            if (!_fullscaleImage.isNull() || loadFullResImage())
                _scaledImage = _fullscaleImage.scaled(TEXTURE_SIZE, TEXTURE_SIZE, Qt::KeepAspectRatio);
        }
    }
    else
    {
        DynamicTexturePtr root = getRoot();

        if(root->_useImagePyramid)
        {
            _scaledImage.load(root->_imagePyramidPath+'/'+getPyramidImageFilename());
        }
        else
        {
            DynamicTexturePtr parentTex(_parent);
            const QImage image = parentTex->getImageFromParent(_imageCoordsInParentImage, this);

            if(!image.isNull())
            {
                _imageSize= image.size();
                _scaledImage = image.scaled(TEXTURE_SIZE, TEXTURE_SIZE, Qt::KeepAspectRatio);
            }
        }
    }

    if( _scaledImage.isNull( ))
    {
        put_flog( LOG_ERROR, "loading failed in DynamicTexture: '%s'",
                  _uri.toLocal8Bit().constData( ));
    }
}

const QSize& DynamicTexture::getSize() const
{
    return _imageSize;
}

QImage DynamicTexture::getRootImage() const
{
    return QImage( _imagePyramidPath+ '/' + getPyramidImageFilename( ));
}

uint DynamicTexture::getMaxLod() const
{
    return LodTools( _imageSize, TEXTURE_SIZE ).getMaxLod();
}

QString DynamicTexture::getTileFilename( const uint tileId ) const
{
    LodTools::TileIndex idx = _getTileIndex( tileId );

    QString filename = QString( ".%1" ).arg( _imageExtension );

    const uint maxLod = getMaxLod();
    while( ++idx.lod <= maxLod )
    {
        // The indices go in the order: 0-1
        //                              3-2
        int i = 0;
        if( idx.y % 2 )
            i = 3 - idx.x % 2;
        else
            i = idx.x % 2;

        filename.prepend( QString::number( i )).prepend( '-' );
        idx.x = idx.x >> 1;
        idx.y = idx.y >> 1;
    }
    filename.prepend( '0' );
    filename.prepend( _imagePyramidPath );
    return filename;
}

size_t DynamicTexture::getFirstTileId( const uint lod ) const
{
    const size_t maxLod = getMaxLod();

    if( lod == maxLod )
        return 0;

    const size_t nextLod = lod + 1;
    return std::pow( 4, maxLod - nextLod ) + getFirstTileId( nextLod );
}

QRect
DynamicTexture::getTileCoord( const uint lod, const uint x, const uint y ) const
{
    Q_UNUSED( lod );

    // All tiles have the same size in the current implementation, but this is
    // likely to change in the future
    const QSize size = _getTileSize();
    return QRect( QPoint( x * size.width(), y * size.height( )), size );
}

QImage DynamicTexture::getCachableTileImage( const uint tileId ) const
{
    return QImage( getTileFilename( tileId ));
}

Indices DynamicTexture::computeVisibleSet( const QRectF& visibleArea,
                                           const uint lod ) const
{
    if( !_lodTilesMapCache.count( lod ))
        _lodTilesMapCache[ lod ] = _gatherAllTiles( lod );

    const LodTools::TileInfos& tiles = _lodTilesMapCache[ lod ];

    Indices visibleTiles;

    if( visibleArea.isEmpty( ))
        return visibleTiles;

    const QRect rectArea = visibleArea.toRect();

    for( const auto& tile : tiles )
    {
        if( tile.coord.intersects( rectArea ))
            visibleTiles.insert( tile.id );
    }

    return visibleTiles;
}

QRect DynamicTexture::getTileRect( const uint tileId ) const
{
    const auto idx = _getTileIndex( tileId );
    return getTileCoord( idx.lod, idx.x, idx.y );
}

QSize DynamicTexture::getTilesArea( const uint lod ) const
{
    const QSize tileSize = _getTileSize();
    const size_t lodShift = getMaxLod() - lod;
    return QSize( tileSize.width() << lodShift, tileSize.height() << lodShift );
}

QSize DynamicTexture::getTilesCount( const uint lod ) const
{
    const QSize lodSize = getTilesArea( lod );
    const QSize tileSize = _getTileSize();
    return QSize( lodSize.width() / tileSize.width(),
                  lodSize.height() / tileSize.height( ));
}

bool DynamicTexture::_canHaveChildren()
{
    return (getRoot()->_imageSize.width() / (1 << _depth) > TEXTURE_SIZE ||
            getRoot()->_imageSize.height() / (1 << _depth) > TEXTURE_SIZE);
}

QSize DynamicTexture::_getTileSize() const
{
    return _imageSize.scaled( TEXTURE_SIZE, TEXTURE_SIZE, Qt::KeepAspectRatio );
}

bool DynamicTexture::makeFolder( const QString& folder )
{
    if( !QDir( folder ).exists ())
    {
        if( !QDir().mkdir( folder ))
        {
            put_flog( LOG_ERROR, "error creating directory: '%s'",
                      folder.toLocal8Bit().constData( ));
            return false;
        }
    }
    return true;
}

bool DynamicTexture::generateImagePyramid( const QString& outputFolder )
{
    assert( isRoot( ));

    const QString imageName( QFileInfo( _uri ).fileName( ));
    const QString pyramidFolder( QDir( outputFolder ).absolutePath() +
                                 "/" + imageName + pyramidFolderSuffix );

    if( !makeFolder( pyramidFolder ))
        return false;

    if( !writePyramidMetadataFiles( pyramidFolder ))
        return false;

    return writePyramidImagesRecursive( pyramidFolder );
}

DynamicTexturePtr DynamicTexture::getRoot()
{
    if(isRoot())
        return shared_from_this();
    else
        return DynamicTexturePtr(_parent)->getRoot();
}

QRectF DynamicTexture::getImageRegionInParentImage(const QRectF& imageRegion) const
{
    QRectF parentRegion;

    parentRegion.setX(_imageCoordsInParentImage.x() + imageRegion.x() * _imageCoordsInParentImage.width());
    parentRegion.setY(_imageCoordsInParentImage.y() + imageRegion.y() * _imageCoordsInParentImage.height());
    parentRegion.setWidth(imageRegion.width() * _imageCoordsInParentImage.width());
    parentRegion.setHeight(imageRegion.height() * _imageCoordsInParentImage.height());

    return parentRegion;
}

QImage DynamicTexture::getImageFromParent( const QRectF& imageRegion,
                                           DynamicTexture* start )
{
    // if we're in the starting node, we must ascend
    if( start == this )
    {
        if( isRoot( ))
        {
            put_flog( LOG_DEBUG, "root object has no parent! In file: '%s'",
                      _uri.toLocal8Bit().constData( ));
            return QImage();
        }

        DynamicTexturePtr parentTex = _parent.lock();
        return parentTex->getImageFromParent(
                    getImageRegionInParentImage( imageRegion ), this );
    }

    if(!_fullscaleImage.isNull())
    {
        // we have a valid image, return the clipped image
        return _fullscaleImage.copy(imageRegion.x()*_fullscaleImage.width(),
                                    imageRegion.y()*_fullscaleImage.height(),
                                    imageRegion.width()*_fullscaleImage.width(),
                                    imageRegion.height()*_fullscaleImage.height());
    }
    else
    {
        // we don't have a valid image
        // if we're the root object, return an empty image
        // otherwise, continue up the tree looking for an image
        if(isRoot())
            return QImage();

        DynamicTexturePtr parentTex = _parent.lock();
        return parentTex->getImageFromParent(getImageRegionInParentImage(imageRegion), start);
    }
}

LodTools::TileIndex DynamicTexture::_getTileIndex( const uint tileId ) const
{
    uint lod = 0;
    uint firstTileId = getFirstTileId( lod );
    while( tileId < firstTileId )
        firstTileId = getFirstTileId( ++lod );

    const int index = tileId - firstTileId;
    const QSize tilesCount = getTilesCount( lod );

    const uint x = index % tilesCount.width();
    const uint y = index / tilesCount.width();

    return LodTools::TileIndex{ x, y, lod };
}

LodTools::TileInfos DynamicTexture::_gatherAllTiles( const uint lod ) const
{
    LodTools::TileInfos tiles;

    const QSize tilesCount = getTilesCount( lod );
    uint tileId = getFirstTileId( lod );
    for( int y = 0; y < tilesCount.height(); ++y )
    {
        for( int x = 0; x < tilesCount.width(); ++x )
        {
            const QRect& coord = getTileCoord( lod, x, y );
            tiles.push_back( LodTools::TileInfo{ tileId, coord });
            ++tileId;
        }
    }

    return tiles;
}

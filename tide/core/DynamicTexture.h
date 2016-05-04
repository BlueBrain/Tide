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

#ifndef DYNAMIC_TEXTURE_H
#define DYNAMIC_TEXTURE_H

#include "types.h"

#include "CachedDataSource.h"
#include "LodTools.h"

#include <QImage>

/**
 * A dynamically loaded large scale image.
 *
 * It can work with two types of image files:
 * (1) A custom precomuted image pyramid (recommended)
 * (2) Direct reading from a large image
 * @see generateImagePyramid()
 */
class DynamicTexture : public CachedDataSource,
        public std::enable_shared_from_this<DynamicTexture>
{
public:
    /**
     * Constructor
     * @param uri The uri of an image or of a Pyramid metadata file
     */
    explicit DynamicTexture( const QString& uri );

    /** The exension of pyramid metadata files */
    static const QString pyramidFileExtension;

    /** The standard suffix for pyramid image folders */
    static const QString pyramidFolderSuffix;

    /** Get the size of the full resolution texture */
    const QSize& getSize() const;

    /** Get the root image of the pyramid. */
    QImage getRootImage() const;


    /** Get the number of tile at the given lod. */
    QSize getTilesCount( uint lod ) const;

    /** Get the index of the first tile of the given lod. */
    size_t getFirstTileId( uint lod ) const;

    /** Get the coordinates in pixels of a specific tile. */
    QRect getTileCoord( uint lod, uint x, uint y ) const;

    /** Get the tile filename for a given tile index. */
    QString getTileFilename( uint tileId ) const;


    /**
     * @copydoc CachedDataSource::getCachableTileImage
     * threadsafe
     */
    QImage getCachableTileImage( uint tileId ) const final;

    /** @copydoc DataSource::getTileRect */
    QRect getTileRect( uint tileId ) const final;

    /** @copydoc DataSource::getTilesArea */
    QSize getTilesArea( uint lod ) const final;

    /** @copydoc DataSource::computeVisibleSet */
    Indices computeVisibleSet( const QRectF& visibleTilesArea,
                               uint lod ) const final;

    /** @copydoc DataSource::getMaxLod */
    uint getMaxLod() const final;


    /**
     * Generate an image Pyramid from the current uri and save it to the disk.
     * @param outputFolder The folder in which the metadata and pyramid images
     *        will be created.
     */
    bool generateImagePyramid( const QString& outputFolder );

private:
    DynamicTexture( DynamicTexturePtr parent, int childIndex );

    typedef std::map<size_t, LodTools::TileInfos> LodTilesMap;
    mutable LodTilesMap _lodTilesMapCache;

    LodTools::TileIndex _getTileIndex( uint tileId ) const;
    LodTools::TileInfos _gatherAllTiles( uint lod ) const;

    /* for root only: */

    QString _uri;
    QString _imageExtension;

    QString _imagePyramidPath;
    bool _useImagePyramid;

    QImage _fullscaleImage;

    /* for children only: */

    std::weak_ptr<DynamicTexture> _parent;
    QRectF _imageCoordsInParentImage;

    /* for all objects: */

    std::vector<int> _treePath; // To construct the image name for each object
    int _depth; // The depth of the object in the image pyramid

    QSize _imageSize; // full scale image dimensions
    QImage _scaledImage; // for creating the image pyramid

    void _loadImage();
    bool _canHaveChildren();
    QSize _getTileSize() const;

    /** Is this object the root element. */
    bool isRoot() const;  // @All

    /**
     * Get the root object,
     * @return A valid DynamicTexturePtr to the root element
     * @throw boost::bad_weak_ptr exception if the root object is deleted during
     *        thread execution
     */
    DynamicTexturePtr getRoot(); // @Child only

    bool readFullImageMetadata( const QString& uri );

    bool readPyramidMetadataFromFile( const QString& uri ); // @Root only
    bool determineImageExtension( const QString& imagePyramidPath );
    bool makeFolder(const QString& folder ); // @Root only
    bool writeMetadataFile( const QString& pyramidFolder,
                            const QString& filename ) const; // @Root only
    // @Root only
    bool writePyramidMetadataFiles( const QString& pyramidFolder ) const;
    QString getPyramidImageFilename() const; // @All

    bool writePyramidImagesRecursive( const QString& pyramidFolder ); // @All

    QRectF getImageRegionInParentImage( const QRectF& imageRegion ) const;

    bool loadFullResImage(); // @Root only
    QImage getImageFromParent( const QRectF& imageRegion,
                               DynamicTexture* start ); // @Child only
};

#endif

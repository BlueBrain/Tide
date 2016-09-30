
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDEWALL_LINK_LIBRARIES
  PUBLIC
    TideCore
    Qt5::Widgets
  PRIVATE
    Qt5::Svg
)

if(TIDE_ENABLE_MOVIE_SUPPORT)
  list(APPEND TIDEWALL_PUBLIC_HEADERS
    MovieSynchronizer.h
    MovieUpdater.h
  )
  list(APPEND TIDEWALL_SOURCES
    MovieSynchronizer.cpp
    MovieUpdater.cpp
  )
endif()

if(TIDE_ENABLE_PDF_SUPPORT)
  list(APPEND TIDEWALL_PUBLIC_HEADERS
    PDFSynchronizer.h
    PDFTiler.h
  )
  list(APPEND TIDEWALL_SOURCES
    PDFSynchronizer.cpp
    PDFTiler.cpp
  )
endif()

if(TIDE_USE_TIFF)
  list(APPEND TIDEWALL_PUBLIC_HEADERS
    ImagePyramidDataSource.h
    ImagePyramidSynchronizer.h
  )
  list(APPEND TIDEWALL_SOURCES
    ImagePyramidDataSource.cpp
    ImagePyramidSynchronizer.cpp
  )
endif()

list(APPEND TIDEWALL_PUBLIC_HEADERS
  BasicSynchronizer.h
  CachedDataSource.h
  ContentSynchronizer.h
  DataProvider.h
  DataSource.h
  DisplayGroupRenderer.h
  ElapsedTimer.h
  FpsCounter.h
  ImageSource.h
  ImageSynchronizer.h
  LodSynchronizer.h
  LodTiler.h
  LodTools.h
  network/WallFromMasterChannel.h
  network/WallToMasterChannel.h
  network/WallToWallChannel.h
  PixelStreamSynchronizer.h
  PixelStreamUpdater.h
  QmlWindowRenderer.h
  QuadLineNode.h
  QuickRenderer.h
  RenderController.h
  StreamImage.h
  SVGGpuImage.h
  SVGSynchronizer.h
  SVGTiler.h
  TestPattern.h
  TextureNode.h
  TextureUploader.h
  TiledSynchronizer.h
  Tile.h
  VisibilityHelper.h
  WallApplication.h
  WallConfiguration.h
  WallWindow.h
)

list(APPEND TIDEWALL_SOURCES
  BasicSynchronizer.cpp
  CachedDataSource.cpp
  ContentSynchronizer.cpp
  DataProvider.cpp
  DisplayGroupRenderer.cpp
  ElapsedTimer.cpp
  FpsCounter.cpp
  ImageSource.cpp
  ImageSynchronizer.cpp
  LodSynchronizer.cpp
  LodTiler.cpp
  LodTools.cpp
  network/WallFromMasterChannel.cpp
  network/WallToMasterChannel.cpp
  network/WallToWallChannel.cpp
  PixelStreamSynchronizer.cpp
  PixelStreamUpdater.cpp
  QmlWindowRenderer.cpp
  QuadLineNode.cpp
  QuickRenderer.cpp
  RenderController.cpp
  StreamImage.cpp
  SVGGpuImage.cpp
  SVGSynchronizer.cpp
  SVGTiler.cpp
  TestPattern.cpp
  TextureNode.cpp
  TextureUploader.cpp
  Tile.cpp
  TiledSynchronizer.cpp
  VisibilityHelper.cpp
  WallApplication.cpp
  WallConfiguration.cpp
  WallWindow.cpp
  resources/wall.qrc
)

set(TIDEWALL_INCLUDE_NAME tide/wall)
common_library(TideWall)

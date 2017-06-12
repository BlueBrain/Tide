# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDEWALL_LINK_LIBRARIES
  PUBLIC
    TideCore
    Qt5::Gui
  PRIVATE
    DeflectQt
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
  )
  list(APPEND TIDEWALL_SOURCES
    ImagePyramidDataSource.cpp
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
  RenderController.h
  screens.h
  StreamImage.h
  SVGGpuImage.h
  SVGTiler.h
  TestPattern.h
  TextureNode.h
  TextureNodeYUV.h
  textureUtils.h
  TiledSynchronizer.h
  Tile.h
  VisibilityHelper.h
  WallApplication.h
  WallConfiguration.h
  WallSynchronizer.h
  WallWindow.h
)

list(APPEND TIDEWALL_SOURCES
  BasicSynchronizer.cpp
  CachedDataSource.cpp
  DataProvider.cpp
  DisplayGroupRenderer.cpp
  ElapsedTimer.cpp
  FpsCounter.cpp
  ImageSource.cpp
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
  RenderController.cpp
  screens.cpp
  StreamImage.cpp
  SVGGpuImage.cpp
  SVGTiler.cpp
  TestPattern.cpp
  TextureNode.cpp
  TextureNodeYUV.cpp
  textureUtils.cpp
  Tile.cpp
  TiledSynchronizer.cpp
  VisibilityHelper.cpp
  WallApplication.cpp
  WallConfiguration.cpp
  WallSynchronizer.cpp
  WallWindow.cpp
  resources/wall.qrc
)

set(TIDEWALL_INCLUDE_NAME tide/wall)
common_library(TideWall)

target_include_directories(TideWall PRIVATE ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

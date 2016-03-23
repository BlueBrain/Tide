
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

if(ENABLE_PDF_SUPPORT)
  list(APPEND TIDEWALL_PUBLIC_HEADERS
    PDFTiler.h
  )
  list(APPEND TIDEWALL_MOC_PUBLIC_HEADERS
    PDFSynchronizer.h
  )
  list(APPEND TIDEWALL_SOURCES
    PDFSynchronizer.cpp
    PDFTiler.cpp
  )
endif()

list(APPEND TIDEWALL_PUBLIC_HEADERS
  ElapsedTimer.h
  FpsCounter.h
  LodTiler.h
  MovieSynchronizer.h
  MovieUpdater.h
  QuadLineNode.h
  StreamImage.h
  SVGTextureFactory.h
  SVGTiler.h
  TestPattern.h
  TextureNode.h
  VisibilityHelper.h
  WallConfiguration.h
)

list(APPEND TIDEWALL_MOC_PUBLIC_HEADERS
  BasicSynchronizer.h
  ContentSynchronizer.h
  DataProvider.h
  DisplayGroupRenderer.h
  DynamicTextureSynchronizer.h
  ImageSynchronizer.h
  LodSynchronizer.h
  PixelStreamSynchronizer.h
  PixelStreamUpdater.h
  QmlWindowRenderer.h
  QuickRenderer.h
  RenderController.h
  SVGSynchronizer.h
  TextureUploader.h
  TiledSynchronizer.h
  Tile.h
  WallApplication.h
  WallToWallChannel.h
  WallWindow.h
)

list(APPEND TIDEWALL_SOURCES
  BasicSynchronizer.cpp
  ContentSynchronizer.cpp
  DataProvider.cpp
  DisplayGroupRenderer.cpp
  DynamicTextureSynchronizer.cpp
  ElapsedTimer.cpp
  FpsCounter.cpp
  ImageSynchronizer.cpp
  LodSynchronizer.cpp
  LodTiler.cpp
  MovieSynchronizer.cpp
  MovieUpdater.cpp
  PixelStreamSynchronizer.cpp
  PixelStreamUpdater.cpp
  QmlWindowRenderer.cpp
  QuadLineNode.cpp
  QuickRenderer.cpp
  RenderController.cpp
  StreamImage.cpp
  SVGImage.cpp
  SVGSynchronizer.cpp
  SVGTextureFactory.cpp
  SVGTiler.cpp
  TestPattern.cpp
  TextureNode.cpp
  TextureUploader.cpp
  Tile.cpp
  TiledSynchronizer.cpp
  VisibilityHelper.cpp
  WallApplication.cpp
  WallConfiguration.cpp
  WallToWallChannel.cpp
  WallWindow.cpp
)

set(TIDEWALL_RESOURCES resources/wall.qrc)

set(TIDEWALL_INCLUDE_NAME tide/wall)
common_library(TideWall)

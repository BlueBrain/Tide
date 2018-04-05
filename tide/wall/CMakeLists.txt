# Copyright (c) 2013-2018, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDEWALL_LINK_LIBRARIES
  PUBLIC
    TideCore
    Qt5::Gui
  PRIVATE
    DeflectQt
)

if(TARGET Qt5::X11Extras)
  list(APPEND TIDEWALL_LINK_LIBRARIES PRIVATE Qt5::X11Extras)
endif()

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
  BackgroundRenderer.h
  BasicSynchronizer.h
  CachedDataSource.h
  ContentSynchronizer.h
  ContentWindowRenderer.h
  DataProvider.h
  DataSource.h
  DisplayGroupRenderer.h
  ElapsedTimer.h
  FpsCounter.h
  HardwareSwapGroup.h
  ImageSource.h
  LodSynchronizer.h
  LodTiler.h
  LodTools.h
  network/WallFromMasterChannel.h
  network/WallToMasterChannel.h
  network/WallToWallChannel.h
  PixelStreamAssembler.h
  PixelStreamProcessor.h
  PixelStreamPassthrough.h
  PixelStreamSynchronizer.h
  PixelStreamUpdater.h
  QuadLineNode.h
  RenderController.h
  screens.h
  StreamImage.h
  SVGGpuImage.h
  SVGTiler.h
  SwapSynchronizer.h
  SwapSynchronizerHardware.h
  SwapSynchronizerSoftware.h
  TestPattern.h
  TextureBorderSwitcher.h
  TextureNode.h
  TextureNodeFactory.h
  TextureNodeRGBA.h
  TextureNodeYUV.h
  TextureSwitcher.h
  textureUtils.h
  TiledSynchronizer.h
  Tile.h
  VisibilityHelper.h
  WallApplication.h
  WallConfiguration.h
  WallRenderContext.h
  WallSurfaceRenderer.h
  WallWindow.h
)

list(APPEND TIDEWALL_SOURCES
  BackgroundRenderer.cpp
  BasicSynchronizer.cpp
  CachedDataSource.cpp
  ContentWindowRenderer.cpp
  DataProvider.cpp
  DisplayGroupRenderer.cpp
  ElapsedTimer.cpp
  FpsCounter.cpp
  HardwareSwapGroup.cpp
  ImageSource.cpp
  LodSynchronizer.cpp
  LodTiler.cpp
  LodTools.cpp
  network/WallFromMasterChannel.cpp
  network/WallToMasterChannel.cpp
  network/WallToWallChannel.cpp
  PixelStreamAssembler.cpp
  PixelStreamProcessor.cpp
  PixelStreamPassthrough.cpp
  PixelStreamSynchronizer.cpp
  PixelStreamUpdater.cpp
  QuadLineNode.cpp
  RenderController.cpp
  screens.cpp
  StreamImage.cpp
  SVGGpuImage.cpp
  SVGTiler.cpp
  SwapSynchronizer.cpp
  SwapSynchronizerHardware.cpp
  SwapSynchronizerSoftware.cpp
  TestPattern.cpp
  TextureBorderSwitcher.cpp
  TextureNodeFactory.cpp
  TextureNodeRGBA.cpp
  TextureNodeYUV.cpp
  TextureSwitcher.cpp
  textureUtils.cpp
  Tile.cpp
  TiledSynchronizer.cpp
  VisibilityHelper.cpp
  WallApplication.cpp
  WallConfiguration.cpp
  WallSurfaceRenderer.cpp
  WallWindow.cpp
  resources/wall.qrc
)

set(TIDEWALL_INCLUDE_NAME tide/wall)
common_library(TideWall)

target_include_directories(TideWall PRIVATE ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

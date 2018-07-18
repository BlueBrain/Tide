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
    datasources/MovieUpdater.h
    synchronizers/MovieSynchronizer.h
  )
  list(APPEND TIDEWALL_SOURCES
    datasources/MovieUpdater.cpp
    synchronizers/MovieSynchronizer.cpp
  )
endif()

if(TIDE_ENABLE_PDF_SUPPORT)
  list(APPEND TIDEWALL_PUBLIC_HEADERS
    datasources/PDFTiler.h
    synchronizers/PDFSynchronizer.h
  )
  list(APPEND TIDEWALL_SOURCES
    datasources/PDFTiler.cpp
    synchronizers/PDFSynchronizer.cpp
  )
endif()

if(TIDE_USE_TIFF)
  list(APPEND TIDEWALL_PUBLIC_HEADERS
    datasources/ImagePyramidDataSource.h
  )
  list(APPEND TIDEWALL_SOURCES
    datasources/ImagePyramidDataSource.cpp
  )
endif()

list(APPEND TIDEWALL_PUBLIC_HEADERS
  DataProvider.h
  datasources/CachedDataSource.h
  datasources/DataSource.h
  datasources/ImageSource.h
  datasources/LodTiler.h
  datasources/SVGGpuImage.h
  datasources/SVGTiler.h
  datasources/PixelStreamUpdater.h
  network/WallFromMasterChannel.h
  network/WallToMasterChannel.h
  network/WallToWallChannel.h
  qml/BackgroundRenderer.h
  qml/DisplayGroupRenderer.h
  qml/qscreens.h
  qml/QuadLineNode.h
  qml/TestPattern.h
  qml/TextureBorderSwitcher.h
  qml/TextureNode.h
  qml/TextureNodeFactory.h
  qml/TextureNodeRGBA.h
  qml/TextureNodeYUV.h
  qml/TextureSwitcher.h
  qml/textureUtils.h
  qml/Tile.h
  qml/WallRenderContext.h
  qml/WallSurfaceRenderer.h
  qml/WallWindow.h
  qml/WindowRenderer.h
  RenderController.h
  synchronizers/BasicSynchronizer.h
  synchronizers/ContentSynchronizer.h
  synchronizers/LodSynchronizer.h
  synchronizers/PixelStreamSynchronizer.h
  synchronizers/TiledSynchronizer.h
  swapsync/HardwareSwapGroup.h
  swapsync/SwapSynchronizer.h
  swapsync/SwapSynchronizerHardware.h
  swapsync/SwapSynchronizerSoftware.h
  tools/ElapsedTimer.h
  tools/FpsCounter.h
  tools/LodTools.h
  tools/PixelStreamAssembler.h
  tools/PixelStreamChannelAssembler.h
  tools/PixelStreamProcessor.h
  tools/PixelStreamPassthrough.h
  tools/SwapSyncObject.h
  tools/VisibilityHelper.h
  WallApplication.h
  WallConfiguration.h
)

list(APPEND TIDEWALL_SOURCES
  datasources/CachedDataSource.cpp
  datasources/ImageSource.cpp
  datasources/LodTiler.cpp
  datasources/SVGGpuImage.cpp
  datasources/SVGTiler.cpp
  datasources/PixelStreamUpdater.cpp
  DataProvider.cpp
  network/WallFromMasterChannel.cpp
  network/WallToMasterChannel.cpp
  network/WallToWallChannel.cpp
  qml/BackgroundRenderer.cpp
  qml/DisplayGroupRenderer.cpp
  qml/qscreens.cpp
  qml/QuadLineNode.cpp
  qml/TestPattern.cpp
  qml/TextureBorderSwitcher.cpp
  qml/TextureNodeFactory.cpp
  qml/TextureNodeRGBA.cpp
  qml/TextureNodeYUV.cpp
  qml/TextureSwitcher.cpp
  qml/textureUtils.cpp
  qml/Tile.cpp
  qml/WallSurfaceRenderer.cpp
  qml/WindowRenderer.cpp
  qml/WallWindow.cpp
  RenderController.cpp
  swapsync/HardwareSwapGroup.cpp
  swapsync/SwapSynchronizer.cpp
  swapsync/SwapSynchronizerHardware.cpp
  swapsync/SwapSynchronizerSoftware.cpp
  synchronizers/BasicSynchronizer.cpp
  synchronizers/LodSynchronizer.cpp
  synchronizers/PixelStreamSynchronizer.cpp
  synchronizers/TiledSynchronizer.cpp
  tools/ElapsedTimer.cpp
  tools/FpsCounter.cpp
  tools/LodTools.cpp
  tools/PixelStreamAssembler.cpp
  tools/PixelStreamChannelAssembler.cpp
  tools/PixelStreamProcessor.cpp
  tools/PixelStreamPassthrough.cpp
  tools/VisibilityHelper.cpp
  WallApplication.cpp
  WallConfiguration.cpp
  resources/wall.qrc
)

set(TIDEWALL_INCLUDE_NAME tide/wall)
common_library(TideWall)

target_include_directories(TideWall PRIVATE ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

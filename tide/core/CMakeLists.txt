
# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDECORE_LINK_LIBRARIES
  PUBLIC
    Deflect
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Qt5::Xml
    Qt5::XmlPatterns
    ${Boost_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
  PRIVATE
    ${FFMPEG_LIBRARIES}
    Qt5::Concurrent
    Qt5::Svg
)

if(TIDE_ENABLE_MOVIE_SUPPORT)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    FFMPEGFrame.h
    FFMPEGMovie.h
    FFMPEGPicture.h
    FFMPEGVideoFrameConverter.h
    FFMPEGVideoStream.h
    MovieContent.h
    thumbnail/MovieThumbnailGenerator.h
  )
  list(APPEND TIDECORE_SOURCES
    FFMPEGFrame.cpp
    FFMPEGMovie.cpp
    FFMPEGPicture.cpp
    FFMPEGVideoFrameConverter.cpp
    FFMPEGVideoStream.cpp
    MovieContent.cpp
    thumbnail/MovieThumbnailGenerator.cpp
  )
endif()

if(TIDE_ENABLE_PDF_SUPPORT)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    PDFContent.h
  )
  list(APPEND TIDECORE_SOURCES
    PDF.cpp
    PDFContent.cpp
    PDFInteractionDelegate.cpp
    thumbnail/PDFThumbnailGenerator.cpp
  )
  list(APPEND TIDECORE_LINK_LIBRARIES PRIVATE ${POPPLER_LIBRARIES})
endif()

list(APPEND TIDECORE_PUBLIC_HEADERS
  CachedDataSource.h
  CommandLineParameters.h
  Configuration.h
  ContentAction.h
  ContentActionsModel.h
  ContentFactory.h
  Content.h
  ContentInteractionDelegate.h
  ContentType.h
  ContentWindowController.h
  ContentWindow.h
  Coordinates.h
  DataSource.h
  DisplayGroup.h
  DynamicTextureContent.h
  DynamicTexture.h
  Image.h
  LayoutEngine.h
  LodTools.h
  log.h
  LoggingUtility.h
  Markers.h
  MasterFromWallChannel.h
  MasterToWallChannel.h
  MPIChannel.h
  MPIContext.h
  MPINospin.h
  Options.h
  PixelStreamContent.h
  PixelStreamInteractionDelegate.h
  QmlTypeRegistration.h
  QtImage.h
  StatePreview.h
  SerializeBuffer.h
  SVGContent.h
  TextureContent.h
  thumbnail/DefaultThumbnailGenerator.h
  thumbnail/FolderThumbnailGenerator.h
  thumbnail/ImageThumbnailGenerator.h
  thumbnail/PyramidThumbnailGenerator.h
  thumbnail/StateThumbnailGenerator.h
  thumbnail/ThumbnailGeneratorFactory.h
  thumbnail/ThumbnailGenerator.h
  thumbnail/ThumbnailProvider.h
  types.h
  VectorialContent.h
  WallFromMasterChannel.h
  WallToMasterChannel.h
  WebbrowserContent.h
  ZoomHelper.h
  ZoomInteractionDelegate.h
)

list(APPEND TIDECORE_SOURCES
  CachedDataSource.cpp
  CommandLineParameters.cpp
  Configuration.cpp
  ContentAction.cpp
  ContentActionsModel.cpp
  Content.cpp
  ContentFactory.cpp
  ContentInteractionDelegate.cpp
  ContentType.cpp
  ContentWindowController.cpp
  ContentWindow.cpp
  Coordinates.cpp
  DisplayGroup.cpp
  DynamicTextureContent.cpp
  DynamicTexture.cpp
  LayoutEngine.cpp
  LodTools.cpp
  log.cpp
  LoggingUtility.cpp
  Markers.cpp
  MasterFromWallChannel.cpp
  MasterToWallChannel.cpp
  MetaTypeRegistration.cpp
  MPIChannel.cpp
  MPIContext.cpp
  MPINospin.cpp
  Options.cpp
  PixelStreamContent.cpp
  PixelStreamInteractionDelegate.cpp
  QtImage.cpp
  StatePreview.cpp
  SVGContent.cpp
  TextureContent.cpp
  thumbnail/DefaultThumbnailGenerator.cpp
  thumbnail/FolderThumbnailGenerator.cpp
  thumbnail/ImageThumbnailGenerator.cpp
  thumbnail/PyramidThumbnailGenerator.cpp
  thumbnail/StateThumbnailGenerator.cpp
  thumbnail/ThumbnailGenerator.cpp
  thumbnail/ThumbnailGeneratorFactory.cpp
  thumbnail/ThumbnailProvider.cpp
  VectorialContent.cpp
  WallFromMasterChannel.cpp
  WallToMasterChannel.cpp
  WebbrowserContent.cpp
  ZoomHelper.cpp
  ZoomInteractionDelegate.cpp
  resources/core.qrc
)

set(TIDECORE_INCLUDE_NAME tide/core)
common_library(TideCore)

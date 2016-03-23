
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
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
    Qt5::Svg
)

if(ENABLE_PDF_SUPPORT)
  list(APPEND TIDECORE_MOC_PUBLIC_HEADERS
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
  ContentFactory.h
  ContentType.h
  DataSource.h
  DynamicTextureContent.h
  DynamicTexture.h
  FFMPEGFrame.h
  FFMPEGMovie.h
  FFMPEGPicture.h
  FFMPEGVideoFrameConverter.h
  FFMPEGVideoStream.h
  Image.h
  LayoutEngine.h
  LodTools.h
  log.h
  MPIChannel.h
  MPIContext.h
  MPINospin.h
  PixelStreamContent.h
  QmlTypeRegistration.h
  QtImage.h
  StatePreview.h
  TextureContent.h
  thumbnail/DefaultThumbnailGenerator.h
  thumbnail/FolderThumbnailGenerator.h
  thumbnail/ImageThumbnailGenerator.h
  thumbnail/MovieThumbnailGenerator.h
  thumbnail/PyramidThumbnailGenerator.h
  thumbnail/StateThumbnailGenerator.h
  thumbnail/ThumbnailGeneratorFactory.h
  thumbnail/ThumbnailGenerator.h
  types.h
  ZoomHelper.h
  ZoomInteractionDelegate.h
)

list(APPEND TIDECORE_MOC_PUBLIC_HEADERS
  ContentAction.h
  ContentActionsModel.h
  Content.h
  ContentInteractionDelegate.h
  ContentWindowController.h
  ContentWindow.h
  Coordinates.h
  DisplayGroup.h
  Markers.h
  MasterFromWallChannel.h
  MasterToWallChannel.h
  MovieContent.h
  Options.h
  PixelStreamInteractionDelegate.h
  QmlControlPanel.h
  SVGContent.h
  VectorialContent.h
  WallFromMasterChannel.h
  WallToMasterChannel.h
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
  FFMPEGFrame.cpp
  FFMPEGMovie.cpp
  FFMPEGPicture.cpp
  FFMPEGVideoFrameConverter.cpp
  FFMPEGVideoStream.cpp
  LayoutEngine.cpp
  LodTools.cpp
  log.cpp
  Markers.cpp
  MasterFromWallChannel.cpp
  MasterToWallChannel.cpp
  MetaTypeRegistration.cpp
  MovieContent.cpp
  MPIChannel.cpp
  MPIContext.cpp
  MPINospin.cpp
  Options.cpp
  PixelStreamContent.cpp
  PixelStreamInteractionDelegate.cpp
  QmlControlPanel.cpp
  QtImage.cpp
  StatePreview.cpp
  SVGContent.cpp
  TextureContent.cpp
  thumbnail/DefaultThumbnailGenerator.cpp
  thumbnail/FolderThumbnailGenerator.cpp
  thumbnail/ImageThumbnailGenerator.cpp
  thumbnail/MovieThumbnailGenerator.cpp
  thumbnail/PyramidThumbnailGenerator.cpp
  thumbnail/StateThumbnailGenerator.cpp
  thumbnail/ThumbnailGenerator.cpp
  thumbnail/ThumbnailGeneratorFactory.cpp
  VectorialContent.cpp
  WallFromMasterChannel.cpp
  WallToMasterChannel.cpp
  ZoomHelper.cpp
  ZoomInteractionDelegate.cpp
)

set(TIDECORE_RESOURCES resources/core.qrc)

set(TIDECORE_INCLUDE_NAME tide/core)
common_library(TideCore)

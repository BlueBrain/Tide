
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

if(TIDE_USE_CAIRO)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    CairoWrappers.h
  )
endif()

if(TIDE_ENABLE_PDF_SUPPORT)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    PDFBackend.h
    PDFContent.h
  )
  list(APPEND TIDECORE_SOURCES
    PDF.cpp
    PDFContent.cpp
    thumbnail/PDFThumbnailGenerator.cpp
  )
  if(TIDE_USE_CAIRO AND TIDE_USE_POPPLER_GLIB)
    list(APPEND TIDECORE_PUBLIC_HEADERS
      PDFPopplerCairoBackend.h
    )
    list(APPEND TIDECORE_SOURCES
      PDFPopplerCairoBackend.cpp
    )
    list(APPEND TIDECORE_LINK_LIBRARIES
      PRIVATE ${poppler-glib_LIBRARIES}
    )
  endif()
  if(TIDE_USE_POPPLER_QT5)
    list(APPEND TIDECORE_PUBLIC_HEADERS
      PDFPopplerQtBackend.h
    )
    list(APPEND TIDECORE_SOURCES
      PDFPopplerQtBackend.cpp
    )
    list(APPEND TIDECORE_LINK_LIBRARIES
      PRIVATE ${poppler-qt5_LIBRARIES}
    )
  endif()
endif()

if(TIDE_USE_CAIRO AND TIDE_USE_RSVG)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    SVGCairoRSVGBackend.h
  )
  list(APPEND TIDECORE_SOURCES
    SVGCairoRSVGBackend.cpp
  )
  list(APPEND TIDECORE_LINK_LIBRARIES
    PRIVATE ${RSVG_LIBRARIES}
  )
endif()

if(TIDE_USE_TIFF)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    ImagePyramidContent.h
    TiffPyramidReader.h
    thumbnail/ImagePyramidThumbnailGenerator.h
  )
  list(APPEND TIDECORE_SOURCES
    ImagePyramidContent.cpp
    TiffPyramidReader.cpp
    thumbnail/ImagePyramidThumbnailGenerator.cpp
  )
  list(APPEND TIDECORE_LINK_LIBRARIES
    PRIVATE
      ${TIFF_LIBRARIES}
  )
endif()

list(APPEND TIDECORE_PUBLIC_HEADERS
  CommandLineParameters.h
  Configuration.h
  ContentAction.h
  ContentActionsModel.h
  ContentFactory.h
  Content.h
  ContentType.h
  ContentWindow.h
  Coordinates.h
  DisplayGroup.h
  DynamicTextureContent.h
  geometry.h
  Image.h
  KeyboardState.h
  log.h
  Markers.h
  network/MPIChannel.h
  network/MPIContext.h
  network/MPIHeader.h
  network/MPINospin.h
  network/ReceiveBuffer.h
  Options.h
  PixelStreamContent.h
  QmlTypeRegistration.h
  qmlUtils.h
  QtImage.h
  StatePreview.h
  serialization/chrono.h
  serialization/deflectTypes.h
  serialization/includes.h
  serialization/qtTypes.h
  serialization/utils.h
  serialization/xmlMacros.h
  SVG.h
  SVGBackend.h
  SVGContent.h
  SVGQtGpuBackend.h
  TextureContent.h
  thumbnail/DefaultThumbnailGenerator.h
  thumbnail/FolderThumbnailGenerator.h
  thumbnail/ImageThumbnailGenerator.h
  thumbnail/StateThumbnailGenerator.h
  thumbnail/ThumbnailGeneratorFactory.h
  thumbnail/ThumbnailGenerator.h
  thumbnail/ThumbnailProvider.h
  types.h
  VectorialContent.h
  ZoomHelper.h
)

list(APPEND TIDECORE_SOURCES
  CommandLineParameters.cpp
  Configuration.cpp
  ContentAction.cpp
  ContentActionsModel.cpp
  Content.cpp
  ContentFactory.cpp
  ContentType.cpp
  ContentWindow.cpp
  Coordinates.cpp
  DisplayGroup.cpp
  DynamicTextureContent.cpp
  geometry.cpp
  KeyboardState.cpp
  log.cpp
  Markers.cpp
  MetaTypeRegistration.cpp
  network/MPIChannel.cpp
  network/MPIContext.cpp
  network/MPINospin.cpp
  Options.cpp
  PixelStreamContent.cpp
  QtImage.cpp
  StatePreview.cpp
  SVG.cpp
  SVGContent.cpp
  SVGQtGpuBackend.cpp
  TextureContent.cpp
  thumbnail/DefaultThumbnailGenerator.cpp
  thumbnail/FolderThumbnailGenerator.cpp
  thumbnail/ImageThumbnailGenerator.cpp
  thumbnail/StateThumbnailGenerator.cpp
  thumbnail/ThumbnailGenerator.cpp
  thumbnail/ThumbnailGeneratorFactory.cpp
  thumbnail/ThumbnailProvider.cpp
  VectorialContent.cpp
  ZoomHelper.cpp
  resources/core.qrc
)

if(TARGET Qt5::WebKitWidgets OR TARGET Qt5::WebEngine)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    WebbrowserContent.h
    WebbrowserHistory.h
  )
  list(APPEND TIDECORE_SOURCES
    WebbrowserContent.cpp
    WebbrowserHistory.cpp
  )
endif()
if(TARGET Qt5::WebKitWidgets)
  list(APPEND TIDECORE_LINK_LIBRARIES PRIVATE Qt5::WebKitWidgets)
endif()

set(TIDECORE_INCLUDE_NAME tide/core)
common_library(TideCore)

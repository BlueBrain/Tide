# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

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
    Qt5::Concurrent
    Qt5::Svg
)

if(TIDE_ENABLE_MOVIE_SUPPORT)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    data/FFMPEGDefines.h
    data/FFMPEGFrame.h
    data/FFMPEGMovie.h
    data/FFMPEGPicture.h
    data/FFMPEGVideoFrameConverter.h
    data/FFMPEGVideoStream.h
    scene/MovieContent.h
    thumbnail/MovieThumbnailGenerator.h
  )
  list(APPEND TIDECORE_SOURCES
    data/FFMPEGFrame.cpp
    data/FFMPEGMovie.cpp
    data/FFMPEGPicture.cpp
    data/FFMPEGVideoFrameConverter.cpp
    data/FFMPEGVideoStream.cpp
    scene/MovieContent.cpp
    thumbnail/MovieThumbnailGenerator.cpp
  )
  list(APPEND TIDECORE_LINK_LIBRARIES
    PRIVATE ${FFMPEG_LIBRARIES}
  )
endif()

if(TIDE_USE_CAIRO)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    data/CairoWrappers.h
  )
endif()

if(TIDE_ENABLE_PDF_SUPPORT)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    data/PDFBackend.h
    scene/PDFContent.h
  )
  list(APPEND TIDECORE_SOURCES
    data/PDF.cpp
    scene/PDFContent.cpp
    thumbnail/PDFThumbnailGenerator.cpp
  )
  if(TIDE_USE_CAIRO AND TIDE_USE_POPPLER_GLIB)
    list(APPEND TIDECORE_PUBLIC_HEADERS
      data/PDFPopplerCairoBackend.h
    )
    list(APPEND TIDECORE_SOURCES
      data/PDFPopplerCairoBackend.cpp
    )
    list(APPEND TIDECORE_LINK_LIBRARIES
      PRIVATE ${poppler-glib_LIBRARIES}
    )
  endif()
  if(TIDE_USE_POPPLER_QT5)
    list(APPEND TIDECORE_PUBLIC_HEADERS
      data/PDFPopplerQtBackend.h
    )
    list(APPEND TIDECORE_SOURCES
      data/PDFPopplerQtBackend.cpp
    )
    list(APPEND TIDECORE_LINK_LIBRARIES
      PRIVATE ${poppler-qt5_LIBRARIES}
    )
  endif()
endif()

if(TIDE_USE_CAIRO AND TIDE_USE_RSVG)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    data/SVGCairoRSVGBackend.h
  )
  list(APPEND TIDECORE_SOURCES
    data/SVGCairoRSVGBackend.cpp
  )
  list(APPEND TIDECORE_LINK_LIBRARIES
    PRIVATE ${RSVG_LIBRARIES}
  )
endif()

if(TIDE_USE_TIFF)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    data/TiffPyramidReader.h
    scene/ImagePyramidContent.h
    thumbnail/ImagePyramidThumbnailGenerator.h
  )
  list(APPEND TIDECORE_SOURCES
    data/TiffPyramidReader.cpp
    scene/ImagePyramidContent.cpp
    thumbnail/ImagePyramidThumbnailGenerator.cpp
  )
  list(APPEND TIDECORE_LINK_LIBRARIES
    PRIVATE
      ${TIFF_LIBRARIES}
  )
endif()

list(APPEND TIDECORE_PUBLIC_HEADERS
  CommandLineParameters.h
  CommandLineParser.h
  Configuration.h
  data/Image.h
  data/QtImage.h
  data/SVG.h
  data/SVGBackend.h
  data/SVGQtGpuBackend.h
  data/YUVImage.h
  geometry.h
  InactivityTimer.h
  log.h
  network/LocalBarrier.h
  network/MPIChannel.h
  network/MPIContext.h
  network/MPIHeader.h
  network/MPINospin.h
  network/NetworkBarrier.h
  network/ReceiveBuffer.h
  network/SharedNetworkBarrier.h
  QmlTypeRegistration.h
  qmlUtils.h
  StatePreview.h
  scene/ContentAction.h
  scene/ContentActionsModel.h
  scene/ContentFactory.h
  scene/Content.h
  scene/ContentType.h
  scene/ContentWindow.h
  scene/DisplayGroup.h
  scene/DynamicTextureContent.h
  scene/KeyboardState.h
  scene/Markers.h
  scene/Options.h
  scene/PixelStreamContent.h
  scene/Rectangle.h
  scene/SVGContent.h
  scene/TextureContent.h
  scene/VectorialContent.h
  serialization/chrono.h
  serialization/deflectTypes.h
  serialization/includes.h
  serialization/qtTypes.h
  serialization/utils.h
  serialization/xmlMacros.h
  thumbnail/DefaultThumbnailGenerator.h
  thumbnail/FolderThumbnailGenerator.h
  thumbnail/ImageThumbnailGenerator.h
  thumbnail/StateThumbnailGenerator.h
  thumbnail/StreamThumbnailGenerator.h
  thumbnail/thumbnail.h
  thumbnail/ThumbnailGeneratorFactory.h
  thumbnail/ThumbnailGenerator.h
  thumbnail/ThumbnailProvider.h
  types.h
  yuv.h
  ZoomHelper.h
)

list(APPEND TIDECORE_SOURCES
  CommandLineParameters.cpp
  CommandLineParser.cpp
  Configuration.cpp
  data/QtImage.cpp
  data/SVG.cpp
  data/SVGQtGpuBackend.cpp
  data/YUVImage.cpp
  geometry.cpp
  InactivityTimer.cpp
  log.cpp
  MetaTypeRegistration.cpp
  network/LocalBarrier.cpp
  network/MPIChannel.cpp
  network/MPIContext.cpp
  network/MPINospin.cpp
  network/SharedNetworkBarrier.cpp
  scene/ContentAction.cpp
  scene/ContentActionsModel.cpp
  scene/Content.cpp
  scene/ContentFactory.cpp
  scene/ContentType.cpp
  scene/ContentWindow.cpp
  scene/DisplayGroup.cpp
  scene/DynamicTextureContent.cpp
  scene/KeyboardState.cpp
  scene/PixelStreamContent.cpp
  scene/Markers.cpp
  scene/Options.cpp
  scene/Rectangle.cpp
  scene/SVGContent.cpp
  scene/TextureContent.cpp
  scene/VectorialContent.cpp
  StatePreview.cpp
  thumbnail/DefaultThumbnailGenerator.cpp
  thumbnail/FolderThumbnailGenerator.cpp
  thumbnail/ImageThumbnailGenerator.cpp
  thumbnail/StateThumbnailGenerator.cpp
  thumbnail/StreamThumbnailGenerator.cpp
  thumbnail/thumbnail.cpp
  thumbnail/ThumbnailGenerator.cpp
  thumbnail/ThumbnailGeneratorFactory.cpp
  thumbnail/ThumbnailProvider.cpp
  yuv.cpp
  ZoomHelper.cpp
  resources/core.qrc
)

if(TARGET Qt5::WebKitWidgets OR TARGET Qt5::WebEngine)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    scene/WebbrowserContent.h
    scene/WebbrowserHistory.h
    thumbnail/WebbrowserThumbnailGenerator.h
  )
  list(APPEND TIDECORE_SOURCES
    scene/WebbrowserContent.cpp
    scene/WebbrowserHistory.cpp
    thumbnail/WebbrowserThumbnailGenerator.cpp
  )
endif()
if(TARGET Qt5::WebKitWidgets)
  list(APPEND TIDECORE_PUBLIC_HEADERS
    scene/AddressBar.h
  )
  list(APPEND TIDECORE_SOURCES
    scene/AddressBar.cpp
  )
  list(APPEND TIDECORE_LINK_LIBRARIES PRIVATE Qt5::WebKitWidgets)
endif()

set(TIDECORE_INCLUDE_NAME tide/core)
common_library(TideCore)

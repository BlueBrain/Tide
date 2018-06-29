# Copyright (c) 2013-2018, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDECORE_LINK_LIBRARIES
  PUBLIC
    DeflectServer
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
  CommandLineParser.h
  geometry.h
  log.h
  QmlTypeRegistration.h
  qmlUtils.h
  StatePreview.h
  types.h
  yuv.h
  ZoomHelper.h
  configuration/Configuration.h
  configuration/ConfigurationWriter.h
  configuration/Process.h
  configuration/Screen.h
  configuration/SurfaceConfig.h
  configuration/SurfaceConfigValidator.h
  configuration/XmlParser.h
  data/Image.h
  data/QtImage.h
  data/SVG.h
  data/SVGBackend.h
  data/SVGQtGpuBackend.h
  data/YUVImage.h
  json/json.h
  json/serialization.h
  json/templates.h
  network/LocalBarrier.h
  network/MPIChannel.h
  network/MPIContext.h
  network/MPIHeader.h
  network/MPINospin.h
  network/NetworkBarrier.h
  network/ReceiveBuffer.h
  network/SharedNetworkBarrier.h
  scene/Background.h
  scene/ContentAction.h
  scene/ContentActionsModel.h
  scene/ContentFactory.h
  scene/Content.h
  scene/ContentType.h
  scene/ContextMenu.h
  scene/CountdownStatus.h
  scene/DisplayGroup.h
  scene/DynamicTextureContent.h
  scene/ErrorContent.h
  scene/KeyboardState.h
  scene/Markers.h
  scene/MultiChannelContent.h
  scene/Options.h
  scene/PixelStreamContent.h
  scene/Rectangle.h
  scene/Scene.h
  scene/ScreenLock.h
  scene/Surface.h
  scene/SVGContent.h
  scene/TextureContent.h
  scene/VectorialContent.h
  scene/Window.h
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
  utils/compilerMacros.h
  utils/IterableSmartPtrCollection.h
)

list(APPEND TIDECORE_SOURCES
  CommandLineParser.cpp
  geometry.cpp
  log.cpp
  MetaTypeRegistration.cpp
  StatePreview.cpp
  yuv.cpp
  ZoomHelper.cpp
  configuration/Configuration.cpp
  configuration/ConfigurationWriter.cpp
  configuration/SurfaceConfig.cpp
  configuration/SurfaceConfigValidator.cpp
  configuration/XmlParser.cpp
  data/QtImage.cpp
  data/SVG.cpp
  data/SVGQtGpuBackend.cpp
  data/YUVImage.cpp
  json/json.cpp
  json/serialization.cpp
  network/LocalBarrier.cpp
  network/MPIChannel.cpp
  network/MPIContext.cpp
  network/MPINospin.cpp
  network/SharedNetworkBarrier.cpp
  resources/core.qrc
  scene/Background.cpp
  scene/ContentAction.cpp
  scene/ContentActionsModel.cpp
  scene/Content.cpp
  scene/ContentFactory.cpp
  scene/ContentType.cpp
  scene/ContextMenu.cpp
  scene/CountdownStatus.cpp
  scene/DisplayGroup.cpp
  scene/DynamicTextureContent.cpp
  scene/ErrorContent.cpp
  scene/KeyboardState.cpp
  scene/Markers.cpp
  scene/MultiChannelContent.cpp
  scene/Options.cpp
  scene/PixelStreamContent.cpp
  scene/Rectangle.cpp
  scene/Scene.cpp
  scene/ScreenLock.cpp
  scene/Surface.cpp
  scene/SVGContent.cpp
  scene/TextureContent.cpp
  scene/VectorialContent.cpp
  scene/Window.cpp
  thumbnail/DefaultThumbnailGenerator.cpp
  thumbnail/FolderThumbnailGenerator.cpp
  thumbnail/ImageThumbnailGenerator.cpp
  thumbnail/StateThumbnailGenerator.cpp
  thumbnail/StreamThumbnailGenerator.cpp
  thumbnail/thumbnail.cpp
  thumbnail/ThumbnailGenerator.cpp
  thumbnail/ThumbnailGeneratorFactory.cpp
  thumbnail/ThumbnailProvider.cpp
)

if(TIDE_ENABLE_WEBBROWSER_SUPPORT)
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

set(TIDECORE_INCLUDE_NAME tide/core)
common_library(TideCore)

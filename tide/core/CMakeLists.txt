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
else()
  list(APPEND TIDECORE_PUBLIC_HEADERS
    data/SVGQtGpuBackend.h
  )
  list(APPEND TIDECORE_SOURCES
    data/SVGQtGpuBackend.cpp
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
  QmlTypeRegistration.h
  StatePreview.h
  types.h
  ui.h
  configuration/Configuration.h
  configuration/ConfigurationWriter.h
  configuration/Process.h
  configuration/Screen.h
  configuration/SurfaceConfig.h
  configuration/SurfaceConfigValidator.h
  configuration/XmlParser.h
  data/Image.h
  data/ImageReader.h
  data/QtImage.h
  data/StreamImage.h
  data/SVG.h
  data/SVGBackend.h
  data/YUVImage.h
  json/json.h
  json/serialization.h
  json/templates.h
  multitouch/DoubleTapDetector.h
  multitouch/MathUtils.h
  multitouch/MultitouchArea.h
  multitouch/PanDetector.h
  multitouch/PinchDetector.h
  multitouch/SwipeDetector.h
  multitouch/TapAndHoldDetector.h
  multitouch/TapDetector.h
  network/LocalBarrier.h
  network/MPIChannel.h
  network/MPIContext.h
  network/MPIHeader.h
  network/MPINospin.h
  network/NetworkBarrier.h
  network/ReceiveBuffer.h
  network/SharedNetworkBarrier.h
  scene/Background.h
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
  scene/ImageContent.h
  scene/VectorialContent.h
  scene/Window.h
  scene/ZoomHelper.h
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
  utils/CommandLineParser.h
  utils/geometry.h
  utils/IterableSmartPtrCollection.h
  utils/log.h
  utils/qml.h
  utils/yuv.h
)

list(APPEND TIDECORE_SOURCES
  MetaTypeRegistration.cpp
  StatePreview.cpp
  ui.cpp
  configuration/Configuration.cpp
  configuration/ConfigurationWriter.cpp
  configuration/SurfaceConfig.cpp
  configuration/SurfaceConfigValidator.cpp
  configuration/XmlParser.cpp
  data/ImageReader.cpp
  data/QtImage.cpp
  data/StreamImage.cpp
  data/SVG.cpp
  data/YUVImage.cpp
  json/json.cpp
  json/serialization.cpp
  multitouch/DoubleTapDetector.cpp
  multitouch/MathUtils.cpp
  multitouch/MultitouchArea.cpp
  multitouch/PanDetector.cpp
  multitouch/PinchDetector.cpp
  multitouch/SwipeDetector.cpp
  multitouch/TapAndHoldDetector.cpp
  multitouch/TapDetector.cpp
  network/LocalBarrier.cpp
  network/MPIChannel.cpp
  network/MPIContext.cpp
  network/MPINospin.cpp
  network/SharedNetworkBarrier.cpp
  resources/core.qrc
  scene/Background.cpp
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
  scene/ImageContent.cpp
  scene/VectorialContent.cpp
  scene/Window.cpp
  scene/ZoomHelper.cpp
  utils/CommandLineParser.cpp
  utils/geometry.cpp
  utils/log.cpp
  utils/yuv.cpp
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

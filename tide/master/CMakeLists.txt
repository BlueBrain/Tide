
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDEMASTER_LINK_LIBRARIES
  PUBLIC
    TideCore
    Qt5::Widgets
)

list(APPEND TIDEMASTER_PUBLIC_HEADERS
  BackgroundWidget.h
  ContentLoader.h
  DisplayGroupController.h
  DisplayGroupListWidget.h
  DisplayGroupView.h
  FileCommandHandler.h
  localstreamer/AsyncImageLoader.h
  localstreamer/CommandLineOptions.h
  localstreamer/DockPixelStreamer.h
  localstreamer/DockToolbar.h
  localstreamer/Pictureflow.h
  localstreamer/PixelStreamerFactory.h
  localstreamer/PixelStreamer.h
  localstreamer/PixelStreamerLauncher.h
  localstreamer/PixelStreamerType.h
  MasterApplication.h
  MasterConfiguration.h
  MasterToForkerChannel.h
  MasterWindow.h
  MultitouchArea.h
  PixelStreamWindowManager.h
  ProcessForker.h
  QmlTypeRegistration.h
  qmlUtils.h
  SessionCommandHandler.h
  State.h
  StateSerializationHelper.h
)

list(APPEND TIDEMASTER_SOURCES
  BackgroundWidget.cpp
  ContentLoader.cpp
  ContentWindowListWidgetItem.cpp
  DisplayGroupController.cpp
  DisplayGroupListWidget.cpp
  DisplayGroupView.cpp
  FileCommandHandler.cpp
  localstreamer/AsyncImageLoader.cpp
  localstreamer/CommandLineOptions.cpp
  localstreamer/DockPixelStreamer.cpp
  localstreamer/DockToolbar.cpp
  localstreamer/Pictureflow.cpp
  localstreamer/PixelStreamer.cpp
  localstreamer/PixelStreamerFactory.cpp
  localstreamer/PixelStreamerLauncher.cpp
  localstreamer/PixelStreamerType.cpp
  MasterApplication.cpp
  MasterConfiguration.cpp
  MasterToForkerChannel.cpp
  MasterWindow.cpp
  MultitouchArea.cpp
  PixelStreamWindowManager.cpp
  ProcessForker.cpp
  SessionCommandHandler.cpp
  State.cpp
  StateSerializationHelper.cpp
  resources/master.qrc
)

if(TARGET Qt5::WebKitWidgets)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    localstreamer/WebkitAuthenticationHelper.h
    localstreamer/WebkitHtmlSelectReplacer.h
    localstreamer/WebkitPixelStreamer.h
    WebbrowserCommandHandler.h
    WebbrowserWidget.h
  )

  list(APPEND TIDEMASTER_SOURCES
    localstreamer/WebkitAuthenticationHelper.cpp
    localstreamer/WebkitHtmlSelectReplacer.cpp
    localstreamer/WebkitPixelStreamer.cpp
    WebbrowserCommandHandler.cpp
    WebbrowserWidget.cpp
  )

  list(APPEND TIDEMASTER_LINK_LIBRARIES PRIVATE Qt5::WebKitWidgets)
endif()

if(TIDE_ENABLE_TUIO_TOUCH_LISTENER)
  list(APPEND TIDEMASTER_HEADERS MultiTouchListener.h)
  list(APPEND TIDEMASTER_SOURCES MultiTouchListener.cpp)
  list(APPEND TIDEMASTER_LINK_LIBRARIES TUIO ${X11_LIBRARIES})
endif()

set(TIDEMASTER_INCLUDE_NAME tide/master)
common_library(TideMaster)

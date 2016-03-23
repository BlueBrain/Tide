
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDEMASTER_LINK_LIBRARIES
  PUBLIC
    TideCore
    Qt5::Widgets
  PRIVATE
    Qt5::WebKitWidgets
)

list(APPEND TIDEMASTER_PUBLIC_HEADERS
  ContentLoader.h
  DisplayGroupController.h
  FileCommandHandler.h
  localstreamer/CommandLineOptions.h
  localstreamer/DockToolbar.h
  localstreamer/PixelStreamerFactory.h
  localstreamer/PixelStreamerType.h
  MasterConfiguration.h
  ProcessForker.h
  QmlTypeRegistration.h
  qmlUtils.h
  SessionCommandHandler.h
  State.h
  StateSerializationHelper.h
)

list(APPEND TIDEMASTER_MOC_PUBLIC_HEADERS
  BackgroundWidget.h
  DisplayGroupListWidget.h
  DisplayGroupView.h
  localstreamer/AsyncImageLoader.h
  localstreamer/DockPixelStreamer.h
  localstreamer/Pictureflow.h
  localstreamer/PixelStreamer.h
  localstreamer/PixelStreamerLauncher.h
  localstreamer/WebkitAuthenticationHelper.h
  localstreamer/WebkitHtmlSelectReplacer.h
  localstreamer/WebkitPixelStreamer.h
  MasterApplication.h
  MasterToForkerChannel.h
  MasterWindow.h
  MultitouchArea.h
  PixelStreamWindowManager.h
  WebbrowserCommandHandler.h
  WebbrowserWidget.h
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
  localstreamer/WebkitAuthenticationHelper.cpp
  localstreamer/WebkitHtmlSelectReplacer.cpp
  localstreamer/WebkitPixelStreamer.cpp
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
  WebbrowserCommandHandler.cpp
  WebbrowserWidget.cpp
)

if(ENABLE_TUIO_TOUCH_LISTENER)
  list(APPEND TIDEMASTER_MOC_HEADERS MultiTouchListener.h)
  list(APPEND TIDEMASTER_SOURCES MultiTouchListener.cpp)
  list(APPEND TIDEMASTER_LINK_LIBRARIES TUIO ${X11_LIBRARIES})
endif()

set(TIDEMASTER_RESOURCES resources/master.qrc)

set(TIDEMASTER_INCLUDE_NAME tide/master)
common_library(TideMaster)

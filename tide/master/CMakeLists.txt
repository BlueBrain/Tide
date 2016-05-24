
# Copyright (c) 2013-2016, EPFL/Blue Brain Project
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
  localstreamer/CommandLineOptions.h
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
  localstreamer/CommandLineOptions.cpp
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
  State.cpp
  StateSerializationHelper.cpp
  resources/master.qrc
)

if(TARGET Qt5::WebKitWidgets)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    localstreamer/WebkitAuthenticationHelper.h
    localstreamer/WebkitHtmlSelectReplacer.h
    localstreamer/WebkitPixelStreamer.h
    WebbrowserWidget.h
  )

  list(APPEND TIDEMASTER_SOURCES
    localstreamer/WebkitAuthenticationHelper.cpp
    localstreamer/WebkitHtmlSelectReplacer.cpp
    localstreamer/WebkitPixelStreamer.cpp
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

if(TIDE_ENABLE_REST_INTERFACE)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    rest/RestCommand.h
    rest/RestInterface.h
    rest/StaticContent.h
  )
  list(APPEND TIDEMASTER_SOURCES
    rest/RestCommand.cpp
    rest/RestInterface.cpp
    rest/StaticContent.cpp
  )
  # Servus PUBLIC because servus::Serializable is a base class of public headers
  list(APPEND TIDEMASTER_LINK_LIBRARIES PUBLIC Servus PRIVATE ZeroEQ)
endif()

common_library(TideMaster)

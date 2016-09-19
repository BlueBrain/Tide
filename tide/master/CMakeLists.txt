
# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDEMASTER_LINK_LIBRARIES
  PUBLIC
    TideCore
    Qt5::Widgets
  PRIVATE
    Qt5::Concurrent
    DeflectQt
)

list(APPEND TIDEMASTER_PUBLIC_HEADERS
  BackgroundWidget.h
  ContentLoader.h
  DisplayGroupController.h
  DisplayGroupListWidget.h
  DisplayGroupView.h
  localstreamer/CommandLineOptions.h
  localstreamer/PixelStreamerLauncher.h
  localstreamer/PixelStreamerType.h
  localstreamer/QmlKeyInjector.h
  LoggingUtility.h
  MasterApplication.h
  MasterConfiguration.h
  MasterToForkerChannel.h
  MasterWindow.h
  multitouch/DoubleTapDetector.h
  multitouch/MathUtils.h
  multitouch/PanDetector.h
  multitouch/PinchDetector.h
  multitouch/SwipeDetector.h
  multitouch/TapAndHoldDetector.h
  multitouch/TapDetector.h
  MultitouchArea.h
  PixelStreamWindowManager.h
  ProcessForker.h
  QmlTypeRegistration.h
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
  localstreamer/PixelStreamerLauncher.cpp
  localstreamer/PixelStreamerType.cpp
  localstreamer/QmlKeyInjector.cpp
  LoggingUtility.cpp
  MasterApplication.cpp
  MasterConfiguration.cpp
  MasterToForkerChannel.cpp
  MasterWindow.cpp
  multitouch/DoubleTapDetector.cpp
  multitouch/MathUtils.cpp
  multitouch/PanDetector.cpp
  multitouch/PinchDetector.cpp
  multitouch/SwipeDetector.cpp
  multitouch/TapAndHoldDetector.cpp
  multitouch/TapDetector.cpp
  MultitouchArea.cpp
  PixelStreamWindowManager.cpp
  ProcessForker.cpp
  State.cpp
  StateSerializationHelper.cpp
  resources/master.qrc
)

if(TARGET Qt5::WebEngine)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    WebbrowserWidget.h
  )
  list(APPEND TIDEMASTER_SOURCES
    WebbrowserWidget.cpp
  )
elseif(TARGET Qt5::WebKitWidgets)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    localstreamer/PixelStreamerFactory.h
    localstreamer/PixelStreamer.h
    localstreamer/WebkitAuthenticationHelper.h
    localstreamer/WebkitHtmlSelectReplacer.h
    localstreamer/WebkitPixelStreamer.h
    WebbrowserWidget.h
  )
  list(APPEND TIDEMASTER_SOURCES
    localstreamer/PixelStreamerFactory.cpp
    localstreamer/WebkitAuthenticationHelper.cpp
    localstreamer/WebkitHtmlSelectReplacer.cpp
    localstreamer/WebkitPixelStreamer.cpp
    WebbrowserWidget.cpp
  )
  list(APPEND TIDEMASTER_LINK_LIBRARIES PRIVATE Qt5::WebKitWidgets)
endif()

if(TIDE_ENABLE_TUIO_TOUCH_LISTENER)
  list(APPEND TIDEMASTER_HEADERS MultitouchListener.h)
  list(APPEND TIDEMASTER_SOURCES MultitouchListener.cpp)
  list(APPEND TIDEMASTER_LINK_LIBRARIES TUIO ${X11_LIBRARIES})
endif()

set(TIDEMASTER_INCLUDE_NAME tide/master)

if(TIDE_ENABLE_REST_INTERFACE)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    rest/JsonOptions.h
    rest/RestCommand.h
    rest/RestInterface.h
    rest/RestLogger.h
    rest/RestServer.h
    rest/StaticContent.h
  )
  list(APPEND TIDEMASTER_SOURCES
    rest/JsonOptions.cpp
    rest/RestCommand.cpp
    rest/RestInterface.cpp
    rest/RestLogger.cpp
    rest/RestServer.cpp
    rest/StaticContent.cpp
  )
  # Servus PUBLIC because servus::Serializable is a base class of public headers
  list(APPEND TIDEMASTER_LINK_LIBRARIES PUBLIC Servus PRIVATE ZeroEQ)
endif()

common_library(TideMaster)

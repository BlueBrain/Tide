# Copyright (c) 2013-2018, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

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
  CommandLineParameters.h
  ContentLoader.h
  control/AutomaticLayout.h
  control/CanvasNode.h
  control/CanvasTree.h
  control/ContentController.h
  control/ContentWindowController.h
  control/DisplayGroupController.h
  control/LayoutEngine.h
  control/LayoutPolicy.h
  control/MultiChannelWindowController.h
  control/PixelStreamController.h
  control/SceneController.h
  control/ZoomController.h
  FileInfoHelper.h
  InactivityTimer.h
  localstreamer/HtmlSelectReplacer.h
  localstreamer/CommandLineOptions.h
  localstreamer/PixelStreamerLauncher.h
  localstreamer/ProcessForker.h
  localstreamer/QmlKeyInjector.h
  LoggingUtility.h
  MasterApplication.h
  MasterDisplayGroupRenderer.h
  MasterSurfaceRenderer.h
  multitouch/DoubleTapDetector.h
  multitouch/MathUtils.h
  multitouch/MultitouchArea.h
  multitouch/PanDetector.h
  multitouch/PinchDetector.h
  multitouch/SwipeDetector.h
  multitouch/TapAndHoldDetector.h
  multitouch/TapDetector.h
  network/MasterFromWallChannel.h
  network/MasterToForkerChannel.h
  network/MasterToWallChannel.h
  PixelStreamWindowManager.h
  QmlTypeRegistration.h
  ScreenshotAssembler.h
  State.h
  StateSerializationHelper.h
  ui/BackgroundWidget.h
  ui/ContentWindowListWidgetItem.h
  ui/DisplayGroupListWidget.h
  ui/MasterQuickView.h
  ui/BackgroundWidget.h
  ui/MasterWindow.h
)

list(APPEND TIDEMASTER_SOURCES
  CommandLineParameters.cpp
  ContentLoader.cpp
  control/AutomaticLayout.cpp
  control/CanvasNode.cpp
  control/CanvasTree.cpp
  control/ContentController.cpp
  control/ContentWindowController.cpp
  control/DisplayGroupController.cpp
  control/LayoutEngine.cpp
  control/LayoutPolicy.cpp
  control/MultiChannelWindowController.cpp
  control/PixelStreamController.cpp
  control/SceneController.cpp
  control/ZoomController.cpp
  InactivityTimer.cpp
  localstreamer/HtmlSelectReplacer.cpp
  localstreamer/CommandLineOptions.cpp
  localstreamer/PixelStreamerLauncher.cpp
  localstreamer/ProcessForker.cpp
  localstreamer/QmlKeyInjector.cpp
  LoggingUtility.cpp
  MasterApplication.cpp
  MasterDisplayGroupRenderer.cpp
  MasterSurfaceRenderer.cpp
  multitouch/DoubleTapDetector.cpp
  multitouch/MathUtils.cpp
  multitouch/MultitouchArea.cpp
  multitouch/PanDetector.cpp
  multitouch/PinchDetector.cpp
  multitouch/SwipeDetector.cpp
  multitouch/TapAndHoldDetector.cpp
  multitouch/TapDetector.cpp
  network/MasterFromWallChannel.cpp
  network/MasterToForkerChannel.cpp
  network/MasterToWallChannel.cpp
  PixelStreamWindowManager.cpp
  ScreenshotAssembler.cpp
  State.cpp
  StateSerializationHelper.cpp
  resources/master.qrc
  ui/BackgroundWidget.cpp
  ui/ContentWindowListWidgetItem.cpp
  ui/DisplayGroupListWidget.cpp
  ui/MasterQuickView.cpp
  ui/MasterWindow.cpp
)

if(TIDE_ENABLE_WEBBROWSER_SUPPORT)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    ui/WebbrowserWidget.h
  )
  list(APPEND TIDEMASTER_SOURCES
    ui/WebbrowserWidget.cpp
  )
endif()

if(TIDE_ENABLE_PDF_SUPPORT)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    control/PDFController.h
  )
  list(APPEND TIDEMASTER_SOURCES
    control/PDFController.cpp
  )
endif()

if(TIDE_ENABLE_PLANAR_CONTROLLER)
  list(APPEND TIDEMASTER_HEADERS MultiScreenController.h)
  list(APPEND TIDEMASTER_SOURCES MultiScreenController.cpp)
  list(APPEND TIDEMASTER_HEADERS PlanarController.h)
  list(APPEND TIDEMASTER_SOURCES PlanarController.cpp)
  list(APPEND TIDEMASTER_SOURCES ScreenController.h)
  list(APPEND TIDEMASTER_SOURCES ScreenControllerFactory.h)
  list(APPEND TIDEMASTER_SOURCES ScreenControllerFactory.cpp)
  list(APPEND TIDEMASTER_LINK_LIBRARIES PRIVATE Qt5::SerialPort)
endif()

set(TIDEMASTER_INCLUDE_NAME tide/master)

if(TIDE_ENABLE_REST_INTERFACE)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    rest/AppRemoteController.h
    rest/FileBrowser.h
    rest/FileReceiver.h
    rest/HtmlContent.h
    rest/jsonschema.h
    rest/RestInterface.h
    rest/RestServer.h
    rest/ThumbnailCache.h
    rest/SceneRemoteController.h
    rest/serialization.h
    rest/templates.h
  )
  list(APPEND TIDEMASTER_SOURCES
    rest/AppRemoteController.cpp
    rest/FileBrowser.cpp
    rest/FileReceiver.cpp
    rest/HtmlContent.cpp
    rest/jsonschema.cpp
    rest/RestInterface.cpp
    rest/RestServer.cpp
    rest/ThumbnailCache.cpp
    rest/SceneRemoteController.cpp
    rest/serialization.cpp
  )
  list(APPEND TIDEMASTER_LINK_LIBRARIES PRIVATE Rockets)
endif()

common_library(TideMaster)

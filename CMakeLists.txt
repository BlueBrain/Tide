# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Tide VERSION 1.5.0)
set(Tide_VERSION_ABI 1)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/common)
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/CMake/common/Common.cmake)
  message(FATAL_ERROR "CMake/common missing, run: git submodule update --init")
endif()

set(TIDE_INCLUDE_NAME tide)
set(TIDE_DESCRIPTION "Tiled Interactive DisplayWall Environment")
set(TIDE_MAINTAINER "Blue Brain Project <bbp-open-source@googlegroups.com>")
set(TIDE_VENDOR "Blue Brain Project")
set(TIDE_LICENSE BSD)
set(TIDE_DEB_DEPENDS python imagemagick libopenmpi-dev openmpi-bin
  libboost-program-options-dev libboost-serialization-dev libboost-test-dev
  qtbase5-private-dev qtdeclarative5-dev libqt5serialport5-dev libqt5svg5-dev
  libqt5webkit5-dev libqt5xmlpatterns5-dev libqt5x11extras5-dev
  qml-module-qtquick-controls
  libpoppler-glib-dev libcairo2-dev libpoppler-qt5-dev librsvg2-dev libtiff5-dev
  libavutil-dev libavformat-dev libavcodec-dev libswscale-dev)
set(TIDE_PORT_DEPENDS boost ffmpeg mpich-devel qt5)

include(Common)

set(_deflect_min_version 0.14)
set(_rockets_min_version 0.1)

common_find_package(Boost 1.56 REQUIRED COMPONENTS program_options serialization
  unit_test_framework)
common_find_package(Deflect ${_deflect_min_version} REQUIRED)
common_find_package(MPI REQUIRED)
common_find_package(Qt5Concurrent REQUIRED)
common_find_package(Qt5Core REQUIRED)
common_find_package(Qt5Gui REQUIRED COMPONENTS Private)
if(NOT TARGET Qt5::QTuioTouchPlugin)
  message(WARNING "Qt5::QTuioTouchPlugin not found, touch may be unavailable")
endif()
common_find_package(Qt5Network REQUIRED)
common_find_package(Qt5Qml REQUIRED)
common_find_package(Qt5Quick 5.4 REQUIRED)
common_find_package(Qt5SerialPort)
if(TARGET Qt5::SerialPort)
  option(TIDE_ENABLE_PLANAR_CONTROLLER "Enable Planar screen control" ON)
endif()
common_find_package(Qt5Svg REQUIRED)
common_find_package(Qt5WebEngine 5.6)
if(TARGET Qt5::WebEngine)
  option(TIDE_ENABLE_WEBBROWSER_SUPPORT "Enable webbrowser support" ON)
endif()
common_find_package(Qt5Widgets REQUIRED)
common_find_package(Qt5XmlPatterns REQUIRED)
common_find_package(Qt5Xml REQUIRED)
if(LINUX)
  common_find_package(Qt5X11Extras REQUIRED)
endif()
common_find_package(RSVG MODULE librsvg-2.0 2.36.2)
common_find_package(Threads REQUIRED)
common_find_package(TIFF)
common_find_package(VirtualKeyboard)
common_find_package(X11) # for swap sync unit tests

common_find_package(FFMPEG)
if(FFMPEG_FOUND)
  option(TIDE_ENABLE_MOVIE_SUPPORT "Enable FFMPEG movie support" ON)
endif()

common_find_package(cairo)
common_find_package(poppler-glib)
if(CAIRO_FOUND AND POPPLER-GLIB_FOUND)
  option(TIDE_ENABLE_PDF_SUPPORT "Enable Pdf support using Poppler-glib + Cairo" ON)
else()
  common_find_package(poppler-qt5)
  if(POPPLER-QT5_FOUND)
    option(TIDE_ENABLE_PDF_SUPPORT "Enable Pdf support using Poppler-qt5" ON)
  endif()
endif()

common_find_package(Rockets ${_rockets_min_version})
if(TARGET Rockets)
  option(TIDE_ENABLE_REST_INTERFACE "Enable REST interface using Rockets" ON)
endif()

common_find_package_post()

option(TIDE_IGNORE_MPI_THREADSAFETY "Ignore MPI_THREAD_MULITPLE requirement (Ubuntu 14.04)" OFF)

add_subdirectory(tide)
add_subdirectory(apps)
add_subdirectory(tests)

# Install example files
install(DIRECTORY examples DESTINATION share/Tide COMPONENT examples)

# Packaging (including subprojects)
if(Deflect_IS_SUBPROJECT)
  list(APPEND TIDE_PACKAGE_REPLACES desktopstreamer)
  set(_deflect_deb qtbase5-dev libturbojpeg)
else()
  set(_deflect_deb "desktopstreamer (>= ${_deflect_min_version})")
endif()
if(Rockets_IS_SUBPROJECT)
  list(APPEND TIDE_PACKAGE_REPLACES rockets)
  set(_rockets_deb libwebsockets-dev)
elseif(TIDE_ENABLE_REST_INTERFACE)
  set(_rockets_deb "rockets (>= ${_rockets_min_version})")
endif()
set(TIDE_PACKAGE_DEB_DEPENDS ${_deflect_deb} ${_rockets_deb} ${TIDE_DEB_DEPENDS})
include(CommonCPack) # also includes CommonPackageConfig

# Documentation
set(COMMON_PROJECT_DOMAIN ch.epfl.bluebrain)
include(DoxygenRule)

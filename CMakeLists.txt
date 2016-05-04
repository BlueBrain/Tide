# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Tide VERSION 1.1.0)
set(Tide_VERSION_ABI 1)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
                              ${CMAKE_SOURCE_DIR}/CMake/common)
include(GitExternal)

set(TIDE_INCLUDE_NAME tide)
set(TIDE_DESCRIPTION "A Tiled Interactive DisplayWall Environment")
set(TIDE_MAINTAINER "Blue Brain Project <bbp-open-source@googlegroups.com>")
set(TIDE_LICENSE BSD)
set(TIDE_DEB_DEPENDS libavutil-dev libavformat-dev libavcodec-dev
  libopenmpi-dev openmpi-bin libswscale-dev libxmu-dev libpoppler-qt5-dev
  libboost-date-time-dev libboost-serialization-dev libboost-test-dev
  libboost-program-options-dev libboost-regex-dev libboost-system-dev
  libboost-thread-dev qtbase5-dev libqt5core5a libqt5declarative5
  libqt5script5 libqt5scripttools5 libqt5svg5-dev libqt5webkit5-dev
  libqt5xmlpatterns5-dev libqt5x11extras5-dev qtquick1-5-dev qtscript5-dev
  libpoppler-qt5-dev)
set(TIDE_PORT_DEPENDS qt5-mac boost ffmpeg mpich-devel)

include(Common)

common_find_package(Boost REQUIRED COMPONENTS program_options date_time
  serialization unit_test_framework regex system thread)
common_find_package(Deflect REQUIRED)
common_find_package(FFMPEG REQUIRED)
common_find_package(MPI REQUIRED)
common_find_package(OpenGL REQUIRED)
common_find_package(Qt5Concurrent REQUIRED)
common_find_package(Qt5Core REQUIRED)
common_find_package(Qt5Network REQUIRED)
common_find_package(Qt5OpenGL REQUIRED)
common_find_package(Qt5Qml REQUIRED)
common_find_package(Qt5Svg REQUIRED)
common_find_package(Qt5WebKitWidgets SYSTEM)
common_find_package(Qt5Widgets REQUIRED)
common_find_package(Qt5XmlPatterns REQUIRED)
common_find_package(Qt5Xml REQUIRED)
common_find_package(ZeroEQ)

if(ZeroEQ_FOUND)
  option(TIDE_ENABLE_REST_INTERFACE "Enable Tide's REST interface using ZeroEQ" ON)
endif()

# WAR for Ubuntu 14.04 in CI super-project conditions
common_find_package(Qt5Quick 5.4)
if(NOT Qt5Quick_FOUND)
  message(WARNING "Qt5Quick 5.4 not found, the project will not be built")
  include(CommonPackageConfig)
  return()
endif()

common_find_package(TUIO 1.4 SYSTEM)
if(TUIO_FOUND)
  option(TIDE_ENABLE_TUIO_TOUCH_LISTENER "Enable TUIO touch listener for multi-touch events" ON)
endif()
common_find_package(Poppler COMPONENTS Qt5)
if(POPPLER_FOUND)
  option(TIDE_ENABLE_PDF_SUPPORT "Enable Pdf support using Poppler" ON)
endif()

if(TIDE_ENABLE_TUIO_TOUCH_LISTENER)
  common_find_package(X11 REQUIRED)
else()
  common_find_package(X11) # for webbrowser unit tests
endif()

if(CMAKE_COMPILER_IS_CLANG)
  # Fix for Qt5 - using namespace QtDeclarative in header
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-header-hygiene")
endif()

common_find_package_post()

add_subdirectory(tide)
add_subdirectory(apps)
add_subdirectory(tests)

# Documentation
set(COMMON_PROJECT_DOMAIN ch.epfl.bluebrain)
include(DoxygenRule)
include(CommonPackageConfig)

# Install example files
install(DIRECTORY examples DESTINATION share/Tide COMPONENT examples)

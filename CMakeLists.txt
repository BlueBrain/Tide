# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Tide)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
                              ${CMAKE_SOURCE_DIR}/CMake/common)
include(GitExternal)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_ABI 1)

set(TIDE_INCLUDE_NAME tide)
set(TIDE_DESCRIPTION "A Tiled Interactive DisplayWall Environment")
set(TIDE_MAINTAINER "Blue Brain Project <bbp-open-source@googlegroups.com>")
set(TIDE_LICENSE BSD)

include(Common)

common_package(Boost REQUIRED COMPONENTS program_options date_time serialization
                                         unit_test_framework regex system thread)
common_package(Deflect REQUIRED)
common_package(FFMPEG REQUIRED)
common_package(MPI REQUIRED)
common_package(OpenGL REQUIRED)
common_package(OpenMP)
common_package(Qt5Concurrent REQUIRED)
common_package(Qt5Core REQUIRED)
common_package(Qt5Network REQUIRED)
common_package(Qt5OpenGL REQUIRED)
common_package(Qt5Qml REQUIRED)
common_package(Qt5Svg REQUIRED)
common_package(Qt5WebKitWidgets SYSTEM REQUIRED)
common_package(Qt5Widgets REQUIRED)
common_package(Qt5XmlPatterns REQUIRED)
common_package(Qt5Xml REQUIRED)

# WAR for Ubuntu 14.04 in CI super-project conditions
common_package(Qt5Quick 5.4)
if(NOT Qt5Quick_FOUND)
  message(WARNING "Qt5Quick 5.4 not found, the project will not be built")
  include(CommonPackageConfig)
  return()
endif()

common_package(TUIO 1.4 SYSTEM)
if(TUIO_FOUND)
  option(ENABLE_TUIO_TOUCH_LISTENER "Enable TUIO touch listener for multi-touch events" ON)
endif()
common_package(Poppler COMPONENTS Qt5)
if(POPPLER_FOUND)
  option(ENABLE_PDF_SUPPORT "Enable Pdf support using Poppler" ON)
endif()

if(ENABLE_TUIO_TOUCH_LISTENER)
  common_package(X11 REQUIRED)
else()
  common_package(X11) # for webbrowser unit tests
endif()

if(CMAKE_COMPILER_IS_CLANG)
  # Fix for Qt5 - using namespace QtDeclarative in header
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-header-hygiene")
endif()

common_package_post()

add_subdirectory(tide)
add_subdirectory(apps)
add_subdirectory(tests)

# Documentation
set(COMMON_PROJECT_DOMAIN ch.epfl.bluebrain)
include(DoxygenRule)
include(CommonPackageConfig)

# Install example files
install(DIRECTORY examples DESTINATION share/Tide COMPONENT examples)


# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Generates a mock library used by the cpp unit tests.

set(TIDEMOCK_INCLUDE_NAME tide/mock)
set(TIDEMOCK_OMIT_LIBRARY_HEADER ON)
set(TIDEMOCK_OMIT_CHECK_TARGETS ON)
set(TIDEMOCK_OMIT_INSTALL ON)
common_library(TideMock)

target_sources(TideMock INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/GlobalQtApp.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/glVersion.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/glxDisplay.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imageCompare.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MinimalGlobalQtApp.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MockNetworkBarrier.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MockTouchEvents.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MockScreenController.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/QGuiAppFixture.h"
)

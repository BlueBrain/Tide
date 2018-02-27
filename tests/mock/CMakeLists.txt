# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Generates a mock library used by the cpp unit tests.

set(TIDEMOCK_HEADERS
  GlobalQtApp.h
  glVersion.h
  glxDisplay.h
  imageCompare.h
  MinimalGlobalQtApp.h
  MockNetworkBarrier.h
  MockTouchEvents.h
  QGuiAppFixture.h
)

if(TIDE_ENABLE_PLANAR_CONTROLLER)
  list(APPEND TIDEMOCK_HEADERS MockScreenController.h)
  set(TIDEMOCK_SOURCES MockScreenController.cpp)
endif()

set(TIDEMOCK_LINK_LIBRARIES TideMaster)

set(TIDEMOCK_INCLUDE_NAME tide/mock)
set(TIDEMOCK_OMIT_LIBRARY_HEADER ON)
set(TIDEMOCK_OMIT_CHECK_TARGETS ON)
set(TIDEMOCK_OMIT_INSTALL ON)

common_library(TideMock)

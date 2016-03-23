
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Generates a mock library used by the cpp unit tests.

set(TIDEMOCK_HEADERS
  glxDisplay.h
  glVersion.h
  GlobalQtApp.h
  MinimalGlobalQtApp.h
)

set(TIDEMOCK_INCLUDE_NAME tide/mock)
set(TIDEMOCK_OMIT_LIBRARY_HEADER ON)
common_library(TideMock)

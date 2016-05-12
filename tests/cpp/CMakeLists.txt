
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Generates executables for the unit tests and defines a "tests" make target
# which runs all of them.

# TEST_LIBRARIES is used by CommonCTest.cmake to link tests against them
set(TEST_LIBRARIES
  TideMock
  TideCore
  TideMaster
  TideWall
  ${Boost_LIBRARIES}
  Qt5::OpenGL
)

if(NOT TIDE_ENABLE_REST_INTERFACE)
  set(EXCLUDE_FROM_TESTS core/RESTCommandTests.cpp)
endif()

if(X11_FOUND AND TARGET Qt5::WebKitWidgets)
  list(APPEND TEST_LIBRARIES ${X11_LIBRARIES} Qt5::WebKitWidgets)
else()
  list(APPEND EXCLUDE_FROM_TESTS
    core/WebbrowserTests.cpp
    core/WebkitHtmlSelectReplacementTests.cpp)
endif()

# Recursively compile unit tests for *.cpp files in the current folder,
# linking with TEST_LIBRARIES and excluding EXCLUDE_FROM_TESTS
include(CommonCTest)


# Copyright (c) 2013-2017, EPFL/Blue Brain Project
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
  Qt5::Gui
)

if(TIDE_ENABLE_REST_INTERFACE AND TARGET ZeroEQHTTP)
  list(APPEND TEST_LIBRARIES ZeroEQHTTP Threads::Threads)
else()
  set(EXCLUDE_FROM_TESTS
    core/FileReceiverTests.cpp
    core/JsonOptionsTests.cpp
    core/JsonRpcTests.cpp
    core/JsonSerializationTests.cpp
    core/LoggingUtilityTest.cpp
    core/RestCommandTests.cpp
    core/RestServerTests.cpp
    core/ThumbnailCacheTests.cpp
  )
endif()

if(X11_FOUND AND TARGET Qt5::WebKitWidgets)
  list(APPEND TEST_LIBRARIES ${X11_LIBRARIES} Qt5::WebKitWidgets)
else()
  list(APPEND EXCLUDE_FROM_TESTS
    core/LocalPixelStreamerTests.cpp
    core/WebkitTests.cpp
    core/WebkitHtmlSelectReplacementTests.cpp)
endif()
if(NOT TARGET Qt5::WebEngine AND NOT TARGET Qt5::WebKitWidgets)
  list(APPEND EXCLUDE_FROM_TESTS core/WebbrowserContentTests.cpp)
endif()

# Recursively compile unit tests for *.cpp files in the current folder,
# linking with TEST_LIBRARIES and excluding EXCLUDE_FROM_TESTS
include(CommonCTest)


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
  Threads::Threads
  Qt5::Gui
)

if(TIDE_ENABLE_REST_INTERFACE AND TARGET Rockets)
  list(APPEND TEST_LIBRARIES Rockets)
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

if(NOT TIDE_ENABLE_PLANAR_CONTROLLER)
  list(APPEND EXCLUDE_FROM_TESTS
    core/MultiScreenControllerTests.cpp
    core/ScreenControllerFactoryTests.cpp
  )
endif()

if(X11_FOUND)
  list(APPEND TEST_LIBRARIES ${X11_LIBRARIES})
else()
  list(APPEND EXCLUDE_FROM_TESTS
    core/HardwareSwapGroupTests.cpp
    core/SwapSynchronizerTests.cpp
  )
endif()

if(NOT TIDE_ENABLE_WEBBROWSER_SUPPORT)
  list(APPEND EXCLUDE_FROM_TESTS core/WebbrowserContentTests.cpp)
endif()

# Recursively compile unit tests for *.cpp files in the current folder,
# linking with TEST_LIBRARIES and excluding EXCLUDE_FROM_TESTS
include(CommonCTest)

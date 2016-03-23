
# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)
include_directories(${PROJECT_BINARY_DIR}) ### for config.h

### for override on pre-C++11 compilers
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI ${PROJECT_BINARY_DIR}/config.h")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include config.h")
endif()

add_subdirectory(core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
add_subdirectory(master)
add_subdirectory(wall)

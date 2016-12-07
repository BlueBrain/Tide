# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${PROJECT_SOURCE_DIR}/tide/core)
include_directories(${PROJECT_SOURCE_DIR}/tide/master)
include_directories(${PROJECT_SOURCE_DIR}/tide/wall)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mock)
include_directories(${PROJECT_BINARY_DIR}) ### for config.h

add_subdirectory(cpp)
add_subdirectory(mock)
add_subdirectory(perf)
add_subdirectory(resources)

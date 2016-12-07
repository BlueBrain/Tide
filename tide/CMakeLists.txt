# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)
include_directories(${PROJECT_BINARY_DIR}) ### for config.h

add_subdirectory(core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
add_subdirectory(master)
add_subdirectory(wall)

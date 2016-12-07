# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${PROJECT_SOURCE_DIR}/tide/wall)

set(TIDEWALL_SOURCES main.cpp)
set(TIDEWALL_LINK_LIBRARIES TideWall)
common_application(tideWall)

# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${PROJECT_SOURCE_DIR}/tide/master)

set(TIDEMASTER_SOURCES main.cpp)
set(TIDEMASTER_LINK_LIBRARIES TideMaster)
common_application(tideMaster)

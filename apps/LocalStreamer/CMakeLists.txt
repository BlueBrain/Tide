
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${PROJECT_SOURCE_DIR}/tide/master)

set(LOCALSTREAMER_HEADERS
  Application.h
)

set(LOCALSTREAMER_SOURCES
  main.cpp
  Application.cpp
)

set(LOCALSTREAMER_LINK_LIBRARIES TideMaster Deflect)

common_application(localstreamer)

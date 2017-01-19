
# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${PROJECT_SOURCE_DIR}/tide/master)

set(TIDELOCALSTREAMER_HEADERS
  Application.h
)

set(TIDELOCALSTREAMER_SOURCES
  Application.cpp
  main.cpp
)

set(TIDELOCALSTREAMER_LINK_LIBRARIES TideMaster Deflect)

common_application(tideLocalstreamer NOHELP)

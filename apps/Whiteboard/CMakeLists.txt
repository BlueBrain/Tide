# Copyright (c) 2016-2017, EPFL/Blue Brain Project
#                          Pawel Podhajski <pawel.podhajski@epfl.ch>
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

set(TIDEWHITEBOARD_HEADERS
  Whiteboard.h
)

set(TIDEWHITEBOARD_SOURCES
  Whiteboard.cpp
  main.cpp
  resources.qrc
)

set(TIDEWHITEBOARD_LINK_LIBRARIES DeflectQt TideMaster)

common_application(tideWhiteboard NOHELP)

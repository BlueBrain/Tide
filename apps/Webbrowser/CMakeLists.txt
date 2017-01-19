# Copyright (c) 2016-2017, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

set(TIDEWEBBROWSER_HEADERS
  Webbrowser.h
)

set(TIDEWEBBROWSER_SOURCES
  main.cpp
  Webbrowser.cpp
  resources.qrc
)

set(TIDEWEBBROWSER_LINK_LIBRARIES DeflectQt TideMaster)

common_application(tideWebbrowser NOHELP)

if(TARGET VirtualKeyboard)
  add_dependencies(tideWebbrowser VirtualKeyboard)
endif()

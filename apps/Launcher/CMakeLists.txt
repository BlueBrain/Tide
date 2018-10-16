# Copyright (c) 2016-2018, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

set(TIDELAUNCHER_HEADERS
  FolderModel.h
  Launcher.h
  ModelFilter.h
)

set(TIDELAUNCHER_SOURCES
  FolderModel.cpp
  Launcher.cpp
  main.cpp
  ModelFilter.cpp
  resources.qrc
)

set(TIDELAUNCHER_LINK_LIBRARIES DeflectQt TideMaster)

common_application(tideLauncher NOHELP)

if(TARGET VirtualKeyboard)
  add_dependencies(tideLauncher VirtualKeyboard)
endif()

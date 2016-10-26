
# Copyright (c) 2016, EPFL/Blue Brain Project
#                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

set(TIDELAUNCHER_HEADERS
  Launcher.h
)

set(TIDELAUNCHER_SOURCES
  Launcher.cpp
  main.cpp
  resources.qrc
)

set(TIDELAUNCHER_LINK_LIBRARIES DeflectQt TideMaster)

common_application(tideLauncher)
if(TARGET VirtualKeyboard)
  add_dependencies(tideLauncher VirtualKeyboard)
endif()

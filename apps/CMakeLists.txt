# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${PROJECT_SOURCE_DIR}/tide/core)

add_subdirectory(TideForker)
add_subdirectory(TideMaster)
add_subdirectory(TideWall)
add_subdirectory(Launcher)
add_subdirectory(Whiteboard)
if(TARGET Qt5::WebKitWidgets)
add_subdirectory(LocalStreamer)
endif()
if(TARGET Qt5::WebEngine)
  add_subdirectory(Webbrowser)
endif()

install(PROGRAMS pyramidmaker tide DESTINATION bin COMPONENT apps)

# Copy the startup script and default config to the bin folder to be able to
# launch after building without requiring an install folder and 'make install'
file(COPY pyramidmaker tide DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY ${PROJECT_SOURCE_DIR}/examples/configuration_1x3.xml
  DESTINATION ${CMAKE_BINARY_DIR}/share/Tide/examples
)

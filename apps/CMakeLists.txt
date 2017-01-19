# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include(CommonHelp)

include_directories(${PROJECT_SOURCE_DIR}/tide/core)

add_subdirectory(TideForker)
add_subdirectory(TideMaster)
add_subdirectory(TideWall)
add_subdirectory(Launcher)
add_subdirectory(Whiteboard)

set(_tide_in ${CMAKE_CURRENT_SOURCE_DIR}/tide)
set(_tide_out ${PROJECT_BINARY_DIR}/bin/tide)
add_custom_command(
  DEPENDS ${_tide_in}
  OUTPUT ${_tide_out}
  POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${_tide_in} ${_tide_out}
)
add_custom_target(tide ALL DEPENDS ${_tide_out} tideForker tideMaster tideWall
  tideLauncher tideWhiteboard
)
add_dependencies(${PROJECT_NAME}-all tide)

if(TARGET Qt5::WebKitWidgets)
  add_subdirectory(LocalStreamer)
  add_dependencies(tide tideLocalstreamer)
endif()
if(TARGET Qt5::WebEngine)
  add_subdirectory(Webbrowser)
  add_dependencies(tide tideWebbrowser)
endif()

# Copy the startup script and default config to the bin folder to be able to
# launch after building without requiring an install folder and 'make install'
file(
  COPY ${PROJECT_SOURCE_DIR}/examples/configuration_1x3.xml
  DESTINATION ${PROJECT_BINARY_DIR}/share/Tide/examples
)

file(COPY pyramidmaker DESTINATION ${PROJECT_BINARY_DIR}/bin)
install(PROGRAMS pyramidmaker tide DESTINATION bin COMPONENT apps)

# Generate application help pages for doxygen
common_help(tide LOCATION ${_tide_out})
common_help(pyramidmaker LOCATION ${PROJECT_BINARY_DIR}/bin/pyramidmaker)

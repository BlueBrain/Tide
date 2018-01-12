# Copyright (c) 2013-2017, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include(CommonHelp)

include_directories(${PROJECT_SOURCE_DIR}/tide/core)

add_subdirectory(TideForker)
add_subdirectory(TideMaster)
add_subdirectory(TideWall)
add_subdirectory(Launcher)
add_subdirectory(Whiteboard)

set(scripts tide pyramidify pyramidmaker)
foreach(script ${scripts})
  set(_in ${CMAKE_CURRENT_SOURCE_DIR}/${script})
  set(_out ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${script})
  add_custom_command(
    DEPENDS ${_in} OUTPUT ${_out}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_in} ${_out})
  add_custom_target(${script} ALL DEPENDS ${_out})
  add_dependencies(${PROJECT_NAME}-all ${script})
  install(PROGRAMS ${script} DESTINATION bin COMPONENT apps)

  # Generate application help pages for doxygen
  common_help(${script} LOCATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${script})
endforeach()

add_dependencies(tide tideForker tideMaster tideWall tideLauncher tideWhiteboard)

if(TIDE_ENABLE_WEBBROWSER_SUPPORT)
  add_subdirectory(Webbrowser)
  add_dependencies(tide tideWebbrowser)
endif()

# Copy the default config to the share folder to be able to launch after
# building without requiring an install folder and 'make install'
file(
  COPY ${PROJECT_SOURCE_DIR}/examples/configuration_1x3.xml
  DESTINATION ${PROJECT_BINARY_DIR}/share/Tide/examples
)

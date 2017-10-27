
# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Copy and configure resources used by the tests

# Copy the files needed by the tests to the build directory
set(TEST_RESOURCES
  webgl_interaction.html
  select_test.htm
  configuration.xml
  configuration_default.xml
  configuration_empty.xml
  configuration_minimal.xml
  configuration_stereo.xml
  legacy.dcx
  reference_screenshot.png
  state_v0.dcx
  state_v0.dcxpreview
  state_v0_broken.dcx
  state_v3.dcx
  state_v3_noTitles.dcx
  state_v4.dcx
  wall.png
)

foreach(_file ${TEST_RESOURCES})
  file(COPY ${_file} DESTINATION ${PROJECT_BINARY_DIR}/tests/cpp)
endforeach()

# Performance test resources
set(PERF_TEST_RESOURCES configuration.xml)

foreach(_file ${PERF_TEST_RESOURCES})
  file(COPY ${_file} DESTINATION ${PROJECT_BINARY_DIR}/tests/perf)
endforeach()

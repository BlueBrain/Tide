
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
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
  legacy.dcx
  state_v0.dcx
  state_v0.dcxpreview
  state_v0_broken.dcx
  state_v3.dcx
  state_v3_noTitles.dcx
  state_v4.dcx
  wall.png
  dynamictexture.pyr
)

foreach(FILE ${TEST_RESOURCES})
  file(COPY ${FILE} DESTINATION ${PROJECT_BINARY_DIR}/tests/cpp)
endforeach()

# Performance test resources
set(PERF_TEST_RESOURCES configuration.xml)

foreach(FILE ${PERF_TEST_RESOURCES})
  file(COPY ${FILE} DESTINATION ${PROJECT_BINARY_DIR}/tests/perf)
endforeach()

# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Generates executables for the performance tests.

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)

enable_testing()
add_definitions(-DBOOST_TEST_DYN_LINK) # generates main() for unit tests

if(NOT WIN32) # tests want to be with DLLs on Windows - no rpath
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

set(TEST_LIBRARIES
  TideCore
  ${Boost_LIBRARIES}
)

set(PERF_TEST_SOURCES
  tideBenchmarkMPI.cpp
)

# Create executables but do not add them to the tests target
foreach(FILE ${PERF_TEST_SOURCES})
  string(REGEX REPLACE ".cpp" "" NAME ${FILE})
  string(REGEX REPLACE "[./]" "_" NAME ${NAME})
  source_group(\\ FILES ${FILE})
  add_executable(${NAME} ${FILE})
  common_compile_options(${NAME})
  set_target_properties(${NAME} PROPERTIES FOLDER "Tests")
  target_link_libraries(${NAME} ${TEST_LIBRARIES})
endforeach()

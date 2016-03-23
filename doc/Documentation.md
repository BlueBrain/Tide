Documentation {#documentation}
============

This document describes the basic structure and provides pointers to
auxilary documentation.

## Directory Layout

* [CMake](https://github.com/Eyescale/CMake#readme): subdirectory
  included using git externals. See below for details.
* tide: Contains the main libraries of the project:
  * core: The core library.
  * master: The master library.
  * wall: The wall library
* apps: Applications delivered with the project.
  * Tide: The main application.
  * LocalStreamer: Used by Tideto generate content from separate processes
    (sandboxing).
* tests: Unit tests
* doc: Doxygen and other documentation.
* examples: Example xml configuration files, installed under share/Tide.

## CMakeLists

The top-level CMakeLists is relatively simple due to the delegation of
details into the CMake external. It starts with the project setup which
defines the project name and includes the CMake/common git external.

## CMake

All BBP projects rely on a common
[CMake repository](https://github.com/Eyescale/CMake) which provides
sensible defaults for compilation, documentation and packaging. It is
integrated as a CMake/common subtree as described in the
[Readme](https://github.com/Eyescale/CMake#readme).

## Unit tests

Unit tests are very important. Take a look at the
[coverage report](CoverageReport/index.html).

Introduction {#mainpage}
============

[TOC]

![](wall.png)

Welcome to Tide, a software for Tiled Interactive DisplayWall Environments
providing multi-user touch interaction for collaborations.

Tide is the direct successor of DisplayCluster at
[BlueBrain](https://github.com/BlueBrain/DisplayCluster.git). Amongst several
performance and stability improvements, Tide provides asynchronous data loading,
texture uploading and tile-based content rendering which is based on a
distributed, event-driven QtQuick2 engine.

Tide provides the following features:
* Interactively view, present and collaborate on media such as high-resolution
  images and movies.
* Receive and interact with content streamed from remote sources such as
  laptops / desktops or high-performance remote visualization machines using the
  [Deflect library](https://github.com/BlueBrain/Deflect.git).

A copy of this documentation can be found at
[bluebrain.github.io](http://bluebrain.github.io/).

Please also take a look at the latest @ref ReleaseNotes.

- - -

Tide uses CMake to create a platform-specific build environment.
The following platforms and build environments are tested:

* Linux: Ubuntu 14.04 ((using Qt 5.4.1 from an installer at
  http://download.qt.io) and RHEL 6.6 (using a Qt 5.4.1 and Boost 1.54.0 module)
  (Makefile, Ninja, x64)
* Mac OS X: 10.10 (limited testing) (Makefile, Ninja, homebrew or macports, x64)

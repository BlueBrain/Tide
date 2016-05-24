Changelog {#Changelog}
============

# git master (1.1.0)

* [23](https://github.com/BlueBrain/Tide/pull/23):
  Replaced the Dock with a new Qml Launcher and a side control panel to open it
  [DISCL-313] and [DISCL-316]. The Launcher improves the user experience and
  navigation with a grid view for files and sessions.
* [22](https://github.com/BlueBrain/Tide/pull/22):
  Detect TapAndHold with the mouse
* [21](https://github.com/BlueBrain/Tide/pull/21):
  Bugfix: avoid setting ContentSize to zero on empty deflect::SizeHints event
* [18](https://github.com/BlueBrain/Tide/pull/18):
  Added side buttons to change PDF pages and navigate browser history
* [15](https://github.com/BlueBrain/Tide/pull/15):
  Fix several issues with the touch events, including for Deflect streamers.
* [14](https://github.com/BlueBrain/Tide/pull/14):
  Detect two-fingers swipe gestures [DISCL-356]. Swipe left and right to
  navigate through PDF pages and Webbrowser history.
* [12](https://github.com/BlueBrain/Tide/pull/12):
  Added a basic REST interface with support for open/load/save commands.

# Release 1.0.0 (07-04-2016)

* Initial version, based on
  [DisplayCluster/qml2](https://github.com/BlueBrain/DisplayCluster/tree/qml2)

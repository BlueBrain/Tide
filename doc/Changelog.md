Changelog {#Changelog}
============

# git master (1.1.0)

* [30](https://github.com/BlueBrain/Tide/pull/30):
  The contents can now be shown in fullscreen mode
* [28](https://github.com/BlueBrain/Tide/pull/28):
  Simplify window interaction [DISCL-320].
  This change makes interacting with contents more natural and intuitive:
  - No differences between windows in presentation mode vs. regular mode
  - A glow effect highlights windows which have the focus (capture touch
    events).
  - Regular windows always move by default; tapAndHold to get focus and zoom in.
  - Webbrowsers and interactive streamers always have focus; move using borders.
  - Resizing always preserves aspect ratio by default; tapAndHold any resize
    handle to change the aspect ratio of compatible contents (images
    and webbbrowsers).
  - The one-to-one button (1:1) also resets the zoom level.
  - Other minor improvements.
* [24](https://github.com/BlueBrain/Tide/pull/24):
  Bugfix: Correctly setup Webbrowser proxy from 'http_proxy' ENV VAR
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

Changelog {#Changelog}
============

# Release 1.2 (git master)

* [52](https://github.com/BlueBrain/Tide/pull/52):
  Minor UI changes:
  - Interacting with a window always brings it to the front
  - Close button is also visible for focused windows
  - Improved layout for focused windows
  - Session thumbnails in the Launcher have correct aspect ratio

# Release 1.1 (30-06-2016)

* [48](https://github.com/BlueBrain/Tide/pull/48):
  Prevent users from opening a file multiple times.
* [47](https://github.com/BlueBrain/Tide/pull/47):
  Loading and saving sessions no longer block the wall interface.
* [46](https://github.com/BlueBrain/Tide/pull/46):
  Improved Launcher:
  - The creation and caching of file thumbnails is more efficient.
  - File thumbnails are displayed with their correct aspect ratio.
  Session preview images are of better quality (no longer pixelated).
* [39](https://github.com/BlueBrain/Tide/pull/39):
  Expose usage statistics to the REST interface for monitoring [DISCL-318].
* [38](https://github.com/BlueBrain/Tide/pull/38):
  Fix a segfault that occured when opening grayscale images.
* [36](https://github.com/BlueBrain/Tide/pull/36):
  The options can be modified from the Launcher panel.
* [34](https://github.com/BlueBrain/Tide/pull/34):
  The options can be retrieved and modified through the REST interface.
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

# Release 1.0 (07-04-2016)

* Initial version, based on
  [DisplayCluster/qml2](https://github.com/BlueBrain/DisplayCluster/tree/qml2)

Changelog {#changelog}
============

# Release 1.4 (git master)

* [202](https://github.com/BlueBrain/Tide/pull/202):
  Fixed incorrect count of open windows reported at tide/stats HTTP endpoint.
* [201](https://github.com/BlueBrain/Tide/pull/201):
  The interface unlocks when the screens are turned off.
* [200](https://github.com/BlueBrain/Tide/pull/200):
  Xml configuration simplification:
  - Replaced screenWidth/Height with displayWidth/Height, which refers
    to the resolution of an invidual physical display.
  - Introduced displaysPerScreenX/Y, used to determine the resolution of a
    logical screen (n * displaySize + (n-1) * bezelSize).
  - Renamed some keys:
    -  numTilesWidth/Height -> numScreensX/Y.
    -  mullionWidth/Height -> bezelWidth/Height.
  - Removed bezelsPerScreenX/Y which became redundant.
  Fixes in web interface related to bezel and configuration.
* [195](https://github.com/BlueBrain/Tide/pull/195):
  New features in html interface:
  - Dragged window can be snapped to a bezel.
  - Dragged window can be maximized to the screen size when hovering a bezel.
* [193](https://github.com/BlueBrain/Tide/issues/193):
  Use correct color space for rendering JPEG-compressed pixel streams.
* [189](https://github.com/BlueBrain/Tide/issues/189):
  Support for textures / PixelStreams in OpenGL (bottom-up) row order.
* [179](https://github.com/BlueBrain/Tide/issues/179):
  Allow turning pixel stream compression on/off while streaming (bugfix).
* [178](https://github.com/BlueBrain/Tide/issues/178):
  Hardware swap synchronization for NVidia Quadro sync cards.
* [174](https://github.com/BlueBrain/Tide/pull/174):
  Fix crash with malformed encoded pixel streams, e.g. JPEG quality > 100
* [169](https://github.com/BlueBrain/Tide/issues/169):
  Performance improvement for PixelStreams with small tile size (64x64).
* [167](https://github.com/BlueBrain/Tide/pull/167):
  Tide can be locked to prevent unwanted streams from opening or actions from
  the HTML interface, e.g. during a presentation.
* [162](https://github.com/BlueBrain/Tide/issues/162):
  Movies and pixel streams can be rendered at 60 fps (up from 30 fps).
* [161](https://github.com/BlueBrain/Tide/issues/161):
  Improved automatic layout for focused windows, no longer in one line.
* [151](https://github.com/BlueBrain/Tide/issues/151):
  Added support for playing stereo 3D movies.
* [147](https://github.com/BlueBrain/Tide/issues/147):
  Performance improvement: added support for multiple windows per process.
  The windows can be located on different X11 displays and still share resources
  such as decoded images and movie frames.
  Example gains:
  - On a machine with 3-GPUs, CPU load is reduced by 66% when decoding a
    fullscreen movie.
  - Using passive stereo windows, an (additional) 50% CPU load is saved when
    decoding movies.

# Release 1.3.1 (05-09-2017)

* [177](https://github.com/BlueBrain/Tide/pull/177):
  Fix mipmap filtering for YUV textures.
* [175](https://github.com/BlueBrain/Tide/pull/175):
  Removed window close and resize mouse buttons from master window because they
  interfered with touch events starting from Qt 5.8.
* [173](https://github.com/BlueBrain/Tide/pull/173):
  Fix crash with malformed encoded pixel streams, e.g. JPEG quality > 100.
* [171](https://github.com/BlueBrain/Tide/pull/171):
  Bugfix: in multi-node configurations some windows opened on incorrect hosts.
* [168](https://github.com/BlueBrain/Tide/pull/168):
  Log output includes a timestamp for the event along with the logger id.
* [165](https://github.com/BlueBrain/Tide/pull/165):
  Demo Launcher ported to version 0.5.6 of the RenderingResourceManager.
* [164](https://github.com/BlueBrain/Tide/pull/164):
  Fix problem with Deflect server using system proxy and Qt 5.8.
* [163](https://github.com/BlueBrain/Tide/pull/163):
  Adaptation for Planar controller responding with numerals.

# Release 1.3 (06-06-2017)

* [157](https://github.com/BlueBrain/Tide/pull/153):
  Tide will power off the screens after a configurable period of inactivity.
  User is notified about the imminent shutdown and can interupt it.
* [153](https://github.com/BlueBrain/Tide/pull/153):
  Tide can control Planar displays now. Users can:
  - turn off the screens using power off button in the Launcher
  - turn on the screens by touching the wall
  - check the screens state via REST interface at: tide/stats
* [150](https://github.com/BlueBrain/Tide/issues/150):
  Ignore legacy window titles option from saved sessions.
* [146](https://github.com/BlueBrain/Tide/pull/146):
  Added an optional bezel to the wall in web interface.
* [138](https://github.com/BlueBrain/Tide/pull/138):
  Tide can be released as a Debian package, optionally including subprojects.
* [128](https://github.com/BlueBrain/Tide/pull/128):
  Multiple improvements in html interface:
  - the web interface no longer blocks the application
  - thumbnails are generated asynchronously and cached
  - proper MIME type for served content
* [127](https://github.com/BlueBrain/Tide/pull/127):
  Pixel streams decode faster by directly rendering YUV frames on GPU.
* [124](https://github.com/BlueBrain/Tide/pull/124):
  Faster movie playback through direct rendering of YUV frames on GPU.
  Performance improved by ~60% (10->16fps) on a test 8K "webm" movie.
* [120](https://github.com/BlueBrain/Tide/pull/120):
  User can now upload content to the local file system via the web interface.
* [122](https://github.com/BlueBrain/Tide/pull/122):
  Added passive stereo rendering mode for pixel streams sent by Deflect 0.13.
* [121](https://github.com/BlueBrain/Tide/pull/121):
  Pretty print FFMPEG log messages using Tide log style and reduced verbosity.
* [119](https://github.com/BlueBrain/Tide/pull/119):
  Playback "webm" movies at up to 8K resolution (VP9 codec).
* [117](https://github.com/BlueBrain/Tide/pull/117):
  User can now open content and load/save sessions via the web interface.
* [114](https://github.com/BlueBrain/Tide/pull/114):
  Tide can be now controlled via a web browser. The web interface provides the
  following features:
    - moving, resizing and closing windows
    - focusing and making content fullscreen
  This offers an alternative to the Master window, especially useful when the
  application runs in headless mode.
* [115](https://github.com/BlueBrain/Tide/pull/115):
  Improved command line parameters of all applications.

# Release 1.2.2 (25-01-2017)

* [116](https://github.com/BlueBrain/Tide/pull/116):
  Bugfix: the application could crash when opening a session containing
  multiple video files for the first time.

# Release 1.2.1 (16-12-2016)

* [113](https://github.com/BlueBrain/Tide/pull/113):
  Fixed several issues linked to ZeroEQ with the 1.2 release:
  - The REST interface was constantly polling, causing a 100% idle CPU load.
  - This caused troubles with window animations in headless mode, resulting in
    touch events hitting the background or some other window.
  - cmake -DINSTALL_PACKAGES=1 did not install cppnetlib's dependencies.
  Additionnally, the REST "open" command can be given a directory path to open
  all the contents inside it. This feature was previously inaccessible in
  headless mode.

# Release 1.2 (09-12-2016)

* [111](https://github.com/BlueBrain/Tide/pull/111):
  Multiple bugfixes:
  - Fixed a bug that could cause focused windows to overlap.
  - Corrected the global thread pool size in multi-node configurations.
  - The application no longer blocks on startup if an error occurs, such as the
    chosen REST port being already in use.
* [109](https://github.com/BlueBrain/Tide/pull/109):
  New REST command to capture full resolution screenshot of the display wall.
* [105](https://github.com/BlueBrain/Tide/pull/105):
  Tide can now be started in "headless" mode, without a visible control window.
  This simplifies the deployment on installations where no dedicated control
  monitor is available, where previously the master window had to be run inside
  a VNC server.
* [104](https://github.com/BlueBrain/Tide/pull/104):
  Tide can be controlled from the python BBP viztools.
* [100](https://github.com/BlueBrain/Tide/pull/100):
  The Launcher can generate thumbnails much faster (requires Qt 5.6.3 or 5.7.1).
* [97](https://github.com/BlueBrain/Tide/pull/97):
  Webbrowsers can be saved and restored from sessions and display the page title
  in their title bar.
* [96](https://github.com/BlueBrain/Tide/pull/96):
  Bug fixes in Whiteboard application.
* [95](https://github.com/BlueBrain/Tide/pull/95):
  More consistent and intuitive user experience:
  - Double-tap a window to make it fullscreen
  - To present multiple contents side by side, select them on the Desktop with a
    single tap then press any of their "eye" icons.
  - Tap the background to exit any presentation mode.
  - On the Desktop, tap the background to unselect all windows.
* [93](https://github.com/BlueBrain/Tide/pull/93):
  Added a whiteboard application with support for:
  - Drawing in multiple colors and with different brush sizes
  - Saving an image to a png file
  - Extending/shrinking a canvas on the size change of applications window
* [92](https://github.com/BlueBrain/Tide/pull/92):
  Improvements to the WebEngine webbrowser (Qt 5.6 or later):
  - Plugins enabled (flash)
  - Replace "select" system drop-down lists with html equivalents (as was
    already done for the Webkit webbrowser)
* [90](https://github.com/BlueBrain/Tide/pull/90):
  The fullscreen mode has some new features:
  - Users can now enlarge and pan contents (without the need to use the zoom)
  - A double-tap toggles between *adjusted* and *maximized* fullscreen
    coordinates
  - Movies can be played/paused from the side control bar
  - The keyboard can be opened from the side control bar for streamed contents
  Also, the keyboard icon is no longer shown for the new WebEngine-based
  webbrowsers who use their own integrated keyboard.
* [85](https://github.com/BlueBrain/Tide/pull/85):
  Fix issues affecting Qt 5.7 caused by a conflict with Tide's virtual keyboard.
* [84](https://github.com/BlueBrain/Tide/pull/84):
  Multitouch improvements [DISCL-383]:
  - Tap and DoubleTap gestures work with any number of fingers
  - DoubleTap a window with two fingers to make it fullscreen
  - The changes also fixed bug [80](https://github.com/BlueBrain/Tide/issues/80)
    on Qt >= 5.6, which could lead to an obscure race condition in the QV4
    JavaScript engine when changing the State property of the ContentWindow
    between resizing / moving / none.
  Deflect adaptations [DISCL-386]:
  - Clients now receive PINCH events instead of WHEEL events
  - Clients also receive raw touch events (touch point added, updated, removed)
* [83](https://github.com/BlueBrain/Tide/pull/83):
  Added TIDE_IGNORE_MPI_THREADSAFETY CMake option for Ubuntu 14.04 after #82
* [82](https://github.com/BlueBrain/Tide/pull/82):
  Documentation and build process improvements:
  - Added documentation about building Tide on different platforms
  - Made Tide configure and build out-of-the-box on a fresh install of Ubuntu
    16.04
  - Corrected *tide* startup script syntax for openmpi (the error prevented
    Tide from launching without a hostsfile on Ubuntu 16.04)
  - Increased MPI thread support requirement to MPI_THREAD_MULTIPLE.
    MPI_THREAD_SERIALIZED did not work anymore and is too complex to support.
* [79](https://github.com/BlueBrain/Tide/pull/79):
  Introduce faster and more complete Webbrowser based on Qml WebEngineView
  [DISCL-366]. It is still in an experimental state and has some know issues:
  - With Qt 5.4.1 - 5.5.1, loading some websites cause the browser to crash due
    to SSL errors (observed on Ubuntu 14.04). Also, interaction with Webgl
    contents may not work as expected.
  - Works well with Qt >= 5.6, however these versions currently have other
    [serious stability issues](https://github.com/BlueBrain/Tide/issues/80).
* [78](https://github.com/BlueBrain/Tide/pull/78):
  Fix regresion after #77 (Launcher + Webbrowser no longer started streaming).
* [77](https://github.com/BlueBrain/Tide/pull/77):
  Fix: Pixel streamer window might be black initially [DISCL-382]
* [75](https://github.com/BlueBrain/Tide/pull/75):
  Webbrowser improvements:
  - New address bar to see the current url and enter an address [DISCL-380].
  - The previous/next buttons are only visible if the browsing history has
    corresponding entries.
* [74](https://github.com/BlueBrain/Tide/pull/74):
  Added support for multi-finger pan gestures [DISCL-357].
  Two-finger pan and pinch gestures can also be used simultaneously for zooming
  & moving in an image. External Deflect applications can make use of the new
  EVT_PAN, with the \<key\> field containing the number of touch points.
* [73](https://github.com/BlueBrain/Tide/pull/73):
  Movie improvements:
  - Added a new control bar for movies to the wall interface [DISCL-314].
  - Fixed a bug that caused paused movies to not update correctly when moved to
    a different screen [DISCL-381].
* [72](https://github.com/BlueBrain/Tide/pull/72):
  The Launcher has an additional list-view mode for browsing files and sessions
  [DISCL-376].
* [71](https://github.com/BlueBrain/Tide/pull/71):
  Detect tap-and-hold gestures with any number of fingers.
* [69](https://github.com/BlueBrain/Tide/pull/69):
  Added a new SVG backend based on Cairo + rsvg for better results [DISCL-379].
* [68](https://github.com/BlueBrain/Tide/pull/68):
  Added support for TIFF image pyramids to replace legacy DynamicTextures
  as a way to view very large images [DISCL-360].
* [67](https://github.com/BlueBrain/Tide/pull/67):
  Faster PDF rendering with new Cairo backend and larger tile size [DISCL-373].
* [66](https://github.com/BlueBrain/Tide/pull/66):
  Fix bugs that could cause the application to deadlock on exit [DISCL-375].
* [65](https://github.com/BlueBrain/Tide/pull/65):
  The wall is redrawn every minute when idle so the on-screen clock stays on
  time.
* [64](https://github.com/BlueBrain/Tide/pull/64):
  Faster session loading and fix for FFMPEG thread safety issues.
* [63](https://github.com/BlueBrain/Tide/pull/63):
  Several corrections and bugfixes for session loading. Due to an error in
  [#28](https://github.com/BlueBrain/Tide/pull/28), some sessions saved
  with Tide version 1.1 may include incorrect "_eventReceiversCount" entries
  and will not load anymore. To open them again, manually edit and
  remove all occurencences from the affected .dcx xml file.
* [59](https://github.com/BlueBrain/Tide/pull/59):
  A virtual keyboard is now available for all applications [DISCL-108]
* [57](https://github.com/BlueBrain/Tide/pull/57):
  A position marker is shown on the wall for mouse interactions.
* [56](https://github.com/BlueBrain/Tide/pull/56):
  Support new FFMPEG 3.1 AVCodec API
* [55](https://github.com/BlueBrain/Tide/pull/55):
  Fix potentially [serious bug](https://github.com/BlueBrain/Deflect/pull/115)
  in Deflect server.
* [54](https://github.com/BlueBrain/Tide/pull/54):
  Users can save sessions from the Launcher on the wall [DISCL-256]
* [53](https://github.com/BlueBrain/Tide/pull/53):
  FFMPEG dependency is optional. Movie support can be controlled with CMake
  option TIDE_ENABLE_MOVIE_SUPPORT.
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

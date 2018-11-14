User Guide {#user_guide}
============

This document is a short how-to to start running Tide.

# Usage

Run 'tide' from the bin folder to launch the application.

Some useful command line options:

* \-\-config can be used to launch the application with a specific
  configuration. A default test configuration is used if none is provided.
* \-\-help lists all the available options.

## %Configuration file

A single json configuration file is required to launch the application. It
defines the number of processes to launch, the hosts on which they have to run
and the list of windows that each one of them will display.

On real installations it is often desireable to hide the master control window
entirely. This is achieved by setting "headless": true in the "master" section.

Here is a simple example of the 1x3 setup used by default when launching Tide:

\include examples/configuration_1x3.json

More examples can be found in the examples folder of the source directory, or
installed under ${install_prefix}/share/Tide/examples.

To examine all possible options, a useful trick is to use tideConverter:

> tideConverter --full examples/configuration_1x1.json configuration_full.json

## TIFF image pyramids

Tide can open any image pyramid saved in standard TIFF file format. To convert
an existing image to a TIFF pyramid one can use for instance ImageMagick:
> convert myimage.xyz -monitor -define tiff:tile-geometry=512x512 -compress jpeg 'ptif:myimage.tif'
For more convenience, use the *pyramidify* tool provided by Tide:
> pyramidify myimagefolder/*.png
Which is equivalent to calling manually the *pyramidmaker* script (also provided
by Tide) for each image:
> pyramidmaker myimage.xyz myimage.tif

## Stereo 3D images

Tide displays side-by-side
[.jps](https://en.wikipedia.org/wiki/JPEG#JPEG_Stereoscopic) stereo jpeg images
in 3D.

## Stereo 3D movies

Tide can render 3D movies in side-by-side format if the movie file contains
information about the stereo mode. For example, the webm and mkv containers can
store such metadata. A valid side-by-side stereo movie can be created using
[ffmpeg](https://ffmpeg.org/ffmpeg-formats.html#Metadata):

> ffmpeg -i left_right_clip.mpg -an -c:v libvpx -metadata stereo_mode=left_right -y stereo_clip.webm

Just changing the container for mkv (no re-encoding):

> ffmpeg -i left_right_clip.mp4 -an -c:v copy -metadata stereo_mode=left_right -y left_right_clip.mkv

To check if a movie has the correct stereo metadata, use ffprobe:

> ffprobe stereo_clip.webm
Metadata:
stereo_mode : left_right
Side data:
stereo3d: side by side

## Open ports

Tide listens on the following ports:

* TCP port 1701 - incoming Deflect connections.
* UDP port 3333 - TUIO messages (if using TuioTouch Qt plugin).
* TCP port 8888 - REST interface (if compiled with ZeroEQ support),
                  configurable.

## OSX notes

The following steps might be required to run the application on OSX.

* Allow 'Remote Login' in 'System Preferences' -> Sharing
* Allow passwordless login on the same machine:
  ~~~~~~~~~~~~~{.sh}
  cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
  ~~~~~~~~~~~~~
* Reference your hostname explicitly in /etc/hosts to point to 127.0.0.1.
  Example:
  ~~~~~~~~~~~~~{.sh}
  hostname>  bluebrain077.epfl.ch
  # Add to /etc/hosts:
  127.0.0.1    bluebrain077.epfl.ch
  ~~~~~~~~~~~~~

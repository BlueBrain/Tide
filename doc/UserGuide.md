User Guide {#UserGuide}
============

This document is a short how-to to start running Tide.

# Usage {#usage}

Run 'tide' from the bin folder to launch the application.

Some useful command line options:

* \-\-config can be used to launch the application with a specific
  configuration. A default test configuration is used if none is provided.
* \-\-help lists all the available options.

## Configuration file {#xmlconfig}

A single xml configuration file is required to launch the application. It
defines the number of processes to launch, the hosts on which they have to run
and the list of windows that each one of them will display.

\include examples/configuration_1x3.xml

More examples can be found in the examples folder of the source directory, or
installed under ${install_prefix}/share/Tide/examples.

## Open ports {#openports}

Tide listens on the following ports:

* TCP port 1701 - incoming Deflect connections.
* UDP port 3333 - TUIO messages (if compiled with TUIO multitouch support).
* TCP port 8888 - REST interface (if compiled with ZeroEQ support),
                  configurable.

## OSX notes {#osxnotes}

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

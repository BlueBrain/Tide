# Copyright (c) BBP/EPFL 2011-2016
#                        Stefan.Eilemann@epfl.ch

set(CPACK_PACKAGE_VENDOR "bluebrain.epfl.ch")

set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Tide Application")
set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Development headers and libraries")
set(CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation")

set(CPACK_COMPONENTS_ALL core dev doc)

# Linux Debian specific settings
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-core, libqt4-gui, libqt4-network, libturbojpeg" )
set(CPACK_DEB_COMPONENT_INSTALL ON) # Set this to package only components in CPACK_COMPONENTS_ALL
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1) # Don't make a separate package for each component

include(CommonCPack)

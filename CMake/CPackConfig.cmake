# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

# Info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

mark_as_advanced(LUNCHBOX_PACKAGE_VERSION)

set(CPACK_PACKAGE_VENDOR "www.eyescale.ch")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multi-threaded toolbox library")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_SOURCE_DIR}/doc/RelNotes.md)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/doc/RelNotes.md)

set(CPACK_DEBIAN_BUILD_DEPENDS libboost-regex-dev libboost-serialization-dev
  libavahi-compat-libdnssd-dev libhwloc-dev)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
  "libstdc++6, libboost-regex-dev, libboost-serialization-dev, libavahi-compat-libdnssd1, libhwloc-dev")

set(CPACK_MACPORTS_CATEGORY devel)
set(CPACK_MACPORTS_DEPENDS boost avahi)

include(CommonCPack)

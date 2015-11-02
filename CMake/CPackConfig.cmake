# Copyright (c) 2012-2015 Stefan Eilemann <eile@eyescale.ch>

# Info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

mark_as_advanced(LUNCHBOX_PACKAGE_VERSION)

set(CPACK_PACKAGE_VENDOR "www.eyescale.ch")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/doc/Changelog.md)
set(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/README.md)

set(CPACK_DEBIAN_PACKAGE_DEPENDS
  "libstdc++6, libboost-regex-dev, libboost-serialization-dev, libavahi-compat-libdnssd1, libhwloc-dev, libturbojpeg")

set(CPACK_MACPORTS_CATEGORY devel)
set(CPACK_MACPORTS_DEPENDS boost)

include(CommonCPack)
include(OSSCPack)

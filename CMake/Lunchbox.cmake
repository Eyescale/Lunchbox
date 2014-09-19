

set(LUNCHBOX_PACKAGE_VERSION 1.9)
set(LUNCHBOX_REPO_URL https://github.com/Eyescale/Lunchbox.git)
set(LUNCHBOX_DEPENDS eyescale eyescalePorts hwloc DNSSD avahi-client
  LibJpegTurbo OpenMP MPI leveldb skv REQUIRED Boost)
set(LUNCHBOX_DEB_DEPENDS libboost-regex-dev libboost-serialization-dev
  libboost-filesystem-dev libboost-system-dev libboost-thread-dev
  libhwloc-dev libavahi-client-dev libjpeg-turbo8-dev libturbojpeg libleveldb-dev)
set(LUNCHBOX_PORT_DEPENDS boost)
set(LUNCHBOX_BOOST_COMPONENTS "regex serialization filesystem system thread")
set(LUNCHBOX_MATURITY RD)
set(LUNCHBOX_FORCE_BUILD ${CI_BUILD})
set(LUNCHBOX_SUBPROJECT ON)
if(CI_BUILD_COMMIT)
  set(LUNCHBOX_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(LUNCHBOX_REPO_TAG master)
endif()
set(LUNCHBOX_FORCE_BUILD ON)
set(LUNCHBOX_SOURCE ${CMAKE_SOURCE_DIR})


set(LUNCHBOX_PACKAGE_VERSION 1.9)
set(LUNCHBOX_REPO_URL https://github.com/Eyescale/Lunchbox.git)
set(LUNCHBOX_REPO_TAG master)
set(LUNCHBOX_DEPENDS eyescale eyescalePorts hwloc DNSSD LibJpegTurbo OpenMP MPI
                     leveldb REQUIRED Boost)
set(LUNCHBOX_DEB_DEPENDS libboost-regex-dev libboost-serialization-dev
  libhwloc-dev libavahi-compat-libdnssd-dev libboost-filesystem-dev
  libboost-system-dev libjpeg-turbo8-dev libturbojpeg libleveldb-dev)
set(LUNCHBOX_PORT_DEPENDS boost)
set(LUNCHBOX_BOOST_COMPONENTS "regex serialization filesystem system")
set(LUNCHBOX_MATURITY RD)
set(LUNCHBOX_FORCE_BUILD ${CI_BUILD})

if(CI_BUILD_COMMIT)
  set(LUNCHBOX_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(LUNCHBOX_REPO_TAG master)
endif()
set(LUNCHBOX_FORCE_BUILD ON)
set(LUNCHBOX_SOURCE ${CMAKE_SOURCE_DIR})
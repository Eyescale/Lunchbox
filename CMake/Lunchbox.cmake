
set(LUNCHBOX_PACKAGE_VERSION 1.10)
set(LUNCHBOX_REPO_URL https://github.com/Eyescale/Lunchbox.git)
set(LUNCHBOX_DEPENDS eyescale eyescalePorts hwloc DNSSD avahi-client
  OpenMP MPI leveldb skv REQUIRED Boost)
set(LUNCHBOX_DEB_DEPENDS libboost-regex-dev libboost-serialization-dev
  libboost-filesystem-dev libboost-system-dev libboost-test-dev
  libboost-thread-dev libhwloc-dev avahi-daemon libavahi-client-dev
  libleveldb-dev libopenmpi-dev openmpi-bin)
set(LUNCHBOX_PORT_DEPENDS boost)
set(LUNCHBOX_BOOST_COMPONENTS
  "regex serialization filesystem system thread unit_test_framework")
set(LUNCHBOX_MATURITY RD)
set(LUNCHBOX_SUBPROJECT ON)

if(CI_BUILD_COMMIT)
  set(LUNCHBOX_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(LUNCHBOX_REPO_TAG master)
endif()
set(LUNCHBOX_FORCE_BUILD ON)
set(LUNCHBOX_SOURCE ${CMAKE_SOURCE_DIR})

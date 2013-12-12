

set(LUNCHBOX_PACKAGE_VERSION 1.8.1)
set(LUNCHBOX_REPO_URL https://github.com/Eyescale/Lunchbox.git)
set(LUNCHBOX_REPO_TAG 1.8)
set(LUNCHBOX_DEPENDS eyescale eyescalePorts hwloc DNSSD REQUIRED Boost)
set(LUNCHBOX_DEB_DEPENDS libboost-regex-dev libboost-serialization-dev
  libhwloc-dev libavahi-compat-libdnssd-dev)
set(LUNCHBOX_BOOST_COMPONENTS "regex serialization")

if(CI_BUILD_COMMIT)
  set(LUNCHBOX_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(LUNCHBOX_REPO_TAG master)
endif()
set(LUNCHBOX_FORCE_BUILD ON)
set(LUNCHBOX_SOURCE ${CMAKE_SOURCE_DIR})

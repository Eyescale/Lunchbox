
# Copyright (c) 2012-2016 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Lunchbox VERSION 1.14.0)
set(Lunchbox_VERSION_ABI 7)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
                              ${CMAKE_SOURCE_DIR}/CMake/common)

include(GitExternal)

option(LUNCHBOX_BUILD_V2_API
  "Enable for pure 2.0 API (breaks compatibility with 1.x API)" OFF)

set(DPUT_HOST "ppa:eilemann/equalizer-dev")

set(LUNCHBOX_DESCRIPTION "Multi-threaded C++ toolbox library for all application developers creating high-performance multi-threaded programs.")
set(LUNCHBOX_MAINTAINER "Stefan Eilemann <eile@eyescale.ch>")
set(LUNCHBOX_LICENSE LGPL)
set(LUNCHBOX_DEB_DEPENDS libboost-filesystem-dev libboost-regex-dev
  libboost-serialization-dev libboost-system-dev libboost-test-dev
  libhwloc-dev avahi-daemon libavahi-client-dev
  libleveldb-dev libmemcached-dev libmemcached-tools memcached)
set(LUNCHBOX_PORT_DEPENDS boost libmemcached memcached)
set(COMMON_PROJECT_DOMAIN ch.eyescale)

include(Common)

if(LUNCHBOX_BUILD_V2_API)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES LUNCHBOX_USE_V2_API)
else()
  list(APPEND COMMON_FIND_PACKAGE_DEFINES LUNCHBOX_USE_V1_API)
endif()

common_find_package(Boost REQUIRED COMPONENTS
  filesystem regex serialization system unit_test_framework)
common_find_package(hwloc)
common_find_package(leveldb)
if(NOT COMMON_USE_CXX03)
  common_find_package(libmemcached 1.0.12)
endif()
common_find_package(Servus REQUIRED)
common_find_package_post()

set(LUNCHBOX_DEPENDENT_LIBRARIES Boost)
if(NOT LUNCHBOX_BUILD_V2_API)
  list(APPEND LUNCHBOX_DEPENDENT_LIBRARIES Servus)
endif()

add_definitions("-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

include(pthreads/CMakeLists.txt)

add_subdirectory(lunchbox)
add_subdirectory(tests)
add_subdirectory(doc)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT ${PROJECT_SOURCE_DIR}/README.md)
include(DoxygenRule) # must be after all targets
include(CPackConfig)

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)

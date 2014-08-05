
# Copyright (c) 2012-2014 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Lunchbox)

list(APPEND CMAKE_MODULE_PATH 
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/common)

include(GitExternal)

include(Buildyard)
if(BUILDYARD_STOP)
  return()
endif()

option(LUNCHBOX_BUILD_V2_API
  "Enable for pure 2.0 API (breaks compatibility with 1.x API)" OFF)

set(LAST_RELEASE 1.9.1) # tarball, MacPorts, ...
set(VERSION_MAJOR "1")
set(VERSION_MINOR "9")
set(VERSION_PATCH "1")
set(VERSION_ABI 3)

set(GITTARGETS_RELEASE_BRANCH minor)
set(DPUT_HOST "ppa:eilemann/equalizer-dev")

set(LUNCHBOX_DESCRIPTION "Multi-threaded C++ toolbox library for all application developers creating high-performance multi-threaded programs.")
set(COMMON_PROJECT_DOMAIN ch.eyescale)

include(Common)

list(APPEND CPPCHECK_EXTRA_ARGS -DLB_GCC_4_1_OR_LATER=1)

if(LUNCHBOX_BUILD_V2_API)
  list(APPEND FIND_PACKAGES_DEFINES LUNCHBOX_USE_V2_API)
else()
  list(APPEND FIND_PACKAGES_DEFINES LUNCHBOX_USE_V1_API)
endif()

set(PROJECT_INCLUDE_NAME lunchbox)
include(FindPackages)
set(LUNCHBOX_DEPENDENT_LIBRARIES Boost)

if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(LUNCHBOX_DEFINITIONS ${LUNCHBOX_DEFINITIONS}
  -DLUNCHBOX_BUILD_DIR="${CMAKE_BINARY_DIR}/")
add_definitions(${LUNCHBOX_DEFINITIONS})

if(APPLE)
  # WAR needed when CMAKE_OSX_SYSROOT is set to find /usr/include/turbojpeg.h
  include_directories(SYSTEM /usr/local/../include)
endif()

include(InstallSymlink)
include(pthreads/CMakeLists.txt)

add_subdirectory(lunchbox)
add_subdirectory(tests)
add_subdirectory(doc)

include(DoxygenRule) # must be after all targets
include(CPackConfig)
include(PackageConfig)
include(GNUModules)

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)


# Copyright (c) 2012-2015 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Lunchbox)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
                              ${CMAKE_SOURCE_DIR}/CMake/common)

include(GitExternal)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "12")
set(VERSION_PATCH "0")
set(VERSION_ABI 4)
option(LUNCHBOX_BUILD_V2_API
  "Enable for pure 2.0 API (breaks compatibility with 1.x API)" OFF)
option(LUNCHBOX_USE_MPI "Enable MPI functionality if found" ON)

set(GITTARGETS_RELEASE_BRANCH minor)
set(DPUT_HOST "ppa:eilemann/equalizer-dev")

set(LUNCHBOX_DESCRIPTION "Multi-threaded C++ toolbox library for all application developers creating high-performance multi-threaded programs.")
set(COMMON_PROJECT_DOMAIN ch.eyescale)

include(Common)

if(BLUEGENE)
  option(LUNCHBOX_USE_SKV "Enable SKV functionality if found" ON)
else()
  option(LUNCHBOX_USE_SKV "Enable SKV functionality if found" OFF)
endif()
if(LUNCHBOX_USE_SKV)
  git_subproject(skv https://github.com/BlueBrain/skv.git master)
endif()

if(LUNCHBOX_BUILD_V2_API)
  list(APPEND COMMON_PACKAGE_DEFINES LUNCHBOX_USE_V2_API)
else()
  list(APPEND COMMON_PACKAGE_DEFINES LUNCHBOX_USE_V1_API)
endif()

common_package(Boost REQUIRED COMPONENTS regex serialization filesystem system
                                         thread unit_test_framework)
common_package(hwloc)
common_package(leveldb)
if(LUNCHBOX_USE_MPI)
  common_package(MPI)
endif()
common_package(OpenMP)
common_package(Servus REQUIRED)
common_package(skv)
common_package_post()

set(LUNCHBOX_DEPENDENT_LIBRARIES Boost)
if(NOT LUNCHBOX_BUILD_V2_API)
  list(APPEND LUNCHBOX_DEPENDENT_LIBRARIES Servus)
endif()

add_definitions("-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

include(InstallSymlink)
include(pthreads/CMakeLists.txt)

add_subdirectory(lunchbox)
add_subdirectory(tests)
add_subdirectory(doc)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT ${PROJECT_SOURCE_DIR}/README.md)
include(DoxygenRule) # must be after all targets
include(LunchboxCPack)

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)

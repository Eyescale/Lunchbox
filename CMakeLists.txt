
# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Lunchbox)

option(LUNCHBOX_USE_OPENMP "Test for OpenMP support" ON)
option(LUNCHBOX_FORCE_OPENMP "Force usage of buggy OpenMP implementation" OFF)
mark_as_advanced(LUNCHBOX_FORCE_OPENMP)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

set(RELEASE_VERSION OFF) # OFF or ABI version
set(LAST_RELEASE 1.6.0) # tarball, MacPorts, ...
set(VERSION_MAJOR "1")
set(VERSION_MINOR "7")
set(VERSION_PATCH "0")
set(LUNCHBOX_FEATURES)

if(CMAKE_OSX_DEPLOYMENT_TARGET)
  set(LUNCHBOX_OSX_VERSION ${CMAKE_OSX_DEPLOYMENT_TARGET})
else()
  set(LUNCHBOX_OSX_VERSION 10.6)
endif()
if(RELEASE_VERSION)
  set(LUNCHBOX_DEFINITIONS -DLUNCHBOX_RELEASE)
  set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX${LUNCHBOX_OSX_VERSION}.sdk
      CACHE PATH "" FORCE)
endif()

include(Common)
include(GitTargets)
include(UpdateFile)

if(CMAKE_COMPILER_IS_XLCXX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Disable OpenMP test for xlC Debug build <http://public.kitware.com/Bug/view.php?id=13469>")
  set(LUNCHBOX_USE_OPENMP OFF)
endif()
if(LUNCHBOX_USE_OPENMP)
  find_package(OpenMP)
endif()
if(OPENMP_FOUND)
  if(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION VERSION_LESS 4.3 AND
      NOT LUNCHBOX_FORCE_OPENMP)
    message(STATUS "Disable OpenMP due to bug in gcc ${GCC_COMPILER_VERSION}")
  else()
    set(FIND_PACKAGES_FOUND_EXTRA LUNCHBOX_USE_OPENMP)
    set(LUNCHBOX_FEATURES "${LUNCHBOX_FEATURES} OpenMP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif(OPENMP_FOUND)

set(FIND_PACKAGES_INCLUDE
  "${CMAKE_BINARY_DIR}/include/lunchbox/defines${SYSTEM}.h")
include(FindPackages)

if(HWLOC_FOUND)
  set(LUNCHBOX_FEATURES "${LUNCHBOX_FEATURES} HWLOC")
endif()
if(DNSSD_FOUND)
  set(LUNCHBOX_FEATURES "${LUNCHBOX_FEATURES} zeroconf")
endif()

set(LUNCHBOX_DEFINITIONS ${LUNCHBOX_DEFINITIONS}
    -DLUNCHBOX_BUILD_DIR="${CMAKE_BINARY_DIR}/")
add_definitions(${LUNCHBOX_DEFINITIONS})

include(InstallSymlink)
include(pthreads/CMakeLists.txt)

add_subdirectory(lunchbox)
add_subdirectory(tests)
add_subdirectory(doc)
include(DoxygenRule) # must be after all targets

message(STATUS "Configured with ${CMAKE_BUILD_TYPE}${LUNCHBOX_FEATURES}")

include(CPackConfig)
include(GNUModules)

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)

include(PackageConfig)

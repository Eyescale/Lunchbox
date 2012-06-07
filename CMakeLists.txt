
# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Lunchbox)

option(LUNCHBOX_USE_OPENMP "Test for OpenMP support" ON)
option(LUNCHBOX_USE_HWLOC "Test for hwloc support" ON)
option(LUNCHBOX_FORCE_OPENMP "Force usage of buggy OpenMP implementation" OFF)

mark_as_advanced(LUNCHBOX_FORCE_OPENMP)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

set(RELEASE_VERSION 140) # OFF or 'Mm0' ABI version
set(VERSION_MAJOR "1")
set(VERSION_MINOR "3")
set(VERSION_PATCH "5")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(LUNCHBOX_OSX_VERSION 10.6)
if(RELEASE_VERSION)
  set(LUNCHBOX_DEFINITIONS -DLUNCHBOX_RELEASE)
  set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX${LUNCHBOX_OSX_VERSION}.sdk
      CACHE PATH "" FORCE)
endif()

include(Common)
include(DoxygenSetup)
include(UpdateFile)

set(LUNCHBOX_FEATURES)

find_package(Boost 1.40 COMPONENTS serialization)
if(Boost_FOUND)
  set(LUNCHBOX_USE_BOOST 1)
  link_directories(${Boost_LIBRARY_DIRS})
  include_directories(AFTER SYSTEM ${Boost_INCLUDE_DIRS})
endif()

if(LUNCHBOX_USE_OPENMP)
  find_package(OpenMP)
endif()
if(OPENMP_FOUND)
  if(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION VERSION_LESS 4.3 AND
      NOT LUNCHBOX_FORCE_OPENMP)
    message(STATUS "Disable OpenMP due to bug in gcc ${GCC_COMPILER_VERSION}")
  else()
    set(LUNCHBOX_OPENMP_USED 1)
    set(LUNCHBOX_FEATURES "${LUNCHBOX_FEATURES} OpenMP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif(OPENMP_FOUND)

if(LUNCHBOX_USE_HWLOC)
  find_package(HWLOC)
endif()
if(HWLOC_FOUND)
  include_directories(SYSTEM ${HWLOC_INCLUDE_DIR})
  set(LUNCHBOX_FEATURES "${LUNCHBOX_FEATURES} HWLOC")
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

install(FILES CMake/FindLunchbox.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT dev)
install(FILES CMake/FindLibraryPackage.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH}/FindLunchbox COMPONENT dev)

message(STATUS "Configured with ${CMAKE_BUILD_TYPE}${LUNCHBOX_FEATURES}")

include(CPackConfig)
include(CTest)


# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(LunchBox)

option(LUNCHBOX_USE_OPENMP "Test for OpenMP support" ON)
option(LUNCHBOX_USE_HWLOC "Test for hwloc support" ON)
option(LUNCHBOX_FORCE_OPENMP "Force usage of buggy OpenMP implementation" OFF)

mark_as_advanced(LUNCHBOX_FORCE_OPENMP)

list(APPEND CMAKE_MODULE_PATH ${LunchBox_SOURCE_DIR}/CMake)
list(APPEND CMAKE_PREFIX_PATH ${SystemDrive}:/cygwin/bin)

include(revision)
include(UpdateFile)

set(LUNCHBOX_RELEASE OFF) # OFF or 'Mm0' ABI version
set(VERSION_MAJOR "0")
set(VERSION_MINOR "9")
set(VERSION_PATCH "0")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(LUNCHBOX_OSX_VERSION 10.6)
if(LUNCHBOX_RELEASE)
  set(LUNCHBOX_DEFINITIONS -DLUNCHBOX_RELEASE)
  set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX${LUNCHBOX_OSX_VERSION}.sdk
      CACHE PATH "" FORCE)
  set(VERSION_ABI ${LUNCHBOX_RELEASE})
else()
#  set(VERSION_ABI ${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH})
  set(VERSION_ABI ${VERSION_MINOR}${VERSION_PATCH})
endif()

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  if(LUNCHBOX_RELEASE)
    set(CMAKE_BUILD_TYPE Release)
  else()
    set(CMAKE_BUILD_TYPE Debug)
  endif()
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND NOT MSVC)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation prefix" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LunchBox_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LunchBox_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LunchBox_BINARY_DIR}/lib)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(OUTPUT_INCLUDE_DIR ${LunchBox_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${OUTPUT_INCLUDE_DIR})
include_directories(BEFORE ${CMAKE_SOURCE_DIR} ${OUTPUT_INCLUDE_DIR})

set(LUNCHBOX_FEATURES)

find_package(Doxygen)
if(DOXYGEN_FOUND)
  set (ALL_TARGETS "")
  macro(add_executable _target)
    _add_executable (${_target} ${ARGN})
    set_property (GLOBAL APPEND PROPERTY ALL_TARGETS ${_target})
  endmacro()
endif()

if(MSVC)
  include(UseGnuWin32)
  add_definitions(
    /D_CRT_SECURE_NO_WARNINGS
    /D_SCL_SECURE_NO_WARNINGS
    /wd4244 # conversion from X to Y, possible loss of data
    /wd4800 # forcing value to bool 'true' or 'false' (performance warning)
    )
  # http://www.ogre3d.org/forums/viewtopic.php?f=2&t=60015&start=0
  if(LUNCHBOX_RELEASE)
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR")
  else()
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR /WX")
  endif()
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCXX)
  include(EqCompilerVersion)
  EQ_COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
  if(GCC_COMPILER_VERSION VERSION_LESS 4.1)
    message(ERROR "GCC 4.1 or later required, found ${GCC_COMPILER_VERSION}")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Winvalid-pch -Wnon-virtual-dtor -Wsign-promo -Wshadow -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter -Wno-write-strings")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
  if(NOT WIN32 AND NOT XCODE_VERSION AND NOT LUNCHBOX_RELEASE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()
endif(CMAKE_COMPILER_IS_GNUCXX)

if(WIN32)
  option(Boost_USE_STATIC_LIBS "Use boost static libs" ON)
endif()
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.36 REQUIRED COMPONENTS system regex date_time serialization)
if(Boost_VERSION GREATER 103999)
  set(LUNCHBOX_USE_BOOST_SERIALIZATION 1)
endif()
add_definitions(-DBOOST_ALL_NO_LIB)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(AFTER SYSTEM ${Boost_INCLUDE_DIRS})

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

IF(REDHAT AND CMAKE_SYSTEM_PROCESSOR MATCHES "64$")
  SET(LIB_SUFFIX 64 CACHE STRING "Library directory suffix")
ENDIF()
SET(LIBRARY_DIR lib${LIB_SUFFIX})

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
  add_definitions(-fPIC) # 64bit Linux wants this
endif()

if(APPLE)
  if(_CMAKE_OSX_MACHINE MATCHES "ppc")
    set(LUNCHBOX_OSX_ARCHITECTURES "ppc;ppc64" CACHE STRING
      "LunchBox build architectures")
  else()
    set(LUNCHBOX_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING
      "LunchBox build architectures")
  endif()
  mark_as_advanced(LUNCHBOX_OSX_ARCHITECTURES)
  set(CMAKE_OSX_ARCHITECTURES "${LUNCHBOX_OSX_ARCHITECTURES}")
  message(STATUS "Building LunchBox for ${CMAKE_OSX_ARCHITECTURES}")
endif(APPLE)

set(LUNCHBOX_DEFINITIONS ${LUNCHBOX_DEFINITIONS}
    -DLUNCHBOX_BUILD_DIR="${CMAKE_BINARY_DIR}/")
add_definitions(${LUNCHBOX_DEFINITIONS})

include(InstallSymlink)
include(pthreads/CMakeLists.txt)
add_subdirectory(lunchbox)
add_subdirectory(tests)
add_subdirectory(doc) # must be after all targets

install(FILES CMake/FindLunchbox.cmake CMake/FindHWLOC.cmake
  DESTINATION share/LunchBox/CMake COMPONENT dev)

message(STATUS "Configured with ${CMAKE_BUILD_TYPE}${LUNCHBOX_FEATURES}")

include(CPackConfig)
include(CTest)

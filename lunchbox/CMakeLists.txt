
# Copyright (c) 2012-2014 Stefan Eilemann <eile@eyescale.ch>

include(files.cmake)

list(APPEND LUNCHBOX_LINK_LIBRARIES ${PTHREAD_LIBRARIES} ${Boost_LIBRARIES})
if(WIN32)
  list(APPEND LUNCHBOX_LINK_LIBRARIES ws2_32 mswsock)
endif(WIN32)
if(LINUX)
  list(APPEND LUNCHBOX_LINK_LIBRARIES dl rt)
endif()
if(HWLOC_FOUND)
  list(APPEND LUNCHBOX_LINK_LIBRARIES ${HWLOC_LIBRARIES})
endif(HWLOC_FOUND)
if(DNSSD_FOUND)
  list(APPEND LUNCHBOX_LINK_LIBRARIES ${DNSSD_LIBRARIES})
endif()
if (MPI_FOUND)
  include_directories(${MPI_CXX_INCLUDE_PATH})
  list(APPEND LUNCHBOX_LINK_LIBRARIES ${MPI_LIBRARIES})
endif()

add_definitions(-DEQ_PLUGIN_BUILD -DHAVE_BYTESWAP_H)
if(NOT WIN32 AND NOT CMAKE_COMPILER_IS_XLCXX)
  add_definitions(-DHAVE_BUILTIN_CTZ -DHAVE_BUILTIN_EXPECT)
endif()

common_library(Lunchbox)

source_group(\\ FILES CMakeLists.txt)
source_group(plugin FILES ${PLUGIN_HEADERS})
source_group(compressor FILES ${LUNCHBOX_COMPRESSORS})
source_group(lunchbox FILES ${LUNCHBOX_PUBLIC_HEADERS} ${LUNCHBOX_HEADERS}
  ${LUNCHBOX_SOURCES})

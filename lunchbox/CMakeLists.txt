
# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

include(configure.cmake)
include(files.cmake)

set(LUNCHBOX_LINKLIBS ${PTHREAD_LIBRARIES} ${Boost_SERIALIZATION_LIBRARY})
if(WIN32)
  list(APPEND LUNCHBOX_LINKLIBS ws2_32 mswsock)
endif(WIN32)
if(LINUX)
  list(APPEND LUNCHBOX_LINKLIBS dl rt)
endif()
if(HWLOC_FOUND)
  list(APPEND LUNCHBOX_LINKLIBS ${HWLOC_LIBRARIES})
endif(HWLOC_FOUND)
if(DNSSD_FOUND)
  list(APPEND LUNCHBOX_LINKLIBS ${DNSSD_LIBRARIES})
endif()

source_group(\\ FILES CMakeLists.txt)
source_group(lunchbox FILES ${LUNCHBOX_PUBLIC_HEADERS} ${LUNCHBOX_HEADERS}
  ${LUNCHBOX_SOURCES})

set(LIBRARY Lunchbox)
set(LIB_INCLUDE lunchbox)

add_library(${LIBRARY} SHARED ${LUNCHBOX_ALL})
target_link_libraries(${LIBRARY} ${LUNCHBOX_LINKLIBS})
set_target_properties(${LIBRARY}
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI}
  COMPILE_DEFINITIONS LUNCHBOX_SHARED
)

install(FILES ${LUNCHBOX_PUBLIC_HEADERS}
        DESTINATION include/${LIB_INCLUDE} COMPONENT dev)
install(TARGETS ${LIBRARY}
        ARCHIVE DESTINATION lib COMPONENT dev
        RUNTIME DESTINATION bin COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib)

configure_file(Lunchbox.pc ${CMAKE_CURRENT_BINARY_DIR}/Lunchbox.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Lunchbox.pc
        DESTINATION ${LIBRARY_DIR}/pkgconfig COMPONENT lib)
install(FILES ${OPTIONS_CMAKE} DESTINATION share/Lunchbox/CMake COMPONENT dev)

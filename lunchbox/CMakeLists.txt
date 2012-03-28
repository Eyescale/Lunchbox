
# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

include(configure.cmake)
include(files.cmake)

set(LUNCHBOX_LINKLIB ${PTHREAD_LIBRARIES})
if(WIN32)
  list(APPEND LUNCHBOX_LINKLIB ws2_32 mswsock)
endif(WIN32)
if(LINUX)
  list(APPEND LUNCHBOX_LINKLIB dl rt)
endif()
if(LUNCHBOX_USE_BOOST_SERIALIZATION)
  list(APPEND LUNCHBOX_LINKLIB ${Boost_SERIALIZATION_LIBRARY})
endif()
if(HWLOC_FOUND)
  list(APPEND LUNCHBOX_LINKLIB ${HWLOC_LIBRARIES})
endif(HWLOC_FOUND)

source_group(\\ FILES CMakeLists.txt)
source_group(lunchbox FILES ${LUNCHBOX_PUBLIC_HEADERS} ${LUNCHBOX_HEADERS}
  ${LUNCHBOX_SOURCES})

set(LIBRARY lunchbox)
set(LIB_INCLUDE ${LIBRARY}${VERSION_MAJOR})

add_library(${LIBRARY} SHARED ${LUNCHBOX_ALL})
target_link_libraries(${LIBRARY} ${LUNCHBOX_LINKLIB})
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
install_symlink("include/${LIB_INCLUDE}" "include/${LIBRARY}" dev)

configure_file(lunchbox.in.pc ${CMAKE_CURRENT_BINARY_DIR}/lunchbox.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lunchbox.pc
        DESTINATION ${LIBRARY_DIR}/pkgconfig COMPONENT lib)

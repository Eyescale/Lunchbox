# Copyright (c) 2010-2016, Daniel Pfeifer
#                         Stefan Eilemann <eile@eyescale.ch>
#
# Change this number when adding tests to force a CMake run: 9

include(InstallFiles)
include_directories(${PROJECT_SOURCE_DIR}/tests)

set(TEST_LIBRARIES Lunchbox ${Boost_LIBRARIES})
if(LEVELDB_FOUND)
  list(APPEND TEST_LIBRARIES ${LEVELDB_LIBRARIES})
endif()
if(SKV_FOUND)
  list(APPEND TEST_LIBRARIES fxlogger)
endif()
if(MSVC)
  list(APPEND TEST_LIBRARIES ${PTHREAD_LIBRARIES})
endif()
if(COVERAGE AND TRAVIS)
  list(APPEND EXCLUDE_FROM_TESTS anySerialization.cpp) #timeout in lcov gather
endif()
if(COMMON_USE_CXX03)
  list(APPEND EXCLUDE_FROM_TESTS perf/mutex.cpp)
endif()

set(UNIT_AND_PERF_TESTS persistentMap.cpp)

include(CommonCTest)
install_files(share/Lunchbox/tests FILES ${TEST_FILES} COMPONENT examples)

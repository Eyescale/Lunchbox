# Copyright (c) 2010-2017, Daniel Pfeifer
#                         Stefan Eilemann <eile@eyescale.ch>
#
# Change this number when adding tests to force a CMake run: 1

include(InstallFiles)
include_directories(${PROJECT_SOURCE_DIR}/tests)

set(TEST_LIBRARIES Lunchbox ${Boost_LIBRARIES})
if(MSVC)
  list(APPEND TEST_LIBRARIES ${PTHREAD_LIBRARIES})
endif()
if(COVERAGE AND TRAVIS)
  list(APPEND EXCLUDE_FROM_TESTS anySerialization.cpp) #timeout in lcov gather
endif()

include(CommonCTest)
install_files(share/Lunchbox/tests FILES ${TEST_FILES} COMPONENT examples)

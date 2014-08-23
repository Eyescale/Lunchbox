# Copyright (c) 2010 Daniel Pfeifer
#               2010-2014, Stefan Eilemann <eile@eyescale.ch>
#
# Change this number when adding tests to force a CMake run: 5

include(InstallFiles)

include_directories(${CMAKE_SOURCE_DIR}) # some tests need private headers
set(TEST_LIBRARIES Lunchbox ${Boost_LIBRARIES})
if(COVERAGE AND TRAVIS)
  list(APPEND EXCLUDE_FROM_TESTS anySerialization.cpp) #timeout in lcov gather
endif()

include(CommonCTest)
install_files(share/Lunchbox/tests FILES ${TEST_FILES} COMPONENT examples)

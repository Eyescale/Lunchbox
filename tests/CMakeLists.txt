# Copyright (c) 2010 Daniel Pfeifer
#               2010-2013, Stefan Eilemann <eile@eyescale.ch>
#
# Change this number when adding tests to force a CMake run: 5

include(InstallFiles)

set(TEST_LIBRARIES Lunchbox ${Boost_LIBRARIES})
if(COVERAGE AND TRAVIS)
  set(EXCLUDE_FROM_TESTS anySerialization.cpp) # times out in lcov gather
endif()

include(CommonCTest)
install_files(share/Lunchbox/tests FILES ${TEST_FILES} COMPONENT examples)

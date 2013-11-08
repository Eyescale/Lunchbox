# Copyright (c) 2010 Daniel Pfeifer
#               2010-2013, Stefan Eilemann <eile@eyescale.ch>
#
# Change this number when adding tests to force a CMake run: 1

include(InstallFiles)

include_directories(${CMAKE_SOURCE_DIR}) # some tests need private headers
set(TEST_LIBRARIES Lunchbox ${Boost_LIBRARIES})
include(CommonCTest)

install_files(share/Lunchbox/tests FILES ${TEST_FILES} COMPONENT examples)

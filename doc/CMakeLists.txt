
# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

if(NOT DOXYGEN_FOUND)
  return()
endif()

configure_file(DoxygenLayout.xml ${CMAKE_CURRENT_BINARY_DIR}/DoxygenLayout.xml
  @ONLY)
configure_file(Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

get_property(INSTALL_DEPENDS GLOBAL PROPERTY ALL_TARGETS)
add_custom_target(doxygen_install
  ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
  DEPENDS ${INSTALL_DEPENDS})

add_custom_target(doxygen
  ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS doxygen_install
  COMMENT "Generating API documentation using doxygen" VERBATIM)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/man/man3)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man/man3 DESTINATION man
  COMPONENT man PATTERN "*_docs_*" EXCLUDE)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/html)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
  DESTINATION share/Lunchbox/doc/API COMPONENT doc)
install(FILES ../RELNOTES.txt ../AUTHORS ../ACKNOWLEDGEMENTS ../LGPL
  ../LICENSE.txt DESTINATION share/Lunchbox/doc COMPONENT doc)

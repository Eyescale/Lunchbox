# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(LUNCHBOX_PACKAGE_VERSION "" CACHE STRING "Additional build version for packages")
mark_as_advanced(LUNCHBOX_PACKAGE_VERSION)

if(LINUX)
  set(CPACK_PACKAGE_NAME "LunchBox${VERSION_ABI}")
else()
  set(CPACK_PACKAGE_NAME "LunchBox")
endif()

if(APPLE)
  set(CPACK_PACKAGE_VENDOR "www.eyescale.ch") # PackageMaker doesn't like http://
else()
  set(CPACK_PACKAGE_VENDOR "http://www.eyescale.ch") # deb lintian insists on URL
endif()

set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multi-threaded toolbox library")
#set(CPACK_PACKAGE_DESCRIPTION_FILE ${LunchBox_SOURCE_DIR}/RELNOTES.txt)
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${LunchBox_SOURCE_DIR}/LICENSE.txt)
#set(CPACK_RESOURCE_FILE_README ${LunchBox_SOURCE_DIR}/RELNOTES.txt)

if(LUNCHBOX_PACKAGE_VERSION)
  set(CPACK_PACKAGE_VERSION_PATCH
      ${CPACK_PACKAGE_VERSION_PATCH}-${LUNCHBOX_PACKAGE_VERSION})
  set(CPACK_RPM_PACKAGE_RELEASE ${LUNCHBOX_PACKAGE_VERSION})
endif()

set(CPACK_COMPONENTS_ALL lib dev)

set(CPACK_COMPONENT_UNSPECIFIED_DISPLAY_NAME "Misc")
set(CPACK_COMPONENT_UNSPECIFIED_DESCRIPTION "Miscellanous")

set(CPACK_COMPONENT_LIB_DISPLAY_NAME "LunchBox Library")
set(CPACK_COMPONENT_LIB_DESCRIPTION "LunchBox Runtime Library")

set(CPACK_COMPONENT_DEV_DISPLAY_NAME "LunchBox Development Files")
set(CPACK_COMPONENT_DEV_DESCRIPTION "Header and Library Files for LunchBox Development")
set(CPACK_COMPONENT_DEV_DEPENDS lib)

set(CPACK_RPM_PACKAGE_LICENSE "LGPL")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries/Parallel")
set(CPACK_RPM_PACKAGE_VERSION ${VERSION})

if(NOT CPACK_DEBIAN_PACKAGE_MAINTAINER)
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
  if(LUNCHBOX_RELEASE)
    set(DPUT_HOST "ppa:eilemann/equalizer")
  else()
    set(DPUT_HOST "ppa:eilemann/equalizer-dev")
  endif()
endif()

set(EQ_IB_PACKAGES "librdmacm-dev, libibverbs-dev, librdmacm-dev")
set(CPACK_DEBIAN_BUILD_DEPENDS bison flex libgl1-mesa-dev gpu-sd1-dev)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libboost-system-dev, libx11-dev, libgl1-mesa-dev, libglewmx1.5-dev, ${EQ_IB_PACKAGES}, ${GPUSD_DEB_DEPENDENCIES}")

SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "/sbin/ldconfig")

set(CPACK_OSX_PACKAGE_VERSION "${EQ_OSX_VERSION}")

if(MSVC)
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_MODIFY_PATH ON)
endif(MSVC)

if(APPLE)
  set(CPACK_GENERATOR "PackageMaker")
endif(APPLE)

if(LINUX)
  find_program(RPM_EXE rpmbuild)
  if(${RPM_EXE} MATCHES RPM_EXE-NOTFOUND)
    set(CPACK_GENERATOR "DEB")
  else()
    set(CPACK_GENERATOR "DEB;RPM")
  endif()
endif(LINUX)

set(CPACK_STRIP_FILES TRUE)
include(InstallRequiredSystemLibraries)
include(CPack)
include(UploadPPA)
if(UPLOADPPA_FOUND)
  upload_ppa(natty)
  upload_ppa(oneiric)
  add_custom_target(dput_${PROJECT_NAME} DEPENDS ${DPUT_${PROJECT_NAME}_TARGETS})
endif()

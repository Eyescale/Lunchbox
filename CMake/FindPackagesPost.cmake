
if(OPENMP_FOUND)
  if(CMAKE_COMPILER_IS_XLCXX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Disable OpenMP test for xlC Debug build <http://public.kitware.com/Bug/view.php?id=13469>")
    set(OPENMP_FOUND)
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION VERSION_LESS 4.3 AND
      NOT LUNCHBOX_FORCE_OPENMP)
    message(STATUS "Disable OpenMP due to bug in gcc ${GCC_COMPILER_VERSION}")
    set(OPENMP_FOUND)
  endif()
endif()


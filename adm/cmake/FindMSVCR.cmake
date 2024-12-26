# This script finds MSVCR library.
# The script requires:
#  MSVCR_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  MSVCR_FOUND        - package is successfully found
#  MSVCR_INCLUDE_DIR  - directory with headers
#  MSVCR_LIBRARY_DIR  - directory with libraries for linker
#  MSVCR_BINARY_DIR   - directory with DLLs
#  MSVCR_RESOURCE_DIR - directory with resource files
include(FindPackageHandleStandardArgs)

set (MSVCR_DIR "" CACHE PATH "Path to MSVCR library.")

# default paths
set (MSVCR_INCLUDE_DIR  "${MSVCR_DIR}/include")
set (MSVCR_LIBRARY_DIR  "${MSVCR_DIR}/lib")
set (MSVCR_BINARY_DIR   "${MSVCR_DIR}/bin")

# validate location of libraries and headers
set (MSVCR_BINARY_DIR_FOUND)

if (EXISTS "${MSVCR_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}msvcr100${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (MSVCR_BINARY_DIR_FOUND ON)
endif()

if (MSVCR_BINARY_DIR_FOUND)
  set (MSVCR_FOUND ON)
  set (MSVCR_INSTALL_PREFIX ${MSVCR_DIR})
else()
  # no fallback searching for CMake configs
  set (MSVCR_FOUND OFF)
endif()

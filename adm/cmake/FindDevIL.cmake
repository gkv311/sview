# This script finds DevIL library.
# The script requires:
#  DEVIL_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  DEVIL_FOUND        - package is successfully found
#  DEVIL_INCLUDE_DIR  - directory with headers
#  DEVIL_LIBRARY_DIR  - directory with libraries for linker
#  DEVIL_BINARY_DIR   - directory with DLLs
#  DEVIL_RESOURCE_DIR - directory with resource files
include(FindPackageHandleStandardArgs)

set (DEVIL_DIR "" CACHE PATH "Path to DevIL library.")

# default paths
set (DEVIL_INCLUDE_DIR  "${DEVIL_DIR}/include")
set (DEVIL_LIBRARY_DIR  "${DEVIL_DIR}/lib")
set (DEVIL_BINARY_DIR   "${DEVIL_DIR}/bin")

# validate location of libraries and headers
set (DEVIL_INCLUDE_DIR_FOUND)
set (DEVIL_BINARY_DIR_FOUND)
if (EXISTS "${DEVIL_INCLUDE_DIR}/IL/il.h")
  set (DEVIL_INCLUDE_DIR_FOUND ON)
endif()

if (EXISTS "${DEVIL_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}ILU${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (DEVIL_BINARY_DIR_FOUND ON)
endif()

if (DEVIL_INCLUDE_DIR_FOUND AND DEVIL_BINARY_DIR_FOUND)
  set (DEVIL_FOUND ON)
  set (DEVIL_INSTALL_PREFIX ${DEVIL_DIR})
else()
  # no fallback searching for CMake configs
  set (DEVIL_FOUND OFF)
endif()

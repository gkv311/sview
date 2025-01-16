# This script finds MSVCR library.
# The script requires:
#  MSVCR_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  MSVCR_FOUND      - package is successfully found
#  MSVCR_BINARY_DIR - directory with DLLs
include(FindPackageHandleStandardArgs)

set (MSVCR_DIR "" CACHE PATH "Path to MSVCR library.")

# default paths
set (MSVCR_BINARY_DIR "${MSVCR_DIR}/bin")

# validate location of libraries and headers
set (MSVCR_BINARY_DIR_FOUND)
if (EXISTS "${MSVCR_BINARY_DIR}/msvcr100.dll")
  set (MSVCR_BINARY_DIR_FOUND ON)
endif()

if (MSVCR_BINARY_DIR_FOUND)
  set (MSVCR_FOUND ON)
else()
  set (MSVCR_FOUND OFF)
endif()

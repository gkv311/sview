# This script finds NVAPI libraries.
# The script requires:
#  NVAPI_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  NVAPI_FOUND        - package is successfully found
#  NVAPI_INCLUDE_DIR  - directory with headers
#  NVAPI_LIBRARY_DIR  - directory with libraries for linker
#  NVAPI_BINARY_DIR   - directory with DLLs
#  NVAPI_RESOURCE_DIR - directory with resource files
include(FindPackageHandleStandardArgs)

set (NVAPI_DIR "" CACHE PATH "Path to NVAPI library.")

# default paths
set (NVAPI_INCLUDE_DIR  "${NVAPI_DIR}/include")
set (NVAPI_LIBRARY_DIR  "${NVAPI_DIR}/lib")
set (NVAPI_BINARY_DIR   "${NVAPI_DIR}/bin")
set (NVAPI_RESOURCE_DIR "${NVAPI_DIR}/res")

#  list of toolkits
set (NVAPI_TKLIST "nvapi")
set (NVAPI_TKSUFFIX "")
math (EXPR MY_BITNESS "32 + 32*(${CMAKE_SIZEOF_VOID_P}/8)")
if (WIN32)
  set (NVAPI_TKSUFFIX "${MY_BITNESS}")
endif()

# validate location of libraries and headers
set (NVAPI_INCLUDE_DIR_FOUND)
set (NVAPI_LIBRARY_DIR_FOUND)
set (NVAPI_IMPLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
if (EXISTS "${NVAPI_INCLUDE_DIR}/nvapi.h")
  set (NVAPI_INCLUDE_DIR_FOUND ON)
endif()

if (EXISTS "${NVAPI_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}nvapi${NVAPI_TKSUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set (NVAPI_LIBRARY_DIR_FOUND ON)
elseif (NOT WIN32 AND EXISTS "${NVAPI_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}nvapi${NVAPI_TKSUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (NVAPI_LIBRARY_DIR_FOUND ON)
  set (NVAPI_IMPLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

if (NVAPI_INCLUDE_DIR_FOUND AND NVAPI_LIBRARY_DIR_FOUND)
  set (NVAPI_FOUND ON)
  set (NVAPI_INSTALL_PREFIX ${NVAPI_DIR})

  # Define toolkits so that CMake can put absolute paths to linker; the library existence is not checked here.
  foreach (aLibIter ${NVAPI_TKLIST})
    add_library (${aLibIter} SHARED IMPORTED)

    set_property (TARGET ${aLibIter} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties (${aLibIter} PROPERTIES IMPORTED_IMPLIB "${NVAPI_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${NVAPI_TKSUFFIX}${NVAPI_IMPLIB_SUFFIX}")
    set_target_properties (${aLibIter} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NVAPI_INCLUDE_DIR})
  endforeach()
else()
  # no fallback
  set (NVAPI_FOUND OFF)
endif()

# This script finds FreeImage library.
# The script requires:
#  FREEIMAGE_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  FREEIMAGE_FOUND        - package is successfully found
#  FREEIMAGE_INCLUDE_DIR  - directory with headers
#  FREEIMAGE_LIBRARY_DIR  - directory with libraries for linker
#  FREEIMAGE_BINARY_DIR   - directory with DLLs
include(FindPackageHandleStandardArgs)

set (FREEIMAGE_DIR "" CACHE PATH "Path to FreeImage library.")

# default paths
set (FREEIMAGE_INCLUDE_DIR "${FREEIMAGE_DIR}/include")
set (FREEIMAGE_LIBRARY_DIR "${FREEIMAGE_DIR}/lib")
set (FREEIMAGE_BINARY_DIR  "${FREEIMAGE_DIR}/bin")

#  list of toolkits
set (FREEIMAGE_TKLIST "freeimage")
set (FREEIMAGE_TKNAME "freeimage")
if (WIN32)
  set (FREEIMAGE_TKNAME "FreeImage")
endif()

# validate location of libraries and headers
set (FREEIMAGE_INCLUDE_DIR_FOUND)
set (FREEIMAGE_LIBRARY_DIR_FOUND)
set (FREEIMAGE_IMPLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
set (FREEIMAGE_SHAREDLIB_FOUND)
if (EXISTS "${FREEIMAGE_INCLUDE_DIR}/FreeImage.h")
  set (FREEIMAGE_INCLUDE_DIR_FOUND ON)
endif()

if (EXISTS "${FREEIMAGE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${FREEIMAGE_TKNAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set (FREEIMAGE_LIBRARY_DIR_FOUND ON)
elseif (NOT WIN32 AND EXISTS "${FREEIMAGE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${FREEIMAGE_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (FREEIMAGE_LIBRARY_DIR_FOUND ON)
  set (FREEIMAGE_IMPLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

if (WIN32)
  if (EXISTS "${FREEIMAGE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${FREEIMAGE_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (FREEIMAGE_SHAREDLIB_FOUND ON)
  endif()
else()
  if (EXISTS "${FREEIMAGE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${FREEIMAGE_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (FREEIMAGE_SHAREDLIB_FOUND ON)
  endif()
endif()

if (FREEIMAGE_INCLUDE_DIR_FOUND AND FREEIMAGE_LIBRARY_DIR_FOUND)
  set (FREEIMAGE_FOUND ON)

  # Define toolkits so that CMake can put absolute paths to linker; the library existence is not checked here.
  add_library (freeimage SHARED IMPORTED)

  set_property (TARGET freeimage APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
  set_target_properties (freeimage PROPERTIES IMPORTED_IMPLIB "${FREEIMAGE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${FREEIMAGE_TKNAME}${FREEIMAGE_IMPLIB_SUFFIX}")
  set_target_properties (freeimage PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${FREEIMAGE_INCLUDE_DIR})
  if (FREEIMAGE_SHAREDLIB_FOUND)
    if (WIN32)
      set_target_properties (freeimage PROPERTIES IMPORTED_LOCATION "${FREEIMAGE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${FREEIMAGE_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    else()
      set_target_properties (freeimage PROPERTIES IMPORTED_LOCATION "${FREEIMAGE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${FREEIMAGE_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()
  endif()
else()
  # no fallback searching for CMake configs
  set (FREEIMAGE_FOUND OFF)
endif()

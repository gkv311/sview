# This script finds FreeType libraries.
# The script requires:
#  FREETYPE_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  FREETYPE_FOUND        - package is successfully found
#  FREETYPE_INCLUDE_DIR  - directory with headers
#  FREETYPE_LIBRARY_DIR  - directory with libraries for linker
#  FREETYPE_BINARY_DIR   - directory with DLLs
#  FREETYPE_RESOURCE_DIR - directory with resource files
include(FindPackageHandleStandardArgs)

set (FREETYPE_DIR "" CACHE PATH "Path to FreeType library.")

# default paths
set (FREETYPE_INCLUDE_DIR  "${FREETYPE_DIR}/include")
set (FREETYPE_LIBRARY_DIR  "${FREETYPE_DIR}/lib")
set (FREETYPE_BINARY_DIR   "${FREETYPE_DIR}/bin")
set (FREETYPE_RESOURCE_DIR "${FREETYPE_DIR}/res")

#  list of toolkits
set (FREETYPE_TKLIST "freetype")

# validate location of libraries and headers
set (FREETYPE_INCLUDE_DIR_FOUND)
set (FREETYPE_LIBRARY_DIR_FOUND)
set (FREETYPE_IMPLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
set (FREETYPE_SHAREDLIB_FOUND)
if (EXISTS "${FREETYPE_INCLUDE_DIR}/ft2build.h")
  set (FREETYPE_INCLUDE_DIR_FOUND ON)
endif()

if (EXISTS "${FREETYPE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}freetype${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set (FREETYPE_LIBRARY_DIR_FOUND ON)
elseif (NOT WIN32 AND EXISTS "${FREETYPE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}freetype${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (FREETYPE_LIBRARY_DIR_FOUND ON)
  set (FREETYPE_IMPLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

if (WIN32)
  if (EXISTS "${FREETYPE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}freetype${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (FREETYPE_SHAREDLIB_FOUND ON)
  endif()
else()
  if (EXISTS "${FREETYPE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}freetype${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (FREETYPE_SHAREDLIB_FOUND ON)
  endif()
endif()

if (FREETYPE_INCLUDE_DIR_FOUND AND FREETYPE_LIBRARY_DIR_FOUND)
  set (FREETYPE_FOUND ON)
  set (FREETYPE_INSTALL_PREFIX ${FREETYPE_DIR})

  # Define toolkits so that CMake can put absolute paths to linker; the library existence is not checked here.
  foreach (aLibIter ${FREETYPE_TKLIST})
    add_library (${aLibIter} SHARED IMPORTED)

    set_property (TARGET ${aLibIter} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties (${aLibIter} PROPERTIES IMPORTED_IMPLIB "${FREETYPE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${FREETYPE_IMPLIB_SUFFIX}")
    set_target_properties (${aLibIter} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${FREETYPE_INCLUDE_DIR})
    if (FREETYPE_SHAREDLIB_FOUND)
      if (WIN32)
        set_target_properties (${aLibIter} PROPERTIES IMPORTED_LOCATION "${FREETYPE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      else()
        set_target_properties (${aLibIter} PROPERTIES IMPORTED_LOCATION "${FREETYPE_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      endif()
    endif()
  endforeach()
else()
  # fallback searching for CMake configs
  if (NOT "${FREETYPE_DIR}" STREQUAL "")
    set (aDirBak "${FREETYPE_DIR}")
    find_package (FREETYPE CONFIG QUIET PATHS "${FREETYPE_DIR}" NO_DEFAULT_PATH)
    set (FREETYPE_DIR "${aDirBak}" CACHE PATH "Path to FreeType libraries." FORCE)
  else()
    find_package (FREETYPE CONFIG QUIET)
  endif()
endif()

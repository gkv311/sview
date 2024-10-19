# This script finds OpenAL libraries.
# The script requires:
#  OPENAL_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  OPENAL_FOUND        - package is successfully found
#  OPENAL_INCLUDE_DIR  - directory with headers
#  OPENAL_LIBRARY_DIR  - directory with libraries for linker
#  OPENAL_BINARY_DIR   - directory with DLLs
#  OPENAL_RESOURCE_DIR - directory with resource files
include(FindPackageHandleStandardArgs)

set (OPENAL_DIR "" CACHE PATH "Path to OpenAL library.")

# default paths
set (OPENAL_INCLUDE_DIR  "${OPENAL_DIR}/include")
set (OPENAL_LIBRARY_DIR  "${OPENAL_DIR}/lib")
set (OPENAL_BINARY_DIR   "${OPENAL_DIR}/bin")
set (OPENAL_RESOURCE_DIR "${OPENAL_DIR}/res")

#  list of toolkits
set (OPENAL_TKLIST "openal")
set (OPENAL_TKNAME "openal")
if (WIN32)
  set (OPENAL_TKNAME "OpenAL32")
endif()

# validate location of libraries and headers
set (OPENAL_INCLUDE_DIR_FOUND)
set (OPENAL_LIBRARY_DIR_FOUND)
set (OPENAL_IMPLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
set (OPENAL_SHAREDLIB_FOUND)
if (EXISTS "${OPENAL_INCLUDE_DIR}/AL/al.h")
  set (OPENAL_INCLUDE_DIR_FOUND ON)
endif()

if (EXISTS "${OPENAL_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OPENAL_TKNAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set (OPENAL_LIBRARY_DIR_FOUND ON)
elseif (NOT WIN32 AND EXISTS "${OPENAL_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OPENAL_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (OPENAL_LIBRARY_DIR_FOUND ON)
  set (OPENAL_IMPLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

if (WIN32)
  if (EXISTS "${OPENAL_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OPENAL_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (OPENAL_SHAREDLIB_FOUND ON)
  endif()
else()
  if (EXISTS "${OPENAL_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OPENAL_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (OPENAL_SHAREDLIB_FOUND ON)
  endif()
endif()

if (OPENAL_INCLUDE_DIR_FOUND AND OPENAL_LIBRARY_DIR_FOUND)
  set (OPENAL_FOUND ON)
  set (OPENAL_INSTALL_PREFIX ${OPENAL_DIR})

  # Define toolkits so that CMake can put absolute paths to linker; the library existence is not checked here.
  add_library (openal SHARED IMPORTED)

  set_property (TARGET openal APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
  set_target_properties (openal PROPERTIES IMPORTED_IMPLIB "${OPENAL_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OPENAL_TKNAME}${OPENAL_IMPLIB_SUFFIX}")
  set_target_properties (openal PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${OPENAL_INCLUDE_DIR})
  if (OPENAL_SHAREDLIB_FOUND)
    if (WIN32)
      set_target_properties (openal PROPERTIES IMPORTED_LOCATION "${OPENAL_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OPENAL_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    else()
      set_target_properties (openal PROPERTIES IMPORTED_LOCATION "${OPENAL_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OPENAL_TKNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()
  endif()
else()
  # fallback searching for CMake configs
  if (NOT "${OPENAL_DIR}" STREQUAL "")
    set (aDirBak "${OPENAL_DIR}")
    find_package (OPENAL CONFIG QUIET PATHS "${OPENAL_DIR}" NO_DEFAULT_PATH)
    set (OPENAL_DIR "${aDirBak}" CACHE PATH "Path to OpenAL libraries." FORCE)
  else()
    find_package (OPENAL CONFIG QUIET)
  endif() 
endif()

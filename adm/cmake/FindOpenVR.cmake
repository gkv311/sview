# This script finds OpenVR libraries.
# The script requires:
#  OPENVR_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  OPENVR_FOUND        - package is successfully found
#  OPENVR_INCLUDE_DIR  - directory with headers
#  OPENVR_LIBRARY_DIR  - directory with libraries for linker
#  OPENVR_BINARY_DIR   - directory with DLLs
#  OPENVR_RESOURCE_DIR - directory with resource files
include(FindPackageHandleStandardArgs)

set (OPENVR_DIR "" CACHE PATH "Path to OpenVR library.")

# default paths
set (OPENVR_INCLUDE_DIR  "${OPENVR_DIR}/include")
set (OPENVR_LIBRARY_DIR  "${OPENVR_DIR}/lib")
set (OPENVR_BINARY_DIR   "${OPENVR_DIR}/bin")
set (OPENVR_RESOURCE_DIR "${OPENVR_DIR}/res")

#  list of toolkits
set (OPENVR_TKLIST "openvr_api")

# validate location of libraries and headers
set (OPENVR_INCLUDE_DIR_FOUND)
set (OPENVR_LIBRARY_DIR_FOUND)
set (OPENVR_IMPLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
set (OPENVR_SHAREDLIB_FOUND)
set (OPENVR_BITNESS_SUFFIX "")
if (EXISTS "${OPENVR_INCLUDE_DIR}/openvr.h")
  set (OPENVR_INCLUDE_DIR_FOUND ON)
endif()

if (ANDROID AND EXISTS "${OPENVR_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}/libopenvr_api.so")
  set (OPENVR_LIBRARY_DIR_FOUND ON)
  set (OPENVR_LIBRARY_DIR "${OPENVR_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")
  set (OPENVR_IMPLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
elseif (ANDROID AND EXISTS "${OPENVR_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}/libopenvr_api.a")
  set (OPENVR_LIBRARY_DIR_FOUND ON)
  set (OPENVR_LIBRARY_DIR "${OPENVR_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")
elseif (MINGW AND EXISTS "${OPENVR_LIBRARY_DIR}/libopenvr_api64.dll.a")
  set (OPENVR_LIBRARY_DIR_FOUND ON)
  set (OPENVR_IMPLIB_SUFFIX ".dll.a")
  set (OPENVR_BITNESS_SUFFIX "64")
elseif (MINGW AND EXISTS "${OPENVR_LIBRARY_DIR}/libopenvr_api.dll.a")
  set (OPENVR_LIBRARY_DIR_FOUND ON)
  set (OPENVR_IMPLIB_SUFFIX ".dll.a")
elseif (EXISTS "${OPENVR_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api64${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set (OPENVR_LIBRARY_DIR_FOUND ON)
  set (OPENVR_BITNESS_SUFFIX "64")
elseif (EXISTS "${OPENVR_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set (OPENVR_LIBRARY_DIR_FOUND ON)
elseif (NOT WIN32 AND EXISTS "${OPENVR_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (OPENVR_LIBRARY_DIR_FOUND ON)
  set (OPENVR_IMPLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

if (WIN32)
  if (EXISTS "${OPENVR_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${OPENVR_BITNESS_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (OPENVR_SHAREDLIB_FOUND ON)
  endif()
else()
  if (EXISTS "${OPENVR_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${OPENVR_BITNESS_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (OPENVR_SHAREDLIB_FOUND ON)
  endif()
endif()

if (OPENVR_INCLUDE_DIR_FOUND AND OPENVR_LIBRARY_DIR_FOUND)
  set (OPENVR_FOUND ON)
  set (OPENVR_INSTALL_PREFIX ${OPENVR_DIR})

  # Define toolkits so that CMake can put absolute paths to linker; the library existence is not checked here.
  foreach (aLibIter ${OPENVR_TKLIST})
    add_library (${aLibIter} SHARED IMPORTED)

    set_property (TARGET ${aLibIter} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties (${aLibIter} PROPERTIES IMPORTED_IMPLIB "${OPENVR_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${OPENVR_BITNESS_SUFFIX}${OPENVR_IMPLIB_SUFFIX}")
    set_target_properties (${aLibIter} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${OPENVR_INCLUDE_DIR})
    if (OPENVR_SHAREDLIB_FOUND)
      if (WIN32)
        set_target_properties (${aLibIter} PROPERTIES IMPORTED_LOCATION "${OPENVR_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${OPENVR_BITNESS_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      else()
        set_target_properties (${aLibIter} PROPERTIES IMPORTED_LOCATION "${OPENVR_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${OPENVR_BITNESS_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      endif()
    endif()
  endforeach()
elseif ("${OPENVR_DIR}" STREQUAL "")
  # OpenVR doesn't provide CMake configs, but has pkgconfig
  find_package (PkgConfig REQUIRED)
  pkg_check_modules (LIBVR REQUIRED IMPORTED_TARGET openvr)
  if (LIBVR_FOUND)
    set (OPENVR_FOUND ON)
    set (OPENVR_INCLUDE_DIR "${LIBVR_INCLUDE_DIRS}")
    set (OPENVR_LIBRARY_DIR "${LIBVR_LIBRARY_DIRS}")
    if (NOT EXISTS "${LIBVR_LIBRARY_DIRS}")
      foreach (aLibIter ${LIBVR_LINK_LIBRARIES})
        get_filename_component (OPENVR_LIBRARY_DIR "${aLibIter}" DIRECTORY)
      endforeach()
    endif()
    set (OPENVR_BINARY_DIR "${OPENVR_LIBRARY_DIR}")
    foreach (aLibIter ${OPENVR_TKLIST})
      add_library (${aLibIter} SHARED IMPORTED)
      set_property (TARGET ${aLibIter} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties (${aLibIter} PROPERTIES IMPORTED_IMPLIB "${OPENVR_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${OPENVR_IMPLIB_SUFFIX}")
      set_target_properties (${aLibIter} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${OPENVR_INCLUDE_DIR})
      set_target_properties (${aLibIter} PROPERTIES IMPORTED_LOCATION "${OPENVR_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endforeach()
  endif()
else()
  set (OPENVR_FOUND OFF)
endif()

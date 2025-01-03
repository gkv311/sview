# This script finds FFmpeg libraries.
# The script requires:
#  FFMPEG_DIR - root folder or folder with CMake configuration files
#
# Script will define the following variables on success:
#  FFMPEG_FOUND        - package is successfully found
#  FFMPEG_INCLUDE_DIR  - directory with headers
#  FFMPEG_LIBRARY_DIR  - directory with libraries for linker
#  FFMPEG_BINARY_DIR   - directory with DLLs
#  FFMPEG_RESOURCE_DIR - directory with resource files
include(FindPackageHandleStandardArgs)

set (FFMPEG_DIR "" CACHE PATH "Path to FFmpeg libraries.")

# default paths
set (FFMPEG_INCLUDE_DIR  "${FFMPEG_DIR}/include")
set (FFMPEG_LIBRARY_DIR  "${FFMPEG_DIR}/lib")
set (FFMPEG_FRAMEWORK_DIR "${FFMPEG_DIR}/Frameworks")
set (FFMPEG_BINARY_DIR   "${FFMPEG_DIR}/bin")
set (FFMPEG_RESOURCE_DIR "${FFMPEG_DIR}/res")

#  list of toolkits
set (FFMPEG_TKLIST "avcodec" "avdevice" "avformat" "avutil" "swscale")

# validate location of libraries and headers
set (FFMPEG_INCLUDE_DIR_FOUND)
set (FFMPEG_LIBRARY_DIR_FOUND)
set (FFMPEG_IMPLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
set (FFMPEG_SHAREDLIB_FOUND)
if (EXISTS "${FFMPEG_INCLUDE_DIR}/libavutil/avutil.h")
  set (FFMPEG_INCLUDE_DIR_FOUND ON)
endif()

if (EXISTS "${FFMPEG_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}avutil${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set (FFMPEG_LIBRARY_DIR_FOUND ON)
elseif (NOT WIN32 AND EXISTS "${FFMPEG_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}avutil${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (FFMPEG_LIBRARY_DIR_FOUND ON)
  set (FFMPEG_IMPLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
elseif (APPLE AND EXISTS "${FFMPEG_FRAMEWORK_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}avutil${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set (FFMPEG_LIBRARY_DIR_FOUND ON)
  set (FFMPEG_IMPLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  set (FFMPEG_LIBRARY_DIR  "${FFMPEG_FRAMEWORK_DIR}")
endif()

if (WIN32)
  if (EXISTS "${FFMPEG_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}avutil${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (FFMPEG_SHAREDLIB_FOUND ON)
  endif()
else()
  if (EXISTS "${FFMPEG_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}avutil${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (FFMPEG_SHAREDLIB_FOUND ON)
  endif()
endif()

if (FFMPEG_INCLUDE_DIR_FOUND AND FFMPEG_LIBRARY_DIR_FOUND)
  set (FFMPEG_FOUND ON)
  set (FFMPEG_INSTALL_PREFIX ${FFMPEG_DIR})

  # Define toolkits so that CMake can put absolute paths to linker; the library existence is not checked here.
  foreach (aLibIter ${FFMPEG_TKLIST})
    add_library (${aLibIter} SHARED IMPORTED)

    set_property (TARGET ${aLibIter} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties (${aLibIter} PROPERTIES IMPORTED_IMPLIB "${FFMPEG_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${FFMPEG_IMPLIB_SUFFIX}")
    set_target_properties (${aLibIter} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR})
    if (FFMPEG_SHAREDLIB_FOUND)
      if (WIN32)
        set_target_properties (${aLibIter} PROPERTIES IMPORTED_LOCATION "${FFMPEG_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      else()
        set_target_properties (${aLibIter} PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      endif()
    endif()
  endforeach()
else()
  # FFmpeg doesn't provide CMake configs, but has pkgconfig
  find_package (PkgConfig REQUIRED)
  pkg_check_modules (LIBAV REQUIRED IMPORTED_TARGET libavcodec libavdevice libavformat libavutil libswscale)
  if (LIBAV_FOUND)
    set (FFMPEG_FOUND ON)
    set (FFMPEG_INCLUDE_DIR  "${LIBAV_INCLUDE_DIRS}")
    set (FFMPEG_LIBRARY_DIR  "${LIBAV_LIBRARY_DIRS}")
    set (FFMPEG_BINARY_DIR   "${LIBAV_LIBRARY_DIRS}")
    foreach (aLibIter ${FFMPEG_TKLIST})
      add_library (${aLibIter} SHARED IMPORTED)
      set_property (TARGET ${aLibIter} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties (${aLibIter} PROPERTIES IMPORTED_IMPLIB "${FFMPEG_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${FFMPEG_IMPLIB_SUFFIX}")
      set_target_properties (${aLibIter} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR})
      set_target_properties (${aLibIter} PROPERTIES IMPORTED_LOCATION "${FFMPEG_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${aLibIter}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endforeach()
  endif()
endif()

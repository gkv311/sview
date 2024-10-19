cmake_minimum_required (VERSION 3.7)

# common solution for multiple independent projects
project (sView)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/adm/cmake" ${CMAKE_MODULE_PATH})
set (APP_VERSION_MAJOR 1)
set (APP_VERSION_MINOR 0)

if (MSVC)
  set (BUILD_FORCE_RelWithDebInfo ON CACHE BOOL "Build release target with debug symbols")
else()
  set (BUILD_FORCE_RelWithDebInfo OFF)
endif()

if (BUILD_FORCE_RelWithDebInfo)
  set (CMAKE_CONFIGURATION_TYPES Release Debug CACHE INTERNAL "" FORCE)
else()
  set (CMAKE_CONFIGURATION_TYPES Release Debug RelWithDebInfo CACHE INTERNAL "" FORCE)
endif()

# compiler flags
set (CMAKE_CXX_STANDARD 11)
if (MSVC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise /MP")
  add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
  if (BUILD_FORCE_RelWithDebInfo)
    # generate debug info (Zi), inline expansion level (Ob1)
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /Ob1")
    set (CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   /Zi /Ob1")
    # generate debug info (debug), OptimizeReferences=true (OPT:REF), EnableCOMDATFolding=true (OPT:ICF)
    set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /debug /OPT:REF /OPT:ICF")
  endif()
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")
  #add_definitions(-DOCC_CONVERT_SIGNALS)
endif()
if (WIN32)
  add_definitions(-DUNICODE)
endif()

# increase compiler warnings level (-W3 for MSVC, -Wextra for GCC)
set (BUILD_TREAT_WARNINGS_AS_ERRORS OFF CACHE BOOL "Treat compilation warnings as errors")
if (MSVC)
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string (REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
  endif()
  if (BUILD_TREAT_WARNINGS_AS_ERRORS)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")    
  endif()
elseif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  if (BUILD_TREAT_WARNINGS_AS_ERRORS)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")    
  endif()
endif()

# generated config header file
add_definitions(-DST_HAVE_STCONFIG)
#add_definitions(-DST_HAVE_MONGOOSE)

# add verbose debug logging for debug config
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DST_DEBUG")

# Define 3rd-party root dir
if (NOT DEFINED 3RDPARTY_DIR)
  set (3RDPARTY_DIR "" CACHE PATH "3rd-party products directory" FORCE)
endif()
if (3RDPARTY_DIR STREQUAL "")
  #message (FATAL_ERROR "could not find 3rd-party products, please set 3RDPARTY_DIR variable" )
else()
  message (STATUS "Using 3rd-party from \"${3RDPARTY_DIR}\"" )
endif()

# define major 3rd-party folders in advance for cmake-gui
set (FREETYPE_DIR "" CACHE PATH "Path to FreeType library.")
set (FFMPEG_DIR   "" CACHE PATH "Path to FFmpeg libraries.")

find_package (OpenGL REQUIRED)

find_package (freetype REQUIRED)
if (NOT FREETYPE_FOUND)
  message (FATAL_ERROR "could not find FreeType, please set FREETYPE_DIR variable" )
else()
  message (STATUS "FREETYPE_INCLUDE_DIR=${FREETYPE_INCLUDE_DIR}")
  message (STATUS "FREETYPE_LIBRARY_DIR=${FREETYPE_LIBRARY_DIR}")
endif()

find_package (FFmpeg REQUIRED)
if (NOT FFMPEG_FOUND)
  message (FATAL_ERROR "could not find FFmpeg, please set FFMPEG_DIR variable" )
else()
  message (STATUS "FFMPEG_INCLUDE_DIR=${FFMPEG_INCLUDE_DIR}")
  message (STATUS "FFMPEG_LIBRARY_DIR=${FFMPEG_LIBRARY_DIR}")
endif()

find_package (OpenAL REQUIRED)
if (NOT OPENAL_FOUND)
  message (FATAL_ERROR "could not find OpenAL, please set OPENAL_DIR variable" )
else()
  message (STATUS "OPENAL_INCLUDE_DIR=${OPENAL_INCLUDE_DIR}")
  message (STATUS "OPENAL_LIBRARY_DIR=${OPENAL_LIBRARY_DIR}")
endif()

find_package (OpenVR REQUIRED)
if (NOT OPENVR_FOUND)
  message (FATAL_ERROR "could not find OpenVR, please set OPENVR_DIR variable" )
else()
  message (STATUS "OPENVR_INCLUDE_DIR=${OPENVR_INCLUDE_DIR}")
  message (STATUS "OPENVR_LIBRARY_DIR=${OPENVR_LIBRARY_DIR}")
endif()

find_package (NVAPI REQUIRED)
if (NOT NVAPI_FOUND)
  message (FATAL_ERROR "could not find NVAPI, please set NVAPI_DIR variable" )
else()
  message (STATUS "NVAPI_INCLUDE_DIR=${NVAPI_INCLUDE_DIR}")
  message (STATUS "NVAPI_LIBRARY_DIR=${NVAPI_LIBRARY_DIR}")
endif()

# solution search folders
include_directories (${PROJECT_SOURCE_DIR}/include)

# add sub-projects to this Solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# localization list
set (aLangList chinese chineset czech english french german korean russian spanish)
set (aLangMap_english  English)
set (aLangMap_spanish  Español)
set (aLangMap_russian  русский)
set (aLangMap_french   français)
set (aLangMap_german   Deutsch)
set (aLangMap_chinese  ChineseS)
set (aLangMap_chineset ChineseT)
set (aLangMap_korean   Korean)
set (aLangMap_czech    Czech)
foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
  foreach (aLangIter ${aLangList})
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}/lang/${aLangMap_${aLangIter}}")
  endforeach()
endforeach()

# function to override output directories
function (st_set_target_output_dirs theTarget)
  # move sub-projects outputs into common folder
  set_target_properties(${theTarget} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/"
    PDB_OUTPUT_DIRECTORY_RELEASE     "${CMAKE_BINARY_DIR}/Release/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug/"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug/"
    PDB_OUTPUT_DIRECTORY_DEBUG       "${CMAKE_BINARY_DIR}/Debug/"
  )

  # configure DLL search path for VS debugger
  #get_target_property (avutil_location avutil LOCATION)
  #get_filename_component (FFMPEG_DLL_DIR ${avutil_location} DIRECTORY)
  if (MSVC)
    #VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${theTarget}>"
    #VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${theTarget}>"
    set_target_properties(${theTarget} PROPERTIES
      VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${FFMPEG_BINARY_DIR};${FREETYPE_BINARY_DIR};${OPENAL_BINARY_DIR};${OPENVR_BINARY_DIR}"
    )
  endif()

  # copy localization to build folder
  if (EXISTS "${PROJECT_SOURCE_DIR}/lang/")
    foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
      foreach (aLangIter ${aLangList})
        configure_file ("${PROJECT_SOURCE_DIR}/lang/${aLangIter}/${theTarget}.lng" "${CMAKE_BINARY_DIR}/${aCfgIter}/lang/${aLangMap_${aLangIter}}/" COPYONLY)
      endforeach()
    endforeach()
  endif()
endfunction()

# core libraries
add_subdirectory(StShared)
add_subdirectory(StCore)
add_subdirectory(StGLWidgets)

# outputs
add_subdirectory(StOutAnaglyph)
add_subdirectory(StOutDistorted)
add_subdirectory(StOutDual)
add_subdirectory(StOutInterlace)
add_subdirectory(StOutIZ3D)
add_subdirectory(StOutPageFlip)

# applications
add_subdirectory(StDiagnostics)
add_subdirectory(StImageViewer)
add_subdirectory(StMoviePlayer)

#add_subdirectory(StBrowserPlugin)
add_subdirectory(StMonitorsDump)
add_subdirectory(StTests)
add_subdirectory(sview)

# CAD Viewer
# find Open CASCADE Technology
#find_package (OpenCASCADE REQUIRED)
#if (NOT OpenCASCADE_FOUND)
#  message (FATAL_ERROR "could not find OpenCASCADE, please set OpenCASCADE_DIR variable" )
#else()
#  message (STATUS "OpenCASCADE_INCLUDE_DIR=${OpenCASCADE_INCLUDE_DIR}")
#  message (STATUS "OpenCASCADE_LIBRARY_DIR=${OpenCASCADE_LIBRARY_DIR}")
#endif()
#add_subdirectory(StCADViewer)

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
    string (REGEX MATCH "/Ob2" ISFLAG "${CMAKE_CXX_FLAGS_RELEASE}")
    if (ISFLAG)
      string (REGEX REPLACE "/Ob2" "/Ob1" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
      string (REGEX REPLACE "/Ob2" "/Ob1" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    else()
      set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ob1")
      set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Ob1")
    endif()
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set (CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   /Zi")
    # generate debug info (debug), OptimizeReferences=true (OPT:REF), EnableCOMDATFolding=true (OPT:ICF)
    set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /debug /OPT:REF /OPT:ICF")
  endif()
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")
  #add_definitions(-DOCC_CONVERT_SIGNALS)
endif()
if (WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
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
set (OPENAL_DIR   "" CACHE PATH "Path to OpenAL library.")
set (OPENVR_DIR   "" CACHE PATH "Path to OpenVR library.")

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

set (USE_OPENVR ON CACHE BOOL "Use OpenVR library.")
if (USE_OPENVR)
  find_package (OpenVR REQUIRED)
  if (NOT OPENVR_FOUND)
    message (FATAL_ERROR "could not find OpenVR, please set OPENVR_DIR variable" )
  else()
    message (STATUS "OPENVR_INCLUDE_DIR=${OPENVR_INCLUDE_DIR}")
    message (STATUS "OPENVR_LIBRARY_DIR=${OPENVR_LIBRARY_DIR}")
  endif()
endif()

if (WIN32)
  set (USE_FREEIMAGE OFF CACHE BOOL "Use FreeImage library.")
  if (USE_FREEIMAGE)
    find_package (FreeImage REQUIRED)
    if (NOT FREEIMAGE_FOUND)
      message (FATAL_ERROR "could not find FreeImage, please set FREEIMAGE_DIR variable" )
    else()
      message (STATUS "FREEIMAGE_BINARY_DIR=${FREEIMAGE_BINARY_DIR}")
    endif()
  endif()

  set (USE_DEVIL OFF CACHE BOOL "Use DevIL library.")
  if (USE_DEVIL)
    find_package (DevIL REQUIRED)
    if (NOT DEVIL_FOUND)
      message (FATAL_ERROR "could not find DevIL, please set DEVIL_DIR variable" )
    else()
      message (STATUS "DEVIL_BINARY_DIR=${DEVIL_BINARY_DIR}")
    endif()
  endif()

  find_package (NVAPI REQUIRED)
  if (NOT NVAPI_FOUND)
    message (FATAL_ERROR "could not find NVAPI, please set NVAPI_DIR variable" )
  else()
    message (STATUS "NVAPI_INCLUDE_DIR=${NVAPI_INCLUDE_DIR}")
    message (STATUS "NVAPI_LIBRARY_DIR=${NVAPI_LIBRARY_DIR}")
  endif()

  set (USE_MSVCR OFF CACHE BOOL "Use MSVCR library.")
  if (USE_MSVCR)
    find_package (MSVCR REQUIRED)
    if (NOT MSVCR_FOUND)
      message (FATAL_ERROR "could not find MSVCR, please set MSCVR_DIR variable" )
    else()
      message (STATUS "MSVCR_BINARY_DIR=${MSVCR_BINARY_DIR}")
    endif()
  endif()
else()
  set (USE_FREEIMAGE OFF)
  set (USE_DEVIL OFF)
  set (USE_MSVCR OFF)
endif()

# solution search folders
include_directories (${PROJECT_SOURCE_DIR}/include)

# add sub-projects to this Solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# install options
if (WIN32)
  set (APP_PREFIX "")
  set (USR_LIB "")
else()
  set (APP_PREFIX "/usr" CACHE BOOL "app prefix")
  set (USR_LIB "/lib" CACHE BOOL "usr lib")
  if (APP_PREFIX)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DAPP_PREFIX=\"${APP_PREFIX}\"")
  endif()
endif()
set (SVIEW_INSTALL_BIN "${CMAKE_INSTALL_PREFIX}/bin")
set (SVIEW_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib")
set (SVIEW_INSTALL_DLL "${CMAKE_INSTALL_PREFIX}/lib")
set (SVIEW_INSTALL_RES "${CMAKE_INSTALL_PREFIX}/share/sView")
if (WIN32)
  set (SVIEW_INSTALL_BIN "${CMAKE_INSTALL_PREFIX}")
  set (SVIEW_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}")
  set (SVIEW_INSTALL_DLL "${CMAKE_INSTALL_PREFIX}")
  set (SVIEW_INSTALL_RES "${CMAKE_INSTALL_PREFIX}")
endif()

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
        if (EXISTS "${PROJECT_SOURCE_DIR}/lang/${aLangIter}/language.lng")
          configure_file ("${PROJECT_SOURCE_DIR}/lang/${aLangIter}/language.lng" "${CMAKE_BINARY_DIR}/${aCfgIter}/lang/${aLangMap_${aLangIter}}/" COPYONLY)
        endif()
      endforeach()
    endforeach()
  endif()
endfunction()

function (sview_set_target_output_dirs theTarget)
  st_set_target_output_dirs(${theTarget})

  # add install target
  install (TARGETS "${theTarget}"
           RUNTIME DESTINATION "${SVIEW_INSTALL_BIN}"
	   LIBRARY DESTINATION "${SVIEW_INSTALL_LIB}")
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

add_subdirectory(StBrowserPlugin)
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

# install common resources
install (DIRECTORY "${CMAKE_SOURCE_DIR}/textures"        DESTINATION "${SVIEW_INSTALL_RES}")
install (DIRECTORY "${CMAKE_BINARY_DIR}/Release/lang"    DESTINATION "${SVIEW_INSTALL_RES}")
install (DIRECTORY "${CMAKE_BINARY_DIR}/Release/shaders" DESTINATION "${SVIEW_INSTALL_RES}")
install (DIRECTORY "${CMAKE_BINARY_DIR}/Release/web"     DESTINATION "${SVIEW_INSTALL_RES}")
if (WIN32)
  # copy FreeType
  set (aFreeTypeDll "${FREETYPE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}freetype${CMAKE_SHARED_LIBRARY_SUFFIX}")
  if (EXISTS "${aFreeTypeDll}")
    install (FILES "${aFreeTypeDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
  endif()

  # copy FFmpeg
  file (GLOB_RECURSE FFMPEG_DLLS ${FFMPEG_BINARY_DIR}/*.dll)
  file (GLOB_RECURSE FFMPEG_EXES ${FFMPEG_BINARY_DIR}/*.exe)
  install (FILES ${FFMPEG_DLLS} DESTINATION ${SVIEW_INSTALL_DLL})
  install (FILES ${FFMPEG_EXES} DESTINATION ${SVIEW_INSTALL_BIN})

  # copy OpenAL
  set (anOpenAlDll "${OPENAL_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}OpenAL32${CMAKE_SHARED_LIBRARY_SUFFIX}")
  if (EXISTS "${anOpenAlDll}")
    install (FILES "${anOpenAlDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
  endif()

  # copy OpenVR
  if (USE_OPENVR)
    set (anOpenVrDll "${OPENVR_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install (FILES "${anOpenVrDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
  endif()

  # copy FreeImage
  if (USE_FREEIMAGE)
    set (aFreeImageDll "${FREEIMAGE_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}FreeImage${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install (FILES "${aFreeImageDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
  endif()

  # copy DevIL
  if (USE_DEVIL)
    set (aDevilDll "${DEVIL_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}DevIL${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (anIluDll  "${DEVIL_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}ILU${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install (FILES "${aDevilDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
    install (FILES "${anIluDll}"  DESTINATION "${SVIEW_INSTALL_DLL}")
  endif()

  # copy MSVCR
  if (USE_MSVCR)
    set (anMsvcrDll "${MSVCR_BINARY_DIR}/msvcr100.dll")
    set (anMsvcpDll "${MSVCR_BINARY_DIR}/msvcp100.dll")
    set (anMfcDll   "${MSVCR_BINARY_DIR}/mfc100u.dll")
    install (FILES "${anMsvcrDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
    install (FILES "${anMsvcpDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
    install (FILES "${anMfcDll}"   DESTINATION "${SVIEW_INSTALL_DLL}")
  endif()
endif()

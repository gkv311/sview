cmake_minimum_required (VERSION 3.7)

# common solution for multiple independent projects
project (sView)

# we use include(adm/cmake/Find*.cmake) explicitly instead of find_package()
# to distinguish normal configuration files from custom search scripts
#set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/adm/cmake" ${CMAKE_MODULE_PATH})

set (APP_VERSION_MAJOR 1)
set (APP_VERSION_MINOR 0)
# Android APK version
set (SVIEW_APK_VERSIONCODE "50")
set (SVIEW_APK_VERSIONNAME "25.2")

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
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
    # generate debug info (debug), OptimizeReferences=true (OPT:REF), EnableCOMDATFolding=true (OPT:ICF)
    set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /debug /OPT:REF /OPT:ICF")
    set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug /OPT:REF /OPT:ICF")
  endif()
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")
  #add_definitions(-DOCC_CONVERT_SIGNALS)
endif()
if (WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
elseif (ANDROID)
  add_definitions(-D__STDC_CONSTANT_MACROS)
endif()

# minimum OS version
if (APPLE)
  # minimal supported version of macOS:
  # - CMake: CMAKE_OSX_DEPLOYMENT
  # - qmake: QMAKE_MACOSX_DEPLOYMENT_TARGET = $(TARGET_OS_VERSION)
  # - environment variable: export MACOSX_DEPLOYMENT_TARGET=$(TARGET_OS_VERSION)
  #EXTRA_CFLAGS   += -mmacosx-version-min=$(TARGET_OS_VERSION)
  #EXTRA_CXXFLAGS += -mmacosx-version-min=$(TARGET_OS_VERSION)
  #EXTRA_LDFLAGS  += -mmacosx-version-min=$(TARGET_OS_VERSION)
  set (CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum OS X deployment version" FORCE)

  #set (CMAKE_INSTALL_NAME_DIR "@executable_path/../Frameworks")
  set (INSTALL_NAME_DIR "@executable_path" CACHE STRING "Directory name for installed targets on Apple platforms")
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
if (WIN32)
  set (USE_STCONFIG ON  CACHE BOOL "Use 'stconfig.conf' configuration file.")
else()
  set (USE_STCONFIG OFF CACHE BOOL "Use 'stconfig.conf' configuration file.")
endif()
if (USE_STCONFIG)
  add_definitions(-DST_HAVE_STCONFIG)
endif()

# add verbose debug logging for debug config
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DST_DEBUG")
# create log file at specified location (useful for debugging on systems like Android)
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DST_DEBUG_LOG_TO_FILE=\"/storage/emulated/0/Android/data/com.sview/files/sview.log\")
# debug OpenGL context
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DST_DEBUG_GL")
# log verbose messages from FFmpeg
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DST_DEBUG_FFMPEG_VERBOSE")
# dump messages into system log
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DST_DEBUG_SYSLOG")
# prefix each message with thread id
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DST_DEBUG_THREADID")

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
set (USE_LIBCONFIG OFF)
set (USE_GTK       OFF)
set (USE_XLIB      OFF)
set (FREETYPE_DIR ""  CACHE PATH "Path to FreeType library.")
set (FFMPEG_DIR   ""  CACHE PATH "Path to FFmpeg libraries.")
set (OPENAL_DIR   ""  CACHE PATH "Path to OpenAL library.")
set (USE_OPENVR   ON  CACHE BOOL "Use OpenVR library.")
set (OPENVR_DIR   ""  CACHE PATH "Path to OpenVR library.")
set (USE_MONGOOSE ON  CACHE BOOL "Enable Web-UI option.")
set (USE_UPDATER  OFF CACHE BOOL "Enable built-in updater.")
if (WIN32)
  set (USE_FREEIMAGE OFF CACHE BOOL "Use FreeImage library.")
  set (USE_DEVIL     OFF CACHE BOOL "Use DevIL library.")
  set (USE_MSVCR     OFF CACHE BOOL "Use MSVCR library.")
  set (USE_NVAPI     ON  CACHE BOOL "Use NVAPI library for better support of legacy 3D Vision glasses")
  set (NVAPI_DIR     ""  CACHE PATH "Path to NVAPI library.")
else()
  set (USE_FREEIMAGE OFF)
  set (USE_DEVIL     OFF)
  set (USE_MSVCR     OFF)
  set (USE_NVAPI     OFF)
endif()
if (APPLE)
  set (USE_OPENVR    OFF)
endif()

if (USE_MONGOOSE)
  add_definitions(-DST_HAVE_MONGOOSE)
endif()
if (USE_UPDATER)
  add_definitions(-DST_UPDATES_CHECK)
endif()

include (adm/cmake/FindFreetype.cmake)
if (NOT FREETYPE_FOUND)
  message (FATAL_ERROR "could not find FreeType, please set FREETYPE_DIR variable" )
else()
  message (STATUS "FREETYPE_INCLUDE_DIR=${FREETYPE_INCLUDE_DIR}")
  message (STATUS "FREETYPE_LIBRARY_DIR=${FREETYPE_LIBRARY_DIR}")
endif()

include (adm/cmake/FindFFmpeg.cmake)
if (NOT FFMPEG_FOUND)
  message (FATAL_ERROR "could not find FFmpeg, please set FFMPEG_DIR variable" )
else()
  message (STATUS "FFMPEG_INCLUDE_DIR=${FFMPEG_INCLUDE_DIR}")
  message (STATUS "FFMPEG_LIBRARY_DIR=${FFMPEG_LIBRARY_DIR}")
endif()

include (adm/cmake/FindOpenAL.cmake)
if (NOT OPENAL_FOUND)
  message (FATAL_ERROR "could not find OpenAL, please set OPENAL_DIR variable" )
else()
  message (STATUS "OPENAL_INCLUDE_DIR=${OPENAL_INCLUDE_DIR}")
  message (STATUS "OPENAL_LIBRARY_DIR=${OPENAL_LIBRARY_DIR}")
endif()

if (USE_OPENVR)
  add_definitions(-DST_HAVE_OPENVR)
  include (adm/cmake/FindOpenVR.cmake)
  if (NOT OPENVR_FOUND)
    message (FATAL_ERROR "could not find OpenVR, please set OPENVR_DIR variable" )
  else()
    message (STATUS "OPENVR_INCLUDE_DIR=${OPENVR_INCLUDE_DIR}")
    message (STATUS "OPENVR_LIBRARY_DIR=${OPENVR_LIBRARY_DIR}")
  endif()
endif()

if (WIN32)
  if (USE_FREEIMAGE)
    include (adm/cmake/FindFreeImage.cmake)
    if (NOT FREEIMAGE_FOUND)
      message (FATAL_ERROR "could not find FreeImage, please set FREEIMAGE_DIR variable" )
    else()
      message (STATUS "FREEIMAGE_BINARY_DIR=${FREEIMAGE_BINARY_DIR}")
    endif()
  endif()

  if (USE_DEVIL)
    include (adm/cmake/FindDevIL.cmake)
    if (NOT DEVIL_FOUND)
      message (FATAL_ERROR "could not find DevIL, please set DEVIL_DIR variable" )
    else()
      message (STATUS "DEVIL_BINARY_DIR=${DEVIL_BINARY_DIR}")
    endif()
  endif()

  if (USE_NVAPI)
    add_definitions(-DST_HAVE_NVAPI)
    include (adm/cmake/FindNVAPI.cmake)
    if (NOT NVAPI_FOUND)
      message (FATAL_ERROR "could not find NVAPI, please set NVAPI_DIR variable" )
    else()
      message (STATUS "NVAPI_INCLUDE_DIR=${NVAPI_INCLUDE_DIR}")
      message (STATUS "NVAPI_LIBRARY_DIR=${NVAPI_LIBRARY_DIR}")
    endif()
  endif()

  if (USE_MSVCR)
    include (adm/cmake/FindMSVCR.cmake)
    if (NOT MSVCR_FOUND)
      message (FATAL_ERROR "could not find MSVCR, please set MSCVR_DIR variable" )
    else()
      message (STATUS "MSVCR_BINARY_DIR=${MSVCR_BINARY_DIR}")
    endif()
  endif()
elseif (APPLE)
  #
elseif (ANDROID)
  add_definitions(-DST_HAVE_EGL)
  set (USE_LIBCONFIG ON)
  include (adm/cmake/FindLibconfigCpp.cmake)
  if (NOT LIBCONFIGCPP_FOUND)
    message (FATAL_ERROR "could not find libconfig++, please set LIBCONFIGCPP_DIR variable" )
  endif()
  message (STATUS "LIBCONFIGCPP_INCLUDE_DIR=${LIBCONFIGCPP_INCLUDE_DIR}")
  message (STATUS "LIBCONFIGCPP_LIBRARY=${LIBCONFIGCPP_LIBRARY}")
else()
  # Linux
  set (USE_GTK       OFF CACHE BOOL "Use GTK2 for openfile dialog.")
  set (USE_XLIB      ON)
  set (USE_LIBCONFIG ON)

  if (USE_GTK)
    find_package (GTK2 REQUIRED gtk)
    if (NOT GTK2_FOUND)
      message (FATAL_ERROR "could not find GTK2 (libgtk2.0-dev)" )
    endif()
    if (NOT GTK2_INCLUDE_DIRS)
      message (FATAL_ERROR "could not find GTK2 (GTK2_INCLUDE_DIRS)" )
    endif()
    message (STATUS "GTK2_INCLUDE_DIRS=${GTK2_INCLUDE_DIRS}")
    add_definitions(-DST_HAVE_GTK)
  endif()

  find_package (X11 REQUIRED)
  if (NOT X11_FOUND)
    message (FATAL_ERROR "could not find X11" )
  endif()
  message (STATUS "X11_INCLUDE_DIR=${X11_INCLUDE_DIR}")

  include (adm/cmake/FindLibconfigCpp.cmake)
  if (NOT LIBCONFIGCPP_FOUND)
    message (FATAL_ERROR "could not find libconfig++, please set LIBCONFIGCPP_DIR variable" )
  endif()
  message (STATUS "LIBCONFIGCPP_INCLUDE_DIR=${LIBCONFIGCPP_INCLUDE_DIR}")
  message (STATUS "LIBCONFIGCPP_LIBRARY=${LIBCONFIGCPP_LIBRARY}")
endif()

if (ANDROID)
  #set (ST_OPENGL_LIBRARIES "${OPENGL_gles2_LIBRARY}" "${OPENGL_egl_LIBRARY}")
  set (ST_OPENGL_LIBRARIES "EGL" "GLESv2")
elseif (USE_XLIB)
  set (OpenGL_GL_PREFERENCE "GLVND")
  find_package (OpenGL REQUIRED)
  # OPENGL_LIBRARIES includes GLU library which is not needed
  #message (STATUS "OPENGL_LIBRARIES=${OPENGL_LIBRARIES}")
  #set (ST_OPENGL_LIBRARIES "${OPENGL_opengl_LIBRARY}" "${OPENGL_glx_LIBRARY}")
  set (ST_OPENGL_LIBRARIES "OpenGL::GL")
else()
  find_package (OpenGL REQUIRED)
  #set (ST_OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY})
  set (ST_OPENGL_LIBRARIES "OpenGL::GL")
endif()
message (STATUS "ST_OPENGL_LIBRARIES=${ST_OPENGL_LIBRARIES}")

# solution search folders
include_directories (${PROJECT_SOURCE_DIR}/include)

# add sub-projects to this Solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# install options
set (SVIEW_BUNDLE_EXT "")
set (SVIEW_BUNDLE_BIN "")
set (SVIEW_BUNDLE_RES "")
if (WIN32)
  set (USR_BIN "")
  set (USR_LIB "")
  set (USR_SHARE "")
  set (USR_SVIEW "")
elseif (APPLE)
  set (SVIEW_BUNDLE_EXT ".app")
  set (SVIEW_BUNDLE_BIN ".app/Contents/MacOS")
  set (SVIEW_BUNDLE_RES ".app/Contents/MacOS")
  set (USR_BIN   "/Contents/MacOS")
  set (USR_LIB   "/Contents/MacOS")
  set (USR_SHARE "/Contents/MacOS")
  set (USR_SVIEW "")
elseif (ANDROID)
  set (SVIEW_BUNDLE_BIN "/lib/${CMAKE_ANDROID_ARCH_ABI}")
  set (SVIEW_BUNDLE_RES "/assets")
  set (USR_BIN   "/bin")
  set (USR_LIB   "/lib")
  set (USR_SHARE "/assets")
  set (USR_SVIEW "")
else()
  include (GNUInstallDirs)
  set (USR_BIN   "/${CMAKE_INSTALL_BINDIR}")
  set (USR_LIB   "/${CMAKE_INSTALL_LIBDIR}")
  set (USR_SHARE "/${CMAKE_INSTALL_DATAROOTDIR}")
  set (USR_SVIEW "/sView")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DAPP_PREFIX=\"\\\"${CMAKE_INSTALL_PREFIX}\\\"\"")
endif()

set (SVIEW_INSTALL_BIN   "${CMAKE_INSTALL_PREFIX}${USR_BIN}")
set (SVIEW_INSTALL_LIB   "${CMAKE_INSTALL_PREFIX}${USR_LIB}")
set (SVIEW_INSTALL_DLL   "${CMAKE_INSTALL_PREFIX}${USR_LIB}")
set (SVIEW_INSTALL_SHARE "${CMAKE_INSTALL_PREFIX}${USR_SHARE}")
set (SVIEW_INSTALL_RES   "${CMAKE_INSTALL_PREFIX}${USR_SHARE}${USR_SVIEW}")

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
if (ANDROID)
  set (aLangMap_english  English)
  set (aLangMap_spanish  Spanish)
  set (aLangMap_russian  Russian)
  set (aLangMap_french   French)
  set (aLangMap_german   German)
  set (aLangMap_chinese  ChineseS)
  set (aLangMap_chineset ChineseT)
  set (aLangMap_korean   Korean)
  set (aLangMap_czech    Czech)
endif()
foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
  foreach (aLangIter ${aLangList})
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/lang/${aLangMap_${aLangIter}}")
  endforeach()
endforeach()

# function to override output directories
function (st_set_target_output_dirs theTarget)
  # move sub-projects outputs into common folder
  set_target_properties(${theTarget} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release${SVIEW_BUNDLE_BIN}/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release${SVIEW_BUNDLE_BIN}/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release${SVIEW_BUNDLE_BIN}/"
    PDB_OUTPUT_DIRECTORY_RELEASE     "${CMAKE_BINARY_DIR}/Release${SVIEW_BUNDLE_BIN}/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug${SVIEW_BUNDLE_BIN}/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug${SVIEW_BUNDLE_BIN}/"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug${SVIEW_BUNDLE_BIN}/"
    PDB_OUTPUT_DIRECTORY_DEBUG       "${CMAKE_BINARY_DIR}/Debug${SVIEW_BUNDLE_BIN}/"
  )

  if (APPLE)
    if (NOT "${INSTALL_NAME_DIR}" STREQUAL "")
      #set_target_properties (${theTarget} PROPERTIES BUILD_WITH_INSTALL_RPATH 1 INSTALL_NAME_DIR "${INSTALL_NAME_DIR}")
      set_target_properties (${theTarget} PROPERTIES BUILD_WITH_INSTALL_NAME_DIR 1 INSTALL_NAME_DIR "${INSTALL_NAME_DIR}")
    endif()
  endif()

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
        configure_file ("${PROJECT_SOURCE_DIR}/lang/${aLangIter}/${theTarget}.lng" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/lang/${aLangMap_${aLangIter}}/" COPYONLY)
        if (EXISTS "${PROJECT_SOURCE_DIR}/lang/${aLangIter}/language.lng")
          configure_file ("${PROJECT_SOURCE_DIR}/lang/${aLangIter}/language.lng" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/lang/${aLangMap_${aLangIter}}/" COPYONLY)
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
  if (MSVC)
    install (FILES "$<TARGET_PDB_FILE:${theTarget}>" DESTINATION "${SVIEW_INSTALL_BIN}" OPTIONAL)
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

if (NOT ANDROID)
  add_subdirectory(StMonitorsDump)
  add_subdirectory(StTests)
endif()
add_subdirectory(sview)

# CAD Viewer
# find Open CASCADE Technology
#include (adm/cmake/FindOpenCASCADE.cmake)
#if (NOT OpenCASCADE_FOUND)
#  message (FATAL_ERROR "could not find OpenCASCADE, please set OpenCASCADE_DIR variable" )
#else()
#  message (STATUS "OpenCASCADE_INCLUDE_DIR=${OpenCASCADE_INCLUDE_DIR}")
#  message (STATUS "OpenCASCADE_LIBRARY_DIR=${OpenCASCADE_LIBRARY_DIR}")
#endif()
#add_subdirectory(StCADViewer)

# install common resources
if (USE_GTK OR USE_XLIB)
  install (DIRECTORY "${CMAKE_SOURCE_DIR}/share/application-registry" DESTINATION "${SVIEW_INSTALL_SHARE}")
  install (DIRECTORY "${CMAKE_SOURCE_DIR}/share/applications"         DESTINATION "${SVIEW_INSTALL_SHARE}")
  install (DIRECTORY "${CMAKE_SOURCE_DIR}/share/icons"                DESTINATION "${SVIEW_INSTALL_SHARE}")
  install (DIRECTORY "${CMAKE_SOURCE_DIR}/share/menu"                 DESTINATION "${SVIEW_INSTALL_SHARE}")
  install (DIRECTORY "${CMAKE_SOURCE_DIR}/share/sView/icons"          DESTINATION "${SVIEW_INSTALL_RES}")
  install (DIRECTORY "${CMAKE_SOURCE_DIR}/share/sView/demo"           DESTINATION "${SVIEW_INSTALL_RES}")
else()
  install (FILES "${CMAKE_SOURCE_DIR}/share/sView/demo/demo.jps"       DESTINATION "${SVIEW_INSTALL_RES}")
  install (FILES "${CMAKE_SOURCE_DIR}/share/sView/demo/demo_robot.jps" DESTINATION "${SVIEW_INSTALL_RES}")
endif()
install (DIRECTORY "${CMAKE_SOURCE_DIR}/textures"                   DESTINATION "${SVIEW_INSTALL_RES}")
install (DIRECTORY "${CMAKE_SOURCE_DIR}/distribution/info"          DESTINATION "${SVIEW_INSTALL_RES}")
install (FILES     "${CMAKE_SOURCE_DIR}/docs/license-gpl-3.0.txt"   DESTINATION "${SVIEW_INSTALL_RES}/info/" RENAME "license.txt")
install (DIRECTORY "${CMAKE_BINARY_DIR}/Release/lang"               DESTINATION "${SVIEW_INSTALL_RES}")
install (DIRECTORY "${CMAKE_BINARY_DIR}/Release/shaders"            DESTINATION "${SVIEW_INSTALL_RES}")
install (DIRECTORY "${CMAKE_BINARY_DIR}/Release/web"                DESTINATION "${SVIEW_INSTALL_RES}")
if (WIN32)
  # association icons
  install (FILES "${CMAKE_SOURCE_DIR}/distribution/media/sView_JPS.ico"   DESTINATION "${SVIEW_INSTALL_RES}/icons/")
  install (FILES "${CMAKE_SOURCE_DIR}/distribution/media/sView_PNS.ico"   DESTINATION "${SVIEW_INSTALL_RES}/icons/")
  install (FILES "${CMAKE_SOURCE_DIR}/distribution/media/sView_Media.ico" DESTINATION "${SVIEW_INSTALL_RES}/icons/")

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
    install (FILES "${anMsvcrDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
    install (FILES "${anMsvcpDll}" DESTINATION "${SVIEW_INSTALL_DLL}")
  endif()
endif()

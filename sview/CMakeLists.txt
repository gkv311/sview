project (sView)

set (USED_SRCFILES
  main.cpp
  StMultiApp.cpp
)
set (USED_MMFILES
  main.ObjC.mm
)

set (USED_INCFILES
  StAppResponder.h
  StMultiApp.h
)

set (USED_MANFILES "")
set (USED_RESFILES
  sview.rc
)
if (WIN32)
  set (USED_MANFILES "../adm/cmake/dpiAware.manifest")
endif()

if (NOT APPLE)
  foreach (aFileIter ${USED_MMFILES})
    set_source_files_properties (${aFileIter} PROPERTIES HEADER_FILE_ONLY TRUE)
  endforeach()
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES} ${USED_MMFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES} ${USED_MANFILES})

if (APPLE)
endif()

# library to build
add_executable (${PROJECT_NAME} WIN32
  ${USED_SRCFILES} ${USED_MMFILES} ${USED_INCFILES} ${USED_RESFILES} ${USED_MANFILES}
)

if (WIN32)
  # add_executable() has WIN32 argument which cannot be switched between release/debug...
  # if ST_DEBUG
  string (REGEX MATCH "/subsystem:windows" ISFLAG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  if (ISFLAG)
    string (REGEX REPLACE "/subsystem:windows" "/subsystem:console" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  else()
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /subsystem:console")
  endif()

  string (REGEX MATCH "/subsystem:console" ISFLAG "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  if (ISFLAG)
    string (REGEX REPLACE "/subsystem:console" "/subsystem:windows" CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  else()
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /subsystem:windows")
  endif()
endif()

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Apps")
st_set_target_output_dirs(${PROJECT_NAME})

# internal dependencies
set (aDeps StOutPageFlip StMoviePlayer StImageViewer StDiagnostics StCore StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
target_link_libraries (${PROJECT_NAME} PRIVATE freetype)

if (APPLE)
  target_link_libraries (${PROJECT_NAME} PRIVATE "-framework Appkit" objc)
endif()

if (APPLE)
  # pre-compile main menu at configuration time
  find_program (IBTOOL ibtool REQUIRED)
  execute_process (COMMAND ${IBTOOL} --compile ${CMAKE_CURRENT_BINARY_DIR}/MainMenu.nib ${CMAKE_CURRENT_SOURCE_DIR}/Resources/English.lproj/MainMenu.xib)
endif()

# copy common resources to build folder
file (GLOB TEXTURE_FILES ../textures/*.png)
foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
  file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_BIN}/textures")
  #file (CREATE_LINK "${PROJECT_SOURCE_DIR}/textures" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_BIN}/textures" SYMBOLIC)
  foreach (aFileIter ${TEXTURE_FILES})
    #message (FATAL_ERROR "aFileIter= ${aFileIter}")
    configure_file ("${aFileIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_BIN}/textures/" COPYONLY)
  endforeach()
endforeach()

# copy macOS bundle resources
if (APPLE)
  foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Frameworks")
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Resources/English.lproj")
    configure_file ("Contents/PkgInfo"           "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/" COPYONLY)
    configure_file ("Contents/Info.plist"        "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/" COPYONLY)
    configure_file ("Resources/sView.icns"       "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Resources/" COPYONLY)
    configure_file ("Resources/sView_Media.icns" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Resources/" COPYONLY)

    # copy pre-compiled main menu
    configure_file (${CMAKE_CURRENT_BINARY_DIR}/MainMenu.nib "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Resources/English.lproj/" COPYONLY)
  endforeach()
endif()

# compile main menu for bundle
if (APPLE)
  #add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
  #  COMMAND ${IBTOOL} --compile "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/English.lproj/MainMenu.nib"
  #          "${CMAKE_CURRENT_SOURCE_DIR}/Resources/English.lproj/MainMenu.xib")
endif()

# copy OpenAL resources
if (APPLE)
  file (GLOB OPENAL_HRFT_FILES ${OPENAL_LIBRARY_DIR}/../share/openal/hrtf/*.mhr)
  file (GLOB OPENAL_PRST_FILES ${OPENAL_LIBRARY_DIR}/../share/openal/presets/*.ambdec)
  foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_BIN}/openal/hrtf")
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_BIN}/openal/presets")
    foreach (aFileIter ${OPENAL_HRFT_FILES})
      configure_file ("${aFileIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_BIN}/openal/hrtf/" COPYONLY)
    endforeach()
    foreach (aFileIter ${OPENAL_PRST_FILES})
      configure_file ("${aFileIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_BIN}/openal/presets/" COPYONLY)
    endforeach()
  endforeach()
endif()

# copy libraries to bundle
if (APPLE)
  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -R -f "${FREETYPE_LIBRARY_DIR}/libfreetype*.dylib" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks")
  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -R -f "${OPENAL_LIBRARY_DIR}/libopenal*.dylib" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks")

  foreach (aLibIter ${FFMPEG_TKLIST})
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND cp -R -f "${FFMPEG_LIBRARY_DIR}/lib${aLibIter}*.dylib" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks")
  endforeach()
endif()

# install target
install (TARGETS "${PROJECT_NAME}"
         RUNTIME DESTINATION "${SVIEW_INSTALL_BIN}"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE)

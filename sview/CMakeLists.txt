project (sView)

set (USED_SRCFILES
  main.cpp
  StMultiApp.cpp
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
  set (USED_MANFILES "../dpiAware.manifest")
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES} ${USED_MANFILES})

# library to build
add_executable (${PROJECT_NAME} WIN32
  ${USED_SRCFILES} ${USED_INCFILES} ${USED_RESFILES} ${USED_MANFILES}
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

# copy common resources to build folder
file (GLOB TEXTURE_FILES ../textures/*.png)
foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
  file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}/textures")
  #file (CREATE_LINK "${PROJECT_SOURCE_DIR}/textures" "${CMAKE_BINARY_DIR}/${aCfgIter}/textures" SYMBOLIC)
  foreach (aFileIter ${TEXTURE_FILES})
    #message (FATAL_ERROR "aFileIter= ${aFileIter}")
    configure_file ("${aFileIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}/textures/" COPYONLY)
  endforeach()
endforeach()

# install target
install (TARGETS "${PROJECT_NAME}"
         RUNTIME DESTINATION "${SVIEW_INSTALL_BIN}"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE)

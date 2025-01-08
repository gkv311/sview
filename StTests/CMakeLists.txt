project (StTests)

set (USED_SRCFILES
  main.cpp
  StTestEmbed.cpp
  StTestGlBand.cpp
  StTestGlStress.cpp
  StTestImageLib.cpp
  StTestMutex.cpp
)
set (USED_MMFILES
  main.mm
  StTestEmbed.ObjC.mm
)

set (USED_INCFILES
  StTest.h
  StTestEmbed.h
  StTestGlBand.h
  StTestGlStress.h
  StTestImageLib.h
  StTestMutex.h
  StTestResponder.h
)

set (USED_MANFILES "")
set (USED_RESFILES "")
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

# library to build
add_executable (${PROJECT_NAME}
  ${USED_SRCFILES} ${USED_MMFILES} ${USED_INCFILES} ${USED_RESFILES} ${USED_MANFILES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Tools")
st_set_target_output_dirs(${PROJECT_NAME})

# internal dependencies
set (aDeps StCore StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
target_link_libraries (${PROJECT_NAME} PRIVATE avutil)
if (USE_XLIB)
  target_link_libraries (${PROJECT_NAME} PRIVATE X11::Xrandr X11::Xext X11::Xpm X11::X11)
endif()
target_link_libraries (${PROJECT_NAME} PRIVATE ${ST_OPENGL_LIBRARIES})
if (APPLE)
  target_link_libraries (${PROJECT_NAME} PRIVATE "-framework CoreFoundation" "-framework Appkit" objc)
endif()

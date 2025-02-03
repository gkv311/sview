project (StDiagnostics)

set (USED_SRCFILES
  StColorProgram.cpp
  StDiagnostics.cpp
  StDiagnosticsGUI.cpp
  StGeometryTest.cpp
)

set (USED_INCFILES
  StColorProgram.h
  StDiagnostics.h
  StDiagnosticsGUI.h
  StGeometryTest.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES "StDiagnostics.rc")
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES})

# library to build
add_library (${PROJECT_NAME} SHARED
  ${USED_SRCFILES} ${USED_INCFILES} ${USED_RESFILES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Apps")
sview_set_target_output_dirs(${PROJECT_NAME})
if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_COMMAND "$<TARGET_FILE:sView.com>"
    VS_DEBUGGER_COMMAND_ARGUMENTS  "--in=diag"
  )
endif()

# internal dependencies
set (aDeps StGLWidgets StOutAnaglyph StOutDual StOutIZ3D StOutInterlace StOutPageFlip StOutDistorted StCore StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
target_link_libraries (${PROJECT_NAME} PRIVATE freetype)
target_link_libraries (${PROJECT_NAME} PRIVATE ${ST_OPENGL_LIBRARIES})

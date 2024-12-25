project (StOutPageFlip)

set (USED_SRCFILES
  StDXAqbsControl.cpp
  StDXManager.cpp
  StDXNVSurface.cpp
  StDXNVWindow.cpp
  StGLColoredLine.cpp
  StGLControlED.cpp
  StOutPageFlip.cpp
  StOutPageFlipExt.cpp
  StQuadBufferCheck.cpp
)
set (USED_MMFILES
  StQuadBufferCheck.ObjC.mm
)

set (USED_INCFILES
  StDXAqbsControl.h
  StDXInfo.h
  StDXManager.h
  StDXNVSurface.h
  StDXNVWindow.h
  StGLColoredLine.h
  StGLControlED.h
  StGLDeviceControl.h
  StOutPageFlip.h
  StOutPageFlipExt.h
  StOutPageFlipStrings.h
  StQuadBufferCheck.h
  StVuzixSDK.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES "StOutPageFlip.rc")
endif()

if (NOT APPLE)
  foreach (aFileIter ${USED_MMFILES})
    set_source_files_properties (${aFileIter} PROPERTIES HEADER_FILE_ONLY TRUE)
  endforeach()
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES} ${USED_MMFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES})

# library to build
add_library (${PROJECT_NAME} SHARED
  ${USED_SRCFILES} ${USED_MMFILES} ${USED_INCFILES} ${USED_RESFILES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Outputs")
sview_set_target_output_dirs(${PROJECT_NAME})

# internal dependencies
set (aDeps StCore StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
include_directories (../3rdparty/include)
target_link_libraries (${PROJECT_NAME} PRIVATE avutil)
if (WIN32)
  target_link_libraries (${PROJECT_NAME} PRIVATE D3D9)
  target_link_libraries (${PROJECT_NAME} PRIVATE nvapi)
endif()
target_link_libraries (${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})
if (USE_XLIB)
  target_link_libraries (${PROJECT_NAME} PRIVATE X11::X11)
endif()

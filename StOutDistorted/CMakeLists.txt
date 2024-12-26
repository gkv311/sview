project (StOutDistorted)

set (USED_SRCFILES
  StOutDistorted.cpp
  StProgramBarrel.cpp
  StProgramFlat.cpp
)

set (USED_INCFILES
  StOutDistorted.h
  StProgramBarrel.h
  StProgramFlat.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES "StOutDistorted.rc")
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES})

# library to build
add_library (${PROJECT_NAME} SHARED
  ${USED_SRCFILES} ${USED_INCFILES} ${USED_RESFILES}
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
target_link_libraries (${PROJECT_NAME} PRIVATE avutil)
if (USE_OPENVR)
  target_link_libraries (${PROJECT_NAME} PRIVATE openvr_api)
endif()
target_link_libraries (${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})

project (StOutInterlace)

set (USED_SRCFILES
  StOutInterlace.cpp
)

set (USED_INCFILES
  StOutInterlace.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES "StOutInterlace.rc")
endif()

set (USED_SHADERFILES
  fEDinterlace.shf
  fEDoff.shf
  vED.shv
)
foreach (aShaderIter ${USED_SHADERFILES})
  set (USED_RESFILES ${USED_RESFILES} "shaders/${aShaderIter}")
endforeach()

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
target_link_libraries (${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})

# copy resources to build folder
foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
  foreach (aShaderIter ${USED_SHADERFILES})
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/shaders/${aShaderIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}/shaders/${PROJECT_NAME}/${aShaderIter}" COPYONLY)
  endforeach()
endforeach()

project (npStBrowserPlugin)

set (USED_SRCFILES
  StActiveXCtrl.cpp
  npEntry.cpp
  npnGate.cpp
  nppGate.cpp
  StActiveXModule.cpp
  StBrowserPlugin.cpp
)

set (USED_INCFILES
  npapi.h
  npfunctions.h
  npplat.h
  npruntime.h
  nptypes.h
  NSPluginBase.h
  StActiveXCtrl.h
  StActiveXModule.h
  StBrowserPlugin.h
  StBrPluginInfo.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES
    StBrowserPlugin.rc
    StBrowserPlugin.def
  )
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES})

set(MIDL_FILE   ${CMAKE_CURRENT_SOURCE_DIR}/StActiveX.idl)
set(MIDL_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/StActiveX.idl.tlb)
if (WIN32)
  add_custom_command(
    OUTPUT ${MIDL_OUTPUT}
    COMMAND midl /nologo /win32 /W1 /client none /tlb "${MIDL_OUTPUT}" "${MIDL_FILE}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${MIDL_FILE}
    MAIN_DEPENDENCY ${MIDL_FILE}
    VERBATIM
  )

  add_compile_definitions(_USRDLL _ATL_CSTRING_EXPLICIT_CONSTRUCTORS _AFXDLL)
  add_compile_definitions(ST_OCX_TLB_PATH="${MIDL_OUTPUT}")
endif()

# library to build
add_library (${PROJECT_NAME} SHARED
  ${USED_SRCFILES} ${USED_INCFILES} ${USED_RESFILES}
)
if (WIN32)
  target_sources(${PROJECT_NAME} PRIVATE ${MIDL_FILE} ${MIDL_OUTPUT})
endif()

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Plugins")
sview_set_target_output_dirs(${PROJECT_NAME})

# internal dependencies
set (aDeps StOutPageFlip StImageViewer StCore StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
target_link_libraries (${PROJECT_NAME} PRIVATE freetype)

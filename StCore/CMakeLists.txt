project (StCore)

set (USED_SRCFILES
  StADLsdk.cpp
  StAndroidGlue.cpp
  StApplication.cpp
  StKeysState.cpp
  StSearchMonitors.cpp
  StWindow.cpp
  StWindowImpl.cpp
  StWindowImplAnd.cpp
  StWindowImplLin.cpp
  StWindowImplWin.cpp
  StWinHandles.cpp
  StXDisplay.cpp
)

set (USED_INCFILES
  ../include/StCore/StAndroidGlue.h
  ../include/StCore/StApplication.h
  ../include/StCore/StEDIDParser.h
  ../include/StCore/StEvent.h
  ../include/StCore/StKeysState.h
  ../include/StCore/StMonitor.h
  ../include/StCore/StNativeWin_t.h
  ../include/StCore/StOpenInfo.h
  ../include/StCore/StSearchMonitors.h
  ../include/StCore/StVirtualKeys.h
  ../include/StCore/StWindow.h
  ../include/StCore/StWinErrorCodes.h
  StADLsdk.h
  StCocoaView.h
  StCocoaWin.h
  StEventsBuffer.h
  stvkeysandroid.h
  stvkeyscarbon.h
  stvkeysxarray.h
  StWindowImpl.h
  StWinHandles.h
  StXDisplay.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES "StCore.rc")
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES})

# library to build
add_library (${PROJECT_NAME} SHARED
  ${USED_SRCFILES} ${USED_INCFILES} ${USED_RESFILES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Core")
sview_set_target_output_dirs(${PROJECT_NAME})

# internal dependencies
set (aDeps StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
include_directories (../3rdparty/include)
if (WIN32)
  target_link_libraries (${PROJECT_NAME} PRIVATE user32 gdi32 Advapi32 Comdlg32 Shell32 Winmm)
  target_link_libraries (${PROJECT_NAME} PRIVATE nvapi)
endif()
target_link_libraries (${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})
if (USE_XLIB)
  target_link_libraries (${PROJECT_NAME} PRIVATE X11::Xrandr X11::Xext X11::Xpm X11::X11)
endif()
if (USE_LIBCONFIG)
  target_link_libraries (${PROJECT_NAME} PRIVATE libconfig++)
endif()

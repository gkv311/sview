project (StGLWidgets)

set (USED_MMFILES
  StCocoaCoords.mm
  StCocoaLocalPool.mm
  StFileNode.ObjC.mm
  StLogger.ObjC.mm
  StPListImpl.mm
  StSocket.ObjC.mm
  StStringUnicode.ObjC.mm
)

set (USED_SRCFILES
  StGLAssignHotKey.cpp
  StGLButton.cpp
  StGLCheckbox.cpp
  StGLCheckboxTextured.cpp
  StGLCombobox.cpp
  StGLDescription.cpp
  StGLFpsLabel.cpp
  StGLImageProgram.cpp
  StGLImageRegion.cpp
  StGLMenu.cpp
  StGLMenuCheckbox.cpp
  StGLMenuItem.cpp
  StGLMenuProgram.cpp
  StGLMenuRadioButton.cpp
  StGLMessageBox.cpp
  StGLMsgStack.cpp
  StGLOpenFile.cpp
  StGLPlayList.cpp
  StGLRadioButton.cpp
  StGLRadioButtonFloat32.cpp
  StGLRadioButtonTextured.cpp
  StGLRangeFieldFloat32.cpp
  StGLRootWidget.cpp
  StGLScrollArea.cpp
  StGLSeekBar.cpp
  StGLSubtitles.cpp
  StGLSwitchTextured.cpp
  StGLTable.cpp
  StGLTextArea.cpp
  StGLTextBorderProgram.cpp
  StGLTextProgram.cpp
  StGLTextureButton.cpp
  StGLWidget.cpp
  StGLWidgetList.cpp
  StSubQueue.cpp
)

set (USED_INCFILES
  ../include/StGLWidgets/StGLAssignHotKey.h
  ../include/StGLWidgets/StGLButton.h
  ../include/StGLWidgets/StGLCheckbox.h
  ../include/StGLWidgets/StGLCheckboxTextured.h
  ../include/StGLWidgets/StGLCombobox.h
  ../include/StGLWidgets/StGLCorner.h
  ../include/StGLWidgets/StGLDescription.h
  ../include/StGLWidgets/StGLFpsLabel.h
  ../include/StGLWidgets/StGLImageProgram.h
  ../include/StGLWidgets/StGLImageRegion.h
  ../include/StGLWidgets/StGLMenu.h
  ../include/StGLWidgets/StGLMenuCheckbox.h
  ../include/StGLWidgets/StGLMenuItem.h
  ../include/StGLWidgets/StGLMenuProgram.h
  ../include/StGLWidgets/StGLMenuRadioButton.h
  ../include/StGLWidgets/StGLMessageBox.h
  ../include/StGLWidgets/StGLMsgStack.h
  ../include/StGLWidgets/StGLOpenFile.h
  ../include/StGLWidgets/StGLPlayList.h
  ../include/StGLWidgets/StGLRadioButton.h
  ../include/StGLWidgets/StGLRadioButtonFloat32.h
  ../include/StGLWidgets/StGLRadioButtonTextured.h
  ../include/StGLWidgets/StGLRangeFieldFloat32.h
  ../include/StGLWidgets/StGLRootWidget.h
  ../include/StGLWidgets/StGLScrollArea.h
  ../include/StGLWidgets/StGLSeekBar.h
  ../include/StGLWidgets/StGLShare.h
  ../include/StGLWidgets/StGLSubtitles.h
  ../include/StGLWidgets/StGLSwitchTextured.h
  ../include/StGLWidgets/StGLTable.h
  ../include/StGLWidgets/StGLTextArea.h
  ../include/StGLWidgets/StGLTextBorderProgram.h
  ../include/StGLWidgets/StGLTextProgram.h
  ../include/StGLWidgets/StGLTextureButton.h
  ../include/StGLWidgets/StGLWidget.h
  ../include/StGLWidgets/StGLWidgetList.h
  ../include/StGLWidgets/StSubQueue.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES "StGLWidgets.rc")
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES} ${USED_MMFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES})

if (APPLE)
  set (USED_SRCFILES ${USED_SRCFILES} ${USED_MMFILES})
endif()

# library to build
add_library (${PROJECT_NAME} SHARED
  ${USED_SRCFILES} ${USED_INCFILES} ${USED_RESFILES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Core")
sview_set_target_output_dirs(${PROJECT_NAME})

# internal dependencies
set (aDeps StCore StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
target_link_libraries (${PROJECT_NAME} PRIVATE avutil)
target_link_libraries (${PROJECT_NAME} PRIVATE freetype)
target_link_libraries (${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})

project (StMoviePlayer)

set (USED_SRCFILES
  StVideo/StALContext.cpp
  StVideo/StAudioQueue.cpp
  StVideo/StAVPacketQueue.cpp
  StVideo/StParamActiveStream.cpp
  StVideo/StPCMBuffer.cpp
  StVideo/StSubtitleQueue.cpp
  StVideo/StSubtitlesASS.cpp
  StVideo/StVideo.cpp
  StVideo/StVideoDxva2.cpp
  StVideo/StVideoQueue.cpp
  StVideo/StVideoTimer.cpp
  StVideo/StVideoToolbox.cpp
  StALDeviceParam.cpp
  StMovieOpenDialog.cpp
  StMoviePlayer.cpp
  StMoviePlayerGUI.cpp
  StMoviePlayerStrings.cpp
  stMongoose.c
)

set (USED_INCFILES
  StVideo/StALContext.h
  StVideo/StAudioQueue.h
  StVideo/StAVPacketQueue.h
  StVideo/StParamActiveStream.h
  StVideo/StPCMBuffer.h
  StVideo/StSubtitleQueue.h
  StVideo/StSubtitlesASS.h
  StVideo/StVideo.h
  StVideo/StVideoQueue.h
  StVideo/StVideoTimer.h
  StALDeviceParam.h
  StMovieOpenDialog.h
  StMoviePlayer.h
  StMoviePlayerGUI.h
  StMoviePlayerInfo.h
  StMoviePlayerStrings.h
  StTimeBox.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES "StMoviePlayer.rc")
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
    VS_DEBUGGER_COMMAND_ARGUMENTS  "--in=video"
  )
endif()

# internal dependencies
set (aDeps StGLWidgets StOutAnaglyph StOutDual StOutIZ3D StOutInterlace StOutPageFlip StOutDistorted StCore StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
include_directories (../3rdparty/include)
target_link_libraries (${PROJECT_NAME} PRIVATE avformat avcodec swscale avutil)
target_link_libraries (${PROJECT_NAME} PRIVATE freetype)
target_link_libraries (${PROJECT_NAME} PRIVATE openal)
target_link_libraries (${PROJECT_NAME} PRIVATE ${ST_OPENGL_LIBRARIES})
if (APPLE)
  target_link_libraries (${PROJECT_NAME} PRIVATE "-framework CoreVideo" objc)
endif()

# copy web resources to build folder
file (GLOB TEXTURE_FILES ../textures/*.svg)
foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
  file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/textures")
  foreach (aFileIter ${TEXTURE_FILES})
    configure_file ("${aFileIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/textures/" COPYONLY)
  endforeach()

  file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/web")
  configure_file ("web/index.htm"   "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/web/" COPYONLY)
  configure_file ("web/favicon.ico" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/web/" COPYONLY)
endforeach()

project (StShared)

set (USED_SRCFILES
  StAV/stAV.cpp
  StAV/StAVFrame.cpp
  StAV/StAVImage.cpp
  StAV/StAVIOContext.cpp
  StAV/StAVIOFileContext.cpp
  StAV/StAVIOJniHttpContext.cpp
  StAV/StAVIOMemContext.cpp
  StAV/StAVPacket.cpp
  StAV/StAVVideoMuxer.cpp
  StFile/StFileNode.cpp
  StFile/StFileNode2.cpp
  StFile/StFolder.cpp
  StFile/StRawFile.cpp
  StFT/StFTFont.cpp
  StFT/StFTFontRegistry.cpp
  StFT/StFTLibrary.cpp
  StGL/StGLCircle.cpp
  StGL/StGLContext.cpp
  StGL/StGLFont.cpp
  StGL/StGLFontEntry.cpp
  StGL/StGLFontManager.cpp
  StGL/StGLFrameBuffer.cpp
  StGL/StGLMatrix.cpp
  StGL/StGLMesh.cpp
  StGL/StGLPrism.cpp
  StGL/StGLProgram.cpp
  StGL/StGLProjCamera.cpp
  StGL/StGLQuadTexture.cpp
  StGL/StGLQuads.cpp
  StGL/StGLResource.cpp
  StGL/StGLShader.cpp
  StGL/StGLStereoFrameBuffer.cpp
  StGL/StGLTextFormatter.cpp
  StGL/StGLTexture.cpp
  StGL/StGLTextureData.cpp
  StGL/StGLTextureQueue.cpp
  StGL/StGLUVCylinder.cpp
  StGL/StGLUVSphere.cpp
  StGL/StGLVertexBuffer.cpp
  StImage/StDevILImage.cpp
  StImage/StExifDir.cpp
  StImage/StExifTags.cpp
  StImage/StFreeImage.cpp
  StImage/StImage.cpp
  StImage/StImageFile.cpp
  StImage/StImagePlane.cpp
  StImage/StJpegParser.cpp
  StImage/StStbImage.cpp
  StImage/StWicImage.cpp
  StSettings/StConfigImpl.cpp
  StSettings/StRegisterImpl.cpp
  StSettings/StSettings.cpp
  StSettings/StTranslations.cpp
  StStrings/stConsole.cpp
  StStrings/StDictionary.cpp
  StStrings/StLangMap.cpp
  StStrings/StLogger.cpp
  StStrings/stUtfTools.cpp
  StAction.cpp
  StBndBox.cpp
  StBndCameraBox.cpp
  StBndSphere.cpp
  StCondition.cpp
  StEDIDParser.cpp
  StFormatEnum.cpp
  StJNIEnv.cpp
  StLibrary.cpp
  StMinGen.cpp
  StMonitor.cpp
  StMsgQueue.cpp
  StMutex.cpp
  StPlayList.cpp
  StProcess.cpp
  StProcess2.cpp
  StResourceManager.cpp
  StThread.cpp
  StVirtualKeys.cpp
)
set (USED_MMFILES
  StFile/StFileNode.ObjC.mm
  StSettings/StPListImpl.mm
  StStrings/StLogger.ObjC.mm
  StStrings/StStringUnicode.ObjC.mm
  StCocoaCoords.mm
  StCocoaLocalPool.mm
  StProcess.ObjC.mm
)

set (USED_INCFILES
  ../include/StAV/stAV.h
  ../include/StAV/StAVBufferPool.h
  ../include/StAV/StAVFrame.h
  ../include/StAV/StAVImage.h
  ../include/StAV/StAVIOContext.h
  ../include/StAV/StAVIOFileContext.h
  ../include/StAV/StAVIOJniHttpContext.h
  ../include/StAV/StAVIOMemContext.h
  ../include/StAV/StAVPacket.h
  ../include/StAV/StAVVideoMuxer.h
  ../include/StCocoa/StCocoaCoords.h
  ../include/StCocoa/StCocoaLocalPool.h
  ../include/StCocoa/StCocoaString.h
  ../include/StFile/StFileNode.h
  ../include/StFile/StFolder.h
  ../include/StFile/StMIME.h
  ../include/StFile/StMIMEList.h
  ../include/StFile/StNode.h
  ../include/StFile/StRawFile.h
  ../include/StFT/StFTFont.h
  ../include/StFT/StFTFontRegistry.h
  ../include/StFT/StFTLibrary.h
  ../include/StJNI/StJNIEnv.h
  ../include/StGL/StGLArbFbo.h
  ../include/StGL/StGLBrightnessMatrix.h
  ../include/StGL/StGLContext.h
  ../include/StGL/StGLDeviceCaps.h
  ../include/StGL/StGLEnums.h
  ../include/StGL/StGLExt.h
  ../include/StGL/StGLFont.h
  ../include/StGL/StGLFontEntry.h
  ../include/StGL/StGLFontManager.h
  ../include/StGL/StGLFrameBuffer.h
  ../include/StGL/StGLFunctions.h
  ../include/StGL/StGLMatrix.h
  ../include/StGL/StGLProgram.h
  ../include/StGL/StGLProgramMatrix.h
  ../include/StGL/StGLResource.h
  ../include/StGL/StGLResources.h
  ../include/StGL/StGLSaturationMatrix.h
  ../include/StGL/StGLShader.h
  ../include/StGL/StGLTextFormatter.h
  ../include/StGL/StGLTexture.h
  ../include/StGL/StGLVarLocation.h
  ../include/StGL/StGLVec.h
  ../include/StGL/StGLVertexBuffer.h
  ../include/StGL/StParams.h
  ../include/StGL/StPlayList.h
  ../include/StGLCore/StGLCore11.h
  ../include/StGLCore/StGLCore11Fwd.h
  ../include/StGLCore/StGLCore12.h
  ../include/StGLCore/StGLCore13.h
  ../include/StGLCore/StGLCore14.h
  ../include/StGLCore/StGLCore15.h
  ../include/StGLCore/StGLCore20.h
  ../include/StGLCore/StGLCore21.h
  ../include/StGLCore/StGLCore30.h
  ../include/StGLCore/StGLCore31.h
  ../include/StGLCore/StGLCore32.h
  ../include/StGLCore/StGLCore33.h
  ../include/StGLCore/StGLCore40.h
  ../include/StGLCore/StGLCore41.h
  ../include/StGLCore/StGLCore42.h
  ../include/StGLCore/StGLCore43.h
  ../include/StGLCore/StGLCore44.h
  ../include/StGLMesh/StBndBox.h
  ../include/StGLMesh/StBndCameraBox.h
  ../include/StGLMesh/StBndContainer.h
  ../include/StGLMesh/StBndSphere.h
  ../include/StGLMesh/StGLCircle.h
  ../include/StGLMesh/StGLMesh.h
  ../include/StGLMesh/StGLPrism.h
  ../include/StGLMesh/StGLQuads.h
  ../include/StGLMesh/StGLUVCylinder.h
  ../include/StGLMesh/StGLUVSphere.h
  ../include/StGLStereo/StFormatEnum.h
  ../include/StGLStereo/StGLProjCamera.h
  ../include/StGLStereo/StGLQuadTexture.h
  ../include/StGLStereo/StGLStereoFrameBuffer.h
  ../include/StGLStereo/StGLStereoTexture.h
  ../include/StGLStereo/StGLTextureData.h
  ../include/StGLStereo/StGLTextureQueue.h
  ../include/StImage/StDevILImage.h
  ../include/StImage/StExifDir.h
  ../include/StImage/StExifEntry.h
  ../include/StImage/StExifTags.h
  ../include/StImage/StFreeImage.h
  ../include/StImage/StImage.h
  ../include/StImage/StImageFile.h
  ../include/StImage/StImagePlane.h
  ../include/StImage/StJpegParser.h
  ../include/StImage/StPixelRGB.h
  ../include/StImage/StStbImage.h
  ../include/StImage/StWicImage.h
  ../include/StSettings/StEnumParam.h
  ../include/StSettings/StFloat32Param.h  
  ../include/StSettings/StParam.h
  ../include/StSettings/StSettings.h
  ../include/StSettings/StTranslations.h
  ../include/StSlots/StAction.h
  ../include/StSlots/StSignal.h
  ../include/StSlots/StSlot.h
  ../include/StSlots/StSlotFunction.h
  ../include/StSlots/StSlotMethod.h
  ../include/StSlots/StSlotMethodUnsafe.h
  ../include/StSlots/StSlotProxy.h
  ../include/StSlots/StSlotTypes.h
  ../include/StSocket/StCheckUpdates.h
  ../include/StStrings/stConsole.h
  ../include/StStrings/StDictionary.h
  ../include/StStrings/StFormatTime.h
  ../include/StStrings/StLangMap.h
  ../include/StStrings/StLogger.h
  ../include/StStrings/StMsgQueue.h
  ../include/StStrings/StString.h
  ../include/StStrings/StStringStream.h
  ../include/StStrings/StStringUnicode.h
  ../include/StStrings/StUtfIterator.h
  ../include/StStrings/stUtfTools.h
  ../include/StStrings/StStringUnicode.inl
  ../include/StStrings/StUtfIterator.inl
  ../include/StSys/StSys.h
  ../include/StTemplates/StArray.h
  ../include/StTemplates/StArrayList.h
  ../include/StTemplates/StArrayStreamBuffer.h
  ../include/StTemplates/StAtomic.h
  ../include/StTemplates/StHandle.h
  ../include/StTemplates/StQuaternion.h
  ../include/StTemplates/StQuickPointersSort.h
  ../include/StTemplates/StQuickSort.h
  ../include/StTemplates/StRect.h
  ../include/StTemplates/StTemplates.h
  ../include/StTemplates/StVec2.h
  ../include/StTemplates/StVec3.h
  ../include/StTemplates/StVec4.h
  ../include/StThreads/StAtomicOp.h
  ../include/StThreads/StCondition.h
  ../include/StThreads/StFPSControl.h
  ../include/StThreads/StFPSMeter.h
  ../include/StThreads/StMinGen.h
  ../include/StThreads/StMutex.h
  ../include/StThreads/StMutexSlim.h
  ../include/StThreads/StProcess.h
  ../include/StThreads/StResourceManager.h
  ../include/StThreads/StThread.h
  ../include/StThreads/StTimer.h
  ../include/StAlienData.h
  ../include/stAssert.h
  ../include/StLibrary.h
  ../include/stTypes.h
  ../include/StVersion.h
)

set (USED_RESFILES "")
if (WIN32)
  set (USED_RESFILES "StShared.rc")
endif()

if (NOT APPLE)
  foreach (aFileIter ${USED_MMFILES})
    set_source_files_properties (${aFileIter} PROPERTIES HEADER_FILE_ONLY TRUE)
  endforeach()
endif()

add_definitions(-DST_SHARED_DLL)

source_group ("Source Files"   FILES ${USED_SRCFILES} ${USED_MMFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES})

# library to build
add_library (${PROJECT_NAME} SHARED
  ${USED_SRCFILES} ${USED_MMFILES} ${USED_INCFILES} ${USED_RESFILES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Core")
sview_set_target_output_dirs(${PROJECT_NAME})

# external dependencies
target_link_libraries (${PROJECT_NAME} PRIVATE avformat avcodec swscale avutil)
target_link_libraries (${PROJECT_NAME} PRIVATE freetype)
target_link_libraries (${PROJECT_NAME} PRIVATE ${ST_OPENGL_LIBRARIES})
if (WIN32)
  target_link_libraries (${PROJECT_NAME} PRIVATE psapi version user32 gdi32 advapi32 comdlg32 shell32 ole32)
elseif (APPLE)
  target_link_libraries (${PROJECT_NAME} PRIVATE "-framework CoreGraphics" "-framework Appkit" objc)
elseif (ANDROID)
  target_link_libraries (${PROJECT_NAME} PRIVATE log android)
endif()
if (USE_GTK)
  target_link_libraries (${PROJECT_NAME} PRIVATE GTK2::gtk GTK2::gthread)
  include_directories (${GTK2_INCLUDE_DIRS})
endif()
if (USE_XLIB)
  target_link_libraries (${PROJECT_NAME} PRIVATE fontconfig X11::X11)
endif()
if (USE_LIBCONFIG)
  target_link_libraries (${PROJECT_NAME} PRIVATE libconfig++)
endif()

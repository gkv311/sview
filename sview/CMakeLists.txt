project (sView)

set (USED_SRCFILES
  main.cpp
  StMultiApp.cpp
)
set (USED_MMFILES
  main.ObjC.mm
)
set (USED_JNIFILES
  jni/main.cpp
)

set (USED_INCFILES
  StAppResponder.h
  StMultiApp.h
)

set (USED_MANFILES "")
set (USED_RESFILES
  sview.rc
)
if (WIN32)
  set (USED_MANFILES "../adm/cmake/dpiAware.manifest")
endif()

if (NOT APPLE)
  foreach (aFileIter ${USED_MMFILES})
    set_source_files_properties (${aFileIter} PROPERTIES HEADER_FILE_ONLY TRUE)
  endforeach()
endif()

source_group ("Source Files"   FILES ${USED_SRCFILES} ${USED_MMFILES} ${USED_JNIFILES})
source_group ("Header Files"   FILES ${USED_INCFILES})
source_group ("Resource Files" FILES ${USED_RESFILES} ${USED_MANFILES})

# library to build
if (ANDROID)
  set (PROJECT_NAME "sview")
  add_library (${PROJECT_NAME} SHARED
    ${USED_SRCFILES} ${USED_JNIFILES} ${USED_INCFILES} ${USED_RESFILES}
  )
else()
  add_executable (${PROJECT_NAME} WIN32
    ${USED_SRCFILES} ${USED_MMFILES} ${USED_INCFILES} ${USED_RESFILES} ${USED_MANFILES}
  )
endif()

if (WIN32)
  # add_executable() has WIN32 argument which cannot be switched between release/debug...
  # if ST_DEBUG
  string (REGEX MATCH "/subsystem:windows" ISFLAG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  if (ISFLAG)
    string (REGEX REPLACE "/subsystem:windows" "/subsystem:console" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  else()
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /subsystem:console")
  endif()

  string (REGEX MATCH "/subsystem:console" ISFLAG "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  if (ISFLAG)
    string (REGEX REPLACE "/subsystem:console" "/subsystem:windows" CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  else()
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /subsystem:windows")
  endif()
endif()

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "Apps")
st_set_target_output_dirs(${PROJECT_NAME})

# internal dependencies
set (aDeps StOutPageFlip StMoviePlayer StImageViewer StDiagnostics StCore StShared)
foreach (aDepIter ${aDeps})
  add_dependencies (${PROJECT_NAME} ${aDepIter})
  target_link_libraries (${PROJECT_NAME} PRIVATE ${aDepIter})
endforeach()

# external dependencies
target_link_libraries (${PROJECT_NAME} PRIVATE freetype)

if (ANDROID)
  target_link_libraries (${PROJECT_NAME} PRIVATE log android EGL GLESv2 c)
endif()

if (APPLE)
  target_link_libraries (${PROJECT_NAME} PRIVATE "-framework Appkit" objc)
endif()

if (APPLE)
  # pre-compile main menu at configuration time
  find_program (IBTOOL ibtool REQUIRED)
  execute_process (COMMAND ${IBTOOL} --compile ${CMAKE_CURRENT_BINARY_DIR}/MainMenu.nib ${CMAKE_CURRENT_SOURCE_DIR}/Resources/English.lproj/MainMenu.xib)
endif()

# copy common resources to build folder
file (GLOB TEXTURE_FILES ../textures/*.png)
foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
  file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/textures")
  #file (CREATE_LINK "${PROJECT_SOURCE_DIR}/textures" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/textures" SYMBOLIC)
  foreach (aFileIter ${TEXTURE_FILES})
    #message (FATAL_ERROR "aFileIter= ${aFileIter}")
    configure_file ("${aFileIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/textures/" COPYONLY)
  endforeach()
endforeach()

# copy macOS bundle resources
if (APPLE)
  foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Frameworks")
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Resources/English.lproj")
    configure_file ("Contents/PkgInfo"           "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/" COPYONLY)
    configure_file ("Contents/Info.plist"        "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/" COPYONLY)
    configure_file ("Resources/sView.icns"       "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Resources/" COPYONLY)
    configure_file ("Resources/sView_Media.icns" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Resources/" COPYONLY)

    # copy pre-compiled main menu
    configure_file (${CMAKE_CURRENT_BINARY_DIR}/MainMenu.nib "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_EXT}/Contents/Resources/English.lproj/" COPYONLY)
  endforeach()
endif()

# compile main menu for bundle
if (APPLE)
  #add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
  #  COMMAND ${IBTOOL} --compile "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/English.lproj/MainMenu.nib"
  #          "${CMAKE_CURRENT_SOURCE_DIR}/Resources/English.lproj/MainMenu.xib")
endif()

# copy OpenAL resources
if (APPLE)
  file (GLOB OPENAL_HRFT_FILES ${OPENAL_LIBRARY_DIR}/../share/openal/hrtf/*.mhr)
  file (GLOB OPENAL_PRST_FILES ${OPENAL_LIBRARY_DIR}/../share/openal/presets/*.ambdec)
  foreach (aCfgIter ${CMAKE_CONFIGURATION_TYPES})
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/openal/hrtf")
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/openal/presets")
    foreach (aFileIter ${OPENAL_HRFT_FILES})
      configure_file ("${aFileIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/openal/hrtf/" COPYONLY)
    endforeach()
    foreach (aFileIter ${OPENAL_PRST_FILES})
      configure_file ("${aFileIter}" "${CMAKE_BINARY_DIR}/${aCfgIter}${SVIEW_BUNDLE_RES}/openal/presets/" COPYONLY)
    endforeach()
  endforeach()
endif()

# copy libraries to bundle
if (APPLE)
  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -R -f "${FREETYPE_LIBRARY_DIR}/libfreetype*.dylib" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks")
  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -R -f "${OPENAL_LIBRARY_DIR}/libopenal*.dylib" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks")

  foreach (aLibIter ${FFMPEG_TKLIST})
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND cp -R -f "${FFMPEG_LIBRARY_DIR}/lib${aLibIter}*.dylib" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks")
  endforeach()
elseif (ANDROID)
  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -R -f "${FREETYPE_LIBRARY_DIR}/libfreetype.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/")
  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -R -f "${OPENAL_LIBRARY_DIR}/libopenal.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/")

  foreach (aLibIter ${FFMPEG_TKLIST})
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND cp -R -f "${FFMPEG_LIBRARY_DIR}/lib${aLibIter}.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/")
  endforeach()

  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND cp -R -f "${CMAKE_ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/${CMAKE_ANDROID_ARCH_ABI}/libgnustl_shared.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
      #COMMAND ${CMAKE_CXX_ANDROID_TOOLCHAIN_PREFIX}strip${CMAKE_CXX_ANDROID_TOOLCHAIN_SUFFIX} --strip-unneeded "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libgnustl_shared.so"
  )
endif()

# install target
install (TARGETS "${PROJECT_NAME}"
         RUNTIME DESTINATION "${SVIEW_INSTALL_BIN}"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if (MSVC)
  install (FILES "$<TARGET_PDB_FILE:${PROJECT_NAME}>" DESTINATION "${SVIEW_INSTALL_BIN}" OPTIONAL)
endif()

if (ANDROID)
  find_package (Java REQUIRED)
  set (ANDROID_PLATFORM    "android-26.jar" CACHE FILEPATH "Path to android-26.jar")
  set (ANDROID_BUILD_TOOLS "" CACHE PATH "Path to Android build tools (aapt, dx, zipalign, etc.)")
  find_program (TOOL_AAPT     NAMES aapt     PATHS ${ANDROID_BUILD_TOOLS} NO_DEFAULT_PATH)
  find_program (TOOL_DX       NAMES dx       PATHS ${ANDROID_BUILD_TOOLS} NO_DEFAULT_PATH)
  find_program (TOOL_ZIPALIGN NAMES zipalign PATHS ${ANDROID_BUILD_TOOLS} NO_DEFAULT_PATH)
  if (NOT TOOL_AAPT)
    message (FATAL_ERROR "could not find aapt, please set ANDROID_BUILD_TOOLS variable" )
  endif()
  if (NOT TOOL_DX)
    message (FATAL_ERROR "could not find dx, please set ANDROID_BUILD_TOOLS variable" )
  endif()
  if (NOT TOOL_ZIPALIGN)
    message (FATAL_ERROR "could not find zipalign, please set ANDROID_BUILD_TOOLS variable" )
  endif()

  # generate APK manifest file with version codes (should be incremented for each build)
  set (SVIEW_APK_MANIFEST "${CMAKE_BINARY_DIR}/AndroidManifest.xml")
  set (SVIEW_APK_VERSIONCODE "1")
  set (SVIEW_APK_VERSIONNAME "20.1")
  configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/AndroidManifest.xml.in" "${SVIEW_APK_MANIFEST}")

  # compile APK resources
  set (AAPT_RESOURCES_OUTPUT_DIR "${CMAKE_BINARY_DIR}/java/gen")
  set (AAPT_RESOURCES_OUTPUT "${AAPT_RESOURCES_OUTPUT_DIR}/com/sview/R.java")
  add_custom_command (OUTPUT ${AAPT_RESOURCES_OUTPUT} DEPENDS ${SVIEW_APK_MANIFEST} VERBATIM
    COMMAND ${TOOL_AAPT} package -v -f -m -S ${CMAKE_CURRENT_SOURCE_DIR}/res -J ${AAPT_RESOURCES_OUTPUT_DIR} -M ${SVIEW_APK_MANIFEST} -I ${ANDROID_PLATFORM}
    COMMENT "Calling aapt to generate R.java for APK resources...")
  add_custom_target (genjavares ALL DEPENDS ${AAPT_RESOURCES_OUTPUT})

  # compile Java files
  set (USED_JAVAFILES
    src/com/sview/CrashReportActivity.java
    src/com/sview/MainActivity.java
    src/com/sview/StActivity.java
    src/com/sview/StImageActivity.java
    src/com/sview/StMovieActivity.java
    src/com/sview/StMovieService.java
    src/com/sview/StS3dvSurface.java
  )

  set (JAVAC_INPUT)
  set (JAVAC_OUTPUT)
  set (JAVAC_OUTPUT_DIR "${CMAKE_BINARY_DIR}/java/classes")
  foreach (aJavaFile ${USED_JAVAFILES})
    get_filename_component (aFileBase ${aJavaFile} NAME_WE)
    list (APPEND JAVAC_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/com/sview/${aFileBase}.java)
    # actually javac might generate multiple '.class' files out of a single '.java' (like 'StMovieActivity.class', 'StMovieActivity$1.class' and more)
    list (APPEND JAVAC_OUTPUT ${JAVAC_OUTPUT_DIR}/com/sview/${aFileBase}.class)
  endforeach()
  # R.java -> R.class, R$attr.class, R$drawable.class, R$mipmap.class, R$string.class
  list (APPEND JAVAC_INPUT  ${AAPT_RESOURCES_OUTPUT})
  list (APPEND JAVAC_OUTPUT ${JAVAC_OUTPUT_DIR}/com/sview/R.class)

  #set (CMAKE_JAVA_COMPILE_FLAGS "-source" "1.7" "-target" "1.7")
  add_custom_command (OUTPUT ${JAVAC_OUTPUT} DEPENDS ${JAVAC_INPUT} VERBATIM
    COMMAND ${Java_JAVAC_EXECUTABLE} -source 1.7 -target 1.7 -d ${JAVAC_OUTPUT_DIR} -classpath ${ANDROID_PLATFORM} -sourcepath ${CMAKE_CURRENT_SOURCE_DIR}/src:${AAPT_RESOURCES_OUTPUT_DIR} ${JAVAC_INPUT}
    COMMENT "Calling Java compiler ${Java_JAVAC_EXECUTABLE}...")
  add_custom_target (javac ALL DEPENDS ${JAVAC_OUTPUT})

  # compile .class files into .dx
  set (SVIEW_DEX_OUTPUT "${CMAKE_BINARY_DIR}/classes.dex")
  add_custom_command (OUTPUT ${SVIEW_DEX_OUTPUT} DEPENDS ${JAVAC_OUTPUT} VERBATIM
    COMMAND ${TOOL_DX} --dex --verbose --output=${SVIEW_DEX_OUTPUT} ${JAVAC_OUTPUT_DIR}
    COMMENT "Calling dx compiler...")
  add_custom_target (javadx ALL DEPENDS ${SVIEW_DEX_OUTPUT})

  # generate debug APK signing key
  set (ANDROID_KEYSTORE          "${CMAKE_BINARY_DIR}/sview_debug.key" CACHE STRING "Path to keystore file")
  set (ANDROID_KEYSTORE_PASSWORD "sview_pswd"                          CACHE STRING "Keystore password")
  set (ANDROID_KEY               "sview android key"                   CACHE STRING "Signing key name in keystore")
  set (ANDROID_KEY_PASSWORD      "sview_pswd"                          CACHE STRING "Signing key password")
  find_program (TOOL_KEYTOOL NAMES keytool PATHS ${JAVA_HOME})
  add_custom_command (OUTPUT ${ANDROID_KEYSTORE} VERBATIM
    COMMAND ${TOOL_KEYTOOL} -genkeypair -validity 1000 -dname "CN=sview_dummy,O=Android,C=JPN" -keystore ${ANDROID_KEYSTORE} -storepass ${ANDROID_KEYSTORE_PASSWORD} -keypass ${ANDROID_KEY_PASSWORD} -alias ${ANDROID_KEY} -keyalg RSA -v
    COMMENT "Generating debug signing key...")
  add_custom_target (sView_keystore_debug DEPENDS ${ANDROID_KEYSTORE})

  # build unsigned APK file
  set (SVIEW_APK_ROOT     "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../..")
  set (SVIEW_APK_UNSIGNED "${CMAKE_BINARY_DIR}/sView-$<CONFIG>.unsigned.apk.tmp")
  set (SVIEW_APK_SIGNED   "${CMAKE_BINARY_DIR}/sView-$<CONFIG>.signed.apk.tmp")
  set (SVIEW_APK_FINAL    "${CMAKE_BINARY_DIR}/sView-$<CONFIG>.apk")
  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD VERBATIM
    COMMAND ${CMAKE_COMMAND} -E remove -f ${SVIEW_APK_UNSIGNED}
    COMMAND ${CMAKE_COMMAND} -E copy ${SVIEW_DEX_OUTPUT} ${SVIEW_APK_ROOT}/
    #COMMAND ${TOOL_AAPT} package -v -f -M ${SVIEW_APK_MANIFEST} -S ${CMAKE_CURRENT_SOURCE_DIR}/res -I ${ANDROID_PLATFORM} -F ${SVIEW_APK_UNSIGNED} ${SVIEW_APK_ROOT}
    COMMAND ${TOOL_AAPT} package -f -M ${SVIEW_APK_MANIFEST} -S ${CMAKE_CURRENT_SOURCE_DIR}/res -I ${ANDROID_PLATFORM} -F ${SVIEW_APK_UNSIGNED} ${SVIEW_APK_ROOT}
    COMMAND ${Java_JARSIGNER_EXECUTABLE} -sigalg SHA1withRSA -digestalg SHA1 -keystore ${ANDROID_KEYSTORE} -storepass ${ANDROID_KEYSTORE_PASSWORD} -keypass ${ANDROID_KEY_PASSWORD}
            -verbose
            -signedjar ${SVIEW_APK_SIGNED} ${SVIEW_APK_UNSIGNED} ${ANDROID_KEY}
    COMMAND ${TOOL_ZIPALIGN} -v -f 4 ${SVIEW_APK_SIGNED} ${SVIEW_APK_FINAL}
    COMMENT "Building APK...")
  add_dependencies (${PROJECT_NAME} javadx)

endif()

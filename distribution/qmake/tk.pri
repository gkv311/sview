# This is a project template file defining an sView Toolkit.
# This project should be included with predefined ST_TOOLKIT_NAME variable.

#TEMPLATE = lib
TEMPLATE = aux

# Disable some dummy Qt defaults
QT -= core gui
CONFIG -= qt app_bundle
CONFIG -= debug_and_release

sViewRoot = $$_PRO_FILE_PWD_/../../..
DESTDIR = $$sViewRoot/build
LIBS += -L$$sViewRoot/build
exists(custom.pri) { include(custom.pri) }

aSrcListHpp0 = $$files($$sViewRoot/$$ST_TOOLKIT_NAME/*.h,     true)
aSrcListHpp1 = $$files($$sViewRoot/include/$$ST_TOOLKIT_NAME/*.h)
aSrcListCpp  = $$files($$sViewRoot/$$ST_TOOLKIT_NAME/*.cpp,   true)
aSrcListMm   = $$files($$sViewRoot/$$ST_TOOLKIT_NAME/*.mm,    true)
aSrcListJava = $$files($$sViewRoot/$$ST_TOOLKIT_NAME/*.java,  true)
aSrcListRc   = $$files($$sViewRoot/$$ST_TOOLKIT_NAME/*.rc,    true)
aSrcListXml  = $$files($$sViewRoot/$$ST_TOOLKIT_NAME/*.xml,   true)
aSrcListPlist= $$files($$sViewRoot/$$ST_TOOLKIT_NAME/*.plist, true)
INCLUDEPATH += $$sViewRoot/include
INCLUDEPATH += $$sViewRoot/3rdparty/include
HEADERS     += $$aSrcListHpp0
HEADERS     += $$aSrcListHpp1
!equals(TEMPLATE, aux) {
  SOURCES += $$aSrcListCpp
  mac { SOURCES += $$aSrcListMm }
}
OTHER_FILES += $$aSrcListJava
OTHER_FILES += $$aSrcListRc
OTHER_FILES += $$aSrcListXml
OTHER_FILES += $$aSrcListPlist

aLngList = chinese czech english french german korean russian
for (aLngIter, aLngList) {
  aSrcListLng = $$files($$sViewRoot/$$ST_TOOLKIT_NAME/lang/$$aLngIter/*.lng)
  OTHER_FILES += $$aSrcListLng
}

equals(ST_TOOLKIT_NAME, StShared) {
  aSrcListHppX = $$files($$sViewRoot/include/*.h,true)
  HEADERS += $$aSrcListHppX

  DEFINES += ST_SHARED_DLL
} else:equals(ST_TOOLKIT_NAME, sview) {
  # Define sView executable which Qt Creator will automatically use for "Run".
  # Touch dummy.cpp to force QMAKE_POST_LINK redirecting to main Makefile to be executed within each build.
  TEMPLATE = app
  mac {
    TARGET = sView.app/Contents/MacOS/sView
  } else {
    TARGET = sView
  }
  SOURCES += $$_PRO_FILE_PWD_/../dummy.cpp
  QMAKE_POST_LINK += rm $(TARGET); touch $$_PRO_FILE_PWD_/../dummy.cpp;

  # Redirect clean to main Makefile.
  realclean.commands = $(MAKE) --directory $$sViewRoot clean
  clean.depends = realclean
  QMAKE_EXTRA_TARGETS += clean realclean

  # Prepare make arguments
  aNbJobs = $$system(getconf _NPROCESSORS_ONLN)
  ST_MAKE_ARGS = -j$$aNbJobs
  ST_DEBUG = 0
  SVIEW_VER_TYPE =
  CONFIG(debug, debug|release) {
    ST_DEBUG = 1
    SVIEW_VER_TYPE = dev
  }

  # Calculate APK build code based on number of git commits for automatic increment.
  # ST_APK_CODE = NbCommits * 10 + ArchShift
  ST_APK_ARCH_SHIFT = 0
  equals(ANDROID_EABI, armeabi-v7a) { ST_APK_ARCH_SHIFT = 0 }
  equals(ANDROID_EABI, arm64-v8a)   { ST_APK_ARCH_SHIFT = 1 }
  equals(ANDROID_EABI, x86)         { ST_APK_ARCH_SHIFT = 2 }
  equals(ANDROID_EABI, x86_64)      { ST_APK_ARCH_SHIFT = 3 }
  ST_REVISION = $$system(git rev-list --count HEAD)
  SVIEW_APK_CODE = $$system(echo $((10 * $${ST_REVISION} + $${ST_APK_ARCH_SHIFT})))
  SVIEW_SDK_VER_STRING = $$system(date +%y.%m)$${SVIEW_VER_TYPE}
  #message("SVIEW_APK_CODE=$${SVIEW_APK_CODE}")
  #message("SVIEW_SDK_VER_STRING=$${SVIEW_SDK_VER_STRING}")

  #ST_MAKE_TARGET = android
  !isEmpty(ST_MAKE_TARGET) { ST_MAKE_ARGS += $$ST_MAKE_TARGET }
  aMakeEnvList = ST_DEBUG SVIEW_APK_CODE SVIEW_SDK_VER_STRING ANDROID_NDK ANDROID_BUILD_TOOLS ANDROID_PLATFORM ANDROID_EABI FFMPEG_ROOT FREETYPE_ROOT OPENAL_ROOT LIBCONFIG_ROOT ANDROID_KEY_GUI ANDROID_KEYSTORE ANDROID_KEYSTORE_PASSWORD ANDROID_KEY ANDROID_KEY_PASSWORD
  for (aMakeEnvIter, aMakeEnvList) {
    !isEmpty($${aMakeEnvIter}) { ST_MAKE_ARGS += $${aMakeEnvIter}=$$val_escape($${aMakeEnvIter}) }
  }

  # Redirect build to main Makefile.
  QMAKE_POST_LINK   += $(MAKE) --directory $$sViewRoot $$ST_MAKE_ARGS;
  #realbuild.commands = $(MAKE) --directory $$sViewRoot $$ST_MAKE_ARGS
  #realbuild.target   = realbuild
  #QMAKE_EXTRA_TARGETS += realbuild
  #POST_TARGETDEPS     += realbuild
}

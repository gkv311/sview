# Makefile for sView project

WORKDIR = $(shell pwd)
SRCDIR = $(WORKDIR)
JAVA_HOME = /usr
APP_PREFIX = /usr

$(info SRCDIR=$(SRCDIR))

LBITS := $(shell getconf LONG_BIT)
HAVE_MONGOOSE := -DST_HAVE_MONGOOSE

# The following lines are required because standard make does not recognize the Objective-C++ .mm suffix
.SUFFIXES: .o .mm
.mm.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

# Compile java files.
# javac takes output folder, not file, and actually sometimes generates multiple files from single .java input.
# To fool make (avoid recompiling on each build) - copy result .class file to source folder;
# this, however, might lead to incomplete build on .java change without make clean.
.SUFFIXES: .class .java
.java.class:
	$(JAVA_HOME)/bin/javac -source 1.7 -target 1.7 -d $(BUILD_ROOT)/java/classes -classpath $(ANDROID_PLATFORM) -sourcepath $(SRCDIR)/sview/src:$(BUILD_ROOT)/java/gen $<
	cp -f $(BUILD_ROOT)/java/classes/com/sview/$(@F) $@

TARGET_OS = linux
TARGET_ARCH2 =

#ANDROID_NDK = $(HOME)/develop/android-ndk-r12b
ifeq ($(OS),Windows_NT)
TARGET_OS = wnt
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
TARGET_OS = linux
endif
ifeq ($(UNAME_S),Darwin)
TARGET_OS = osx
TARGET_OS_VERSION = 10.10
endif
endif

ifdef ANDROID_NDK
TARGET_OS = android
endif

BUILD_ROOT_BUNDLE = build
BUILD_ROOT = build
USR_LIB = lib
ifeq ($(TARGET_OS),osx)
BUILD_ROOT_BUNDLE = build/sView.app
BUILD_ROOT = $(BUILD_ROOT_BUNDLE)/Contents/MacOS
endif

ST_DEBUG = 0
FFMPEG_ROOT =
FREETYPE_ROOT =
OPENAL_ROOT =
LIBCONFIG_ROOT =
LIBSUBFOLDER = lib
LIBSUFFIX = so
ANDROID_BUILD_TOOLS = $(ANDROID_HOME)/build-tools/26.0.3
ANDROID_PLATFORM = $(ANDROID_HOME)/platforms/android-26/android.jar
ANDROID_KEYSTORE = $(BUILD_ROOT)/sview_debug.key
ANDROID_KEYSTORE_PASSWORD = sview_store_pswd
ANDROID_KEY = "sview android key"
ANDROID_KEY_PASSWORD = sview_pswd
#ST_REVISION = `git rev-list --count HEAD`
SVIEW_APK_CODE = 1
SVIEW_SDK_VER_STRING = 20.1

# function defining library install_name to @executable_path on OS X
libinstname =
ifeq ($(TARGET_OS),osx)
libinstname = -Wl,-install_name,@executable_path/$(1)
endif

LIB_PTHREAD =
LIB_GLX =
LIB_GTK =
LIB_OCCT  = -lTKMeshVS -lTKXDESTEP -lTKSTEP -lTKSTEPAttr -lTKSTEP209 -lTKSTEPBase -lTKXDEIGES -lTKIGES -lTKXSBase -lTKOpenGl -lTKXCAF -lTKVCAF -lTKCAF -lTKV3d -lTKHLR -lTKMesh -lTKService -lTKOffset -lTKFillet -lTKShHealing
LIB_OCCT += -lTKBool -lTKBO -lTKPrim -lTKTopAlgo -lTKGeomAlgo -lTKBRep -lTKGeomBase -lTKG3d -lTKG2d -lTKMath -lTKLCAF -lTKCDF -lTKernel
LIB_XLIB =
LIB_CONFIG =
LIB_ANDROID =
LIB_IOKIT =
LIB_COREVIDEO =
LIB_OPENAL = -lopenal
LIB_OUTPUTS = -lStOutAnaglyph -lStOutDual -lStOutInterlace -lStOutPageFlip -lStOutIZ3D -lStOutDistorted
TOOLCHAIN =

# Linux libraries
ifeq ($(TARGET_OS),linux)
LIB_PTHREAD = -lpthread
LIB_GLX = -lGL -lX11 -lXext
LIB_GTK = `pkg-config gtk+-2.0 --libs` -lgthread-2.0 -ldl
LIB_XLIB = -lXrandr -lXpm
LIB_CONFIG = -lconfig++
endif

# OS X libraries
ifeq ($(TARGET_OS),osx)
LIBSUFFIX = dylib
LIB_PTHREAD = -lobjc
LIB_GLX = -framework OpenGL -framework Appkit
LIB_IOKIT = -framework IOKit
LIB_COREVIDEO = -framework CoreVideo
ifeq ($(OPENAL_ROOT),)
  LIB_OPENAL = -framework OpenAL
endif
endif

# Android libraries
ifdef ANDROID_NDK
ANDROID_EABI = armeabi-v7a
ifeq ($(ANDROID_EABI),arm64-v8a)
  TOOLCHAIN = $(ANDROID_NDK)/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-
  ANDROID_SYSROOT = --sysroot=$(ANDROID_NDK)/platforms/android-21/arch-arm64
  ANDROID_MARCH   = -march=armv8-a
else ifeq ($(ANDROID_EABI),x86)
  TOOLCHAIN = $(ANDROID_NDK)/toolchains/i686-linux-android-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-
  ANDROID_SYSROOT = --sysroot=$(ANDROID_NDK)/platforms/android-15/arch-x86
  ANDROID_MARCH   = -march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32
else
  # armeabi-v7a
  TOOLCHAIN = $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-
  ANDROID_SYSROOT = --sysroot=$(ANDROID_NDK)/platforms/android-15/arch-arm
  ANDROID_MARCH   = -march=armv7-a -mfloat-abi=softfp
endif

HAVE_MONGOOSE =
LIB_PTHREAD = -lc
LIB_GLX = -lEGL -lGLESv2
LIB_GTK = -llog
LIB_XLIB =
LIB_CONFIG = -lconfig++
LIB_ANDROID = -landroid
LIB_OUTPUTS = -lStOutAnaglyph -lStOutInterlace -lStOutDistorted
endif

CC  = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AR  = $(TOOLCHAIN)ar
LD  = $(TOOLCHAIN)g++
STRIP = $(TOOLCHAIN)strip
STRIPFLAGS = --strip-unneeded

LDSTRIP = -s
LDZDEF = -z defs
EXTRA_CFLAGS   =
EXTRA_CXXFLAGS = -DAPP_PREFIX="\"$(APP_PREFIX)\""
EXTRA_LDFLAGS  =
ifeq ($(TARGET_OS),osx)
LDZDEF =
endif

CCVERSION := $(shell $(CC) -dumpversion)
CCMACHINE := $(shell $(CC) -dumpmachine)

ifneq (,$(findstring i586,$(CCMACHINE)))
TARGET_ARCH2 = x86
else ifneq (,$(findstring i686,$(CCMACHINE)))
TARGET_ARCH2 = x86
else ifneq (,$(findstring x86,$(CCMACHINE)))
TARGET_ARCH2 = x86
else ifneq (,$(findstring mingw,$(CCMACHINE)))
TARGET_ARCH2 = x86
endif

# to activate debug build
ifneq ($(ST_DEBUG),0)
EXTRA_CXXFLAGS = -DST_DEBUG
LDSTRIP =
STRIPFLAGS = --info
endif
#EXTRA_CXXFLAGS = -DST_DEBUG_LOG_TO_FILE=\"/storage/emulated/0/Android/data/com.sview/files/sview.log\" -DST_DEBUG
#EXTRA_CXXFLAGS += -DST_DEBUG_GL
#EXTRA_CXXFLAGS += -DST_DEBUG_FFMPEG_VERBOSE
#EXTRA_CXXFLAGS += -DST_DEBUG_SYSLOG
#EXTRA_CXXFLAGS += -DST_DEBUG_THREADID

ifdef ANDROID_NDK
LIBSUBFOLDER = libs/$(ANDROID_EABI)
EXTRA_CFLAGS   += $(ANDROID_SYSROOT) $(ANDROID_MARCH)
EXTRA_CXXFLAGS += $(ANDROID_SYSROOT) $(ANDROID_MARCH)
EXTRA_CXXFLAGS += -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/include -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(ANDROID_EABI)/include -DST_HAVE_EGL
EXTRA_LDFLAGS  += $(ANDROID_SYSROOT) -L$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(ANDROID_EABI) -lstdc++ -lgnustl_shared
else ifeq ($(TARGET_ARCH2),x86)
# necessary for 32-bit x86 builds, where MMX and SSE are not enabled by default
EXTRA_CFLAGS   += -mmmx -msse
EXTRA_CXXFLAGS += -mmmx -msse
endif

ifeq ($(TARGET_OS),osx)
# todo
LDSTRIP =

# minimal supported version of macOS:
# - CMake: CMAKE_OSX_DEPLOYMENT
# - qmake: QMAKE_MACOSX_DEPLOYMENT_TARGET = $(TARGET_OS_VERSION)
# - environment variable: export MACOSX_DEPLOYMENT_TARGET=$(TARGET_OS_VERSION)
EXTRA_CFLAGS   += -mmacosx-version-min=$(TARGET_OS_VERSION)
EXTRA_CXXFLAGS += -mmacosx-version-min=$(TARGET_OS_VERSION)
EXTRA_LDFLAGS  += -mmacosx-version-min=$(TARGET_OS_VERSION)

# workaround homebrew
HAS_PKGCONF := $(shell command -v pkg-config 2> /dev/null)
ifdef HAS_PKGCONF
EXTRA_CXXFLAGS += $(shell pkg-config --silence-errors freetype2 --cflags)
endif

endif

ifneq ($(FREETYPE_ROOT),)
EXTRA_CXXFLAGS += -I$(FREETYPE_ROOT)/include
EXTRA_LDFLAGS  += -L$(FREETYPE_ROOT)/$(LIBSUBFOLDER)
endif

ifneq ($(FFMPEG_ROOT),)
EXTRA_CXXFLAGS += -I$(FFMPEG_ROOT)/include
EXTRA_LDFLAGS  += -L$(FFMPEG_ROOT)/$(LIBSUBFOLDER)
endif

ifneq ($(OPENAL_ROOT),)
EXTRA_CXXFLAGS += -I$(OPENAL_ROOT)/include
EXTRA_LDFLAGS  += -L$(OPENAL_ROOT)/$(LIBSUBFOLDER)
endif

ifneq ($(LIBCONFIG_ROOT),)
EXTRA_CXXFLAGS += -I$(LIBCONFIG_ROOT)/include
EXTRA_LDFLAGS  += -L$(LIBCONFIG_ROOT)/$(LIBSUBFOLDER)
endif

ifeq ($(TARGET_OS),linux)
EXTRA_CXXFLAGS += `pkg-config gtk+-2.0 --cflags`
# notification about updates available on sview.ru
#EXTRA_CXXFLAGS += -DST_UPDATES_CHECK
endif

# optionally fail on any compiler warning except #warning and deprecations
WERROR_LEVEL = 0
ifeq ($(WERROR_LEVEL),1)
EXTRA_CXXFLAGS += -Werror -Wno-error=cpp -Wno-error=deprecated-declarations
endif

INC =  -I$(SRCDIR)/3rdparty/include -I$(SRCDIR)/include
CFLAGS   = -fPIC $(HAVE_MONGOOSE) $(INC) $(EXTRA_CFLAGS)
ifneq ($(ST_DEBUG),0)
CXXFLAGS = -g -std=c++0x -Wall -fPIC $(HAVE_MONGOOSE) $(INC) $(EXTRA_CXXFLAGS)
else
CXXFLAGS = -O3 -std=c++0x -Wall -fPIC $(HAVE_MONGOOSE) $(INC) $(EXTRA_CXXFLAGS)
endif
LIBDIR = -L$(BUILD_ROOT)
LIB =
LDFLAGS = $(LDSTRIP) $(LDZDEF) $(EXTRA_LDFLAGS)

aStShared       := libStShared.$(LIBSUFFIX)
aStGLWidgets    := libStGLWidgets.$(LIBSUFFIX)
aStCore         := libStCore.$(LIBSUFFIX)
aStOutAnaglyph  := libStOutAnaglyph.$(LIBSUFFIX)
aStOutDual      := libStOutDual.$(LIBSUFFIX)
aStOutInterlace := libStOutInterlace.$(LIBSUFFIX)
aStOutPageFlip  := libStOutPageFlip.$(LIBSUFFIX)
aStOutIZ3D      := libStOutIZ3D.$(LIBSUFFIX)
aStOutDistorted := libStOutDistorted.$(LIBSUFFIX)
aStImageViewer  := libStImageViewer.$(LIBSUFFIX)
aStMoviePlayer  := libStMoviePlayer.$(LIBSUFFIX)
aStDiagnostics  := libStDiagnostics.$(LIBSUFFIX)
aStCADViewer    := libStCADViewer.$(LIBSUFFIX)
sViewAndroidCad := libsviewcad.$(LIBSUFFIX)
sView           := sView
sViewAndroid    := libsview.$(LIBSUFFIX)
sViewApk        := $(SRCDIR)/build/sView.apk
sViewApkSigned  := $(SRCDIR)/build/sView.signed.apk.tmp
sViewApkUnsigned:= $(SRCDIR)/build/sView.unsigned.apk.tmp
sViewApkManifest:= $(SRCDIR)/build/AndroidManifest.xml
aDestAndroid    := build/apk-tmp
sViewDex        := $(aDestAndroid)/classes.dex

all:         shared $(aStDiagnostics) $(sView)
android_cad: pre_all_android shared $(sViewAndroidCad) $(sViewApk) install_android install_android_cad_libs
android:     pre_all_android shared $(sViewAndroid)    $(sViewApk) install_android install_android_libs
shared:      pre_all $(aStShared) $(aStGLWidgets) $(aStCore) outputs_all $(aStImageViewer) $(aStMoviePlayer)
clean:       clean_StShared clean_StGLWidgets clean_StCore clean_sView clean_StOutAnaglyph clean_StOutDual clean_StOutInterlace clean_StOutPageFlip clean_StOutIZ3D clean_StOutDistorted clean_StImageViewer clean_StMoviePlayer clean_StDiagnostics clean_StCADViewer clean_sViewAndroid
distclean:   clean

ifdef ANDROID_NDK
outputs_all: $(aStOutAnaglyph) $(aStOutInterlace) $(aStOutDistorted)
else
outputs_all: $(aStOutAnaglyph) $(aStOutDual) $(aStOutInterlace) $(aStOutPageFlip) $(aStOutIZ3D) $(aStOutDistorted)
endif

install:
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/bin
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/$(USR_LIB)/sView
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/$(USR_LIB)/firefox/plugins
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/$(USR_LIB)/mozilla/plugins
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/share
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/share/sView/info
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/share/sView/lang
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/share/sView/shaders
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/share/sView/textures
	mkdir -p $(DESTDIR)/$(APP_PREFIX)/share/sView/web
	cp -f -r share/*                     $(DESTDIR)/$(APP_PREFIX)/share/
	cp -f -r $(BUILD_ROOT)/lang/*        $(DESTDIR)/$(APP_PREFIX)/share/sView/lang/
	cp -f -r $(BUILD_ROOT)/shaders/*     $(DESTDIR)/$(APP_PREFIX)/share/sView/shaders/
	cp -f -r $(BUILD_ROOT)/textures/*    $(DESTDIR)/$(APP_PREFIX)/share/sView/textures/
	cp -f -r $(BUILD_ROOT)/web/*         $(DESTDIR)/$(APP_PREFIX)/share/sView/web/
	cp -f    license-gpl-3.0.txt         $(DESTDIR)/$(APP_PREFIX)/share/sView/info/license.txt
	cp -f -r $(BUILD_ROOT)/*.$(LIBSUFFIX) $(DESTDIR)/$(APP_PREFIX)/$(USR_LIB)/
	cp -f    $(BUILD_ROOT)/sView         $(DESTDIR)/$(APP_PREFIX)/$(USR_LIB)/sView/sView
	ln --force --symbolic ../$(USR_LIB)/sView/sView       $(DESTDIR)/$(APP_PREFIX)/bin/sView
	ln --force --symbolic ../../share/sView/demo/demo.jps $(DESTDIR)/$(APP_PREFIX)/$(USR_LIB)/sView/demo.jps
	rm -f    $(DESTDIR)/$(APP_PREFIX)/$(USR_LIB)/sView/*.a

install_android: $(sViewApkManifest)
	mkdir -p $(aDestAndroid)/assets/info
	mkdir -p $(aDestAndroid)/assets/lang/German
	mkdir -p $(aDestAndroid)/assets/lang/French
	mkdir -p $(aDestAndroid)/assets/lang/English
	mkdir -p $(aDestAndroid)/assets/lang/Spanish
	mkdir -p $(aDestAndroid)/assets/lang/Russian
	mkdir -p $(aDestAndroid)/assets/lang/Czech
	mkdir -p $(aDestAndroid)/assets/lang/ChineseS
	mkdir -p $(aDestAndroid)/assets/lang/ChineseT
	mkdir -p $(aDestAndroid)/assets/lang/Korean
	mkdir -p $(aDestAndroid)/assets/shaders
	mkdir -p $(aDestAndroid)/assets/textures
	cp -f -r $(BUILD_ROOT)/lang/Deutsch/*  $(aDestAndroid)/assets/lang/German/
	cp -f -r $(BUILD_ROOT)/lang/français/* $(aDestAndroid)/assets/lang/French/
	cp -f -r $(BUILD_ROOT)/lang/English/*  $(aDestAndroid)/assets/lang/English/
	cp -f -r $(BUILD_ROOT)/lang/Español/*  $(aDestAndroid)/assets/lang/Spanish/
	cp -f -r $(BUILD_ROOT)/lang/русский/*  $(aDestAndroid)/assets/lang/Russian/
	cp -f -r $(BUILD_ROOT)/lang/Czech/*    $(aDestAndroid)/assets/lang/Czech/
	cp -f -r $(BUILD_ROOT)/lang/ChineseS/* $(aDestAndroid)/assets/lang/ChineseS/
	cp -f -r $(BUILD_ROOT)/lang/ChineseT/* $(aDestAndroid)/assets/lang/ChineseT/
	cp -f -r $(BUILD_ROOT)/lang/Korean/*   $(aDestAndroid)/assets/lang/Korean/
	cp -f -r $(BUILD_ROOT)/shaders/*       $(aDestAndroid)/assets/shaders/
	cp -f -r $(BUILD_ROOT)/textures/*      $(aDestAndroid)/assets/textures/
	cp -f    license-gpl-3.0.txt           $(aDestAndroid)/assets/info/license.txt
	$(ANDROID_BUILD_TOOLS)/aapt package -v -f -m -S $(SRCDIR)/sview/res -J $(BUILD_ROOT)/java/gen -M $(sViewApkManifest) -I $(ANDROID_PLATFORM)

install_android_libs: $(aStShared) $(aStGLWidgets) $(aStCore) $(aStOutAnaglyph) $(aStOutInterlace) $(aStOutDistorted) $(aStImageViewer) $(aStMoviePlayer) $(sViewAndroid)
	cp -f $(BUILD_ROOT)/$(aStShared)       $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStGLWidgets)    $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStCore)         $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStOutAnaglyph)  $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStOutInterlace) $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStOutDistorted) $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStImageViewer)  $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStMoviePlayer)  $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(sViewAndroid)    $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(ANDROID_EABI)/libgnustl_shared.so $(aDestAndroid)/lib/$(ANDROID_EABI)/
	$(STRIP) $(STRIPFLAGS) $(aDestAndroid)/lib/$(ANDROID_EABI)/libgnustl_shared.so
	cp -f $(FREETYPE_ROOT)/libs/$(ANDROID_EABI)/libfreetype.so    $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(OPENAL_ROOT)/libs/$(ANDROID_EABI)/libopenal.so        $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libavcodec.so       $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libavdevice.so      $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libavformat.so      $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libavutil.so        $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libswresample.so    $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libswscale.so       $(aDestAndroid)/lib/$(ANDROID_EABI)/

install_android_cad_libs: $(aStShared) $(aStGLWidgets) $(aStCore) $(aStOutAnaglyph) $(aStOutInterlace) $(aStOutDistorted) $(aStImageViewer) $(aStMoviePlayer) $(sViewAndroidCad)
	cp -f $(BUILD_ROOT)/$(aStShared)       $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStGLWidgets)    $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStCore)         $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStOutAnaglyph)  $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStOutInterlace) $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStOutDistorted) $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStImageViewer)  $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(aStMoviePlayer)  $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(BUILD_ROOT)/$(sViewAndroidCad) $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FREETYPE_ROOT)/libs/$(ANDROID_EABI)/libfreetype.so    $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(OPENAL_ROOT)/libs/$(ANDROID_EABI)/libopenal.so        $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libavcodec.so       $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libavdevice.so      $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libavformat.so      $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libavutil.so        $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libswresample.so    $(aDestAndroid)/lib/$(ANDROID_EABI)/
	cp -f $(FFMPEG_ROOT)/libs/$(ANDROID_EABI)/libswscale.so       $(aDestAndroid)/lib/$(ANDROID_EABI)/
	mkdir -p $(aDestAndroid)/assets/shaders
	mkdir -p $(aDestAndroid)/assets/lang

pre_all:
	mkdir -p $(BUILD_ROOT)/lang/English
	mkdir -p $(BUILD_ROOT)/lang/Español
	mkdir -p $(BUILD_ROOT)/lang/русский
	mkdir -p $(BUILD_ROOT)/lang/français
	mkdir -p $(BUILD_ROOT)/lang/Deutsch
	mkdir -p $(BUILD_ROOT)/lang/Czech
	mkdir -p $(BUILD_ROOT)/lang/ChineseS
	mkdir -p $(BUILD_ROOT)/lang/ChineseT
	mkdir -p $(BUILD_ROOT)/lang/Korean
	mkdir -p $(BUILD_ROOT)/textures
	mkdir -p $(BUILD_ROOT)/web
	cp -f -r textures/* $(BUILD_ROOT)/textures/

pre_all_android:
	mkdir -p $(aDestAndroid)/lib/$(ANDROID_EABI)
	mkdir -p $(BUILD_ROOT)/java/gen
	mkdir -p $(BUILD_ROOT)/java/classes

# StShared shared library
aStShared_SRCS1 := $(sort $(wildcard $(SRCDIR)/StShared/*.cpp))
aStShared_OBJS1 := ${aStShared_SRCS1:.cpp=.o}
aStShared_SRCS2 :=
aStShared_OBJS2 :=
ifeq ($(TARGET_OS),osx)
aStShared_SRCS2 := $(sort $(wildcard $(SRCDIR)/StShared/*.mm))
aStShared_OBJS2 := ${aStShared_SRCS2:.mm=.o}
endif
aStShared_LIB  := $(LIB) $(LIB_GLX) $(LIB_GTK) $(LIB_ANDROID) -lavutil -lavformat -lavcodec -lswscale -lfreetype $(LIB_CONFIG) $(LIB_PTHREAD)
$(aStShared) : $(aStShared_OBJS1) $(aStShared_OBJS2)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStShared_OBJS1) $(aStShared_OBJS2) $(aStShared_LIB) -o $(BUILD_ROOT)/$@
clean_StShared:
	rm -f $(BUILD_ROOT)/$(aStShared)
	rm -rf StShared/*.o

# StGLWidgets shared library
aStGLWidgets_SRCS := $(sort $(wildcard $(SRCDIR)/StGLWidgets/*.cpp))
aStGLWidgets_OBJS := ${aStGLWidgets_SRCS:.cpp=.o}
aStGLWidgets_LIB  := $(LIB) -lStShared $(LIB_GLX)
$(aStGLWidgets) : pre_StGLWidgets $(aStShared) $(aStGLWidgets_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStGLWidgets_OBJS) $(aStGLWidgets_LIB) -o $(BUILD_ROOT)/$@
pre_StGLWidgets:

clean_StGLWidgets:
	rm -f $(BUILD_ROOT)/$(aStGLWidgets)
	rm -rf StGLWidgets/*.o

# StCore library
aStCore_SRCS1 := $(sort $(wildcard $(SRCDIR)/StCore/*.cpp))
aStCore_OBJS1 := ${aStCore_SRCS1:.cpp=.o}
aStCore_SRCS2 :=
aStCore_OBJS2 :=
ifeq ($(TARGET_OS),osx)
aStCore_SRCS2 := $(sort $(wildcard $(SRCDIR)/StCore/*.mm))
aStCore_OBJS2 := ${aStCore_SRCS2:.mm=.o}
endif
aStCore_LIB  := $(LIB) -lStShared $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD) $(LIB_XLIB) $(LIB_ANDROID) $(LIB_IOKIT)
$(aStCore) : $(aStShared) $(aStCore_OBJS1) $(aStCore_OBJS2)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStCore_OBJS1) $(aStCore_OBJS2) $(aStCore_LIB) -o $(BUILD_ROOT)/$@
clean_StCore:
	rm -f $(BUILD_ROOT)/$(aStCore)
	rm -rf StCore/*.o

# StOutAnaglyph library (Anaglyph output)
aStOutAnaglyph_SRCS := $(sort $(wildcard $(SRCDIR)/StOutAnaglyph/*.cpp))
aStOutAnaglyph_OBJS := ${aStOutAnaglyph_SRCS:.cpp=.o}
aStOutAnaglyph_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutAnaglyph) : pre_StOutAnaglyph $(aStCore) $(aStOutAnaglyph_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStOutAnaglyph_OBJS) $(aStOutAnaglyph_LIB) -o $(BUILD_ROOT)/$@
pre_StOutAnaglyph:
	mkdir -p $(BUILD_ROOT)/shaders/StOutAnaglyph/
	cp -f -r StOutAnaglyph/shaders/*      $(BUILD_ROOT)/shaders/StOutAnaglyph/
	cp -f -r StOutAnaglyph/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutAnaglyph/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StOutAnaglyph/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutAnaglyph/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutAnaglyph/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutAnaglyph/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutAnaglyph/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutAnaglyph/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StOutAnaglyph/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutAnaglyph:
	rm -f $(BUILD_ROOT)/$(aStOutAnaglyph)
	rm -rf StOutAnaglyph/*.o
	rm -rf $(BUILD_ROOT)/shaders/*
	rm -rf $(BUILD_ROOT)/lang/*/StOutAnaglyph.lng

# StOutDual library (Dual output)
aStOutDual_SRCS := $(sort $(wildcard $(SRCDIR)/StOutDual/*.cpp))
aStOutDual_OBJS := ${aStOutDual_SRCS:.cpp=.o}
aStOutDual_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutDual) : pre_StOutDual $(aStCore) $(aStOutDual_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStOutDual_OBJS) $(aStOutDual_LIB) -o $(BUILD_ROOT)/$@
pre_StOutDual:
	cp -f -r StOutDual/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutDual/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StOutDual/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutDual/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutDual/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutDual/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutDual/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutDual/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StOutDual/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutDual:
	rm -f $(BUILD_ROOT)/$(aStOutDual)
	rm -rf StOutDual/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StOutDual.lng

# StOutIZ3D library (iZ3D monitor)
aStOutIZ3D_SRCS := $(sort $(wildcard $(SRCDIR)/StOutIZ3D/*.cpp))
aStOutIZ3D_OBJS := ${aStOutIZ3D_SRCS:.cpp=.o}
aStOutIZ3D_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutIZ3D) : pre_StOutIZ3D $(aStCore) $(aStOutIZ3D_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStOutIZ3D_OBJS) $(aStOutIZ3D_LIB) -o $(BUILD_ROOT)/$@
pre_StOutIZ3D:
	mkdir -p $(BUILD_ROOT)/shaders/StOutIZ3D/
	cp -f -r StOutIZ3D/shaders/*      $(BUILD_ROOT)/shaders/StOutIZ3D/
	cp -f -r StOutIZ3D/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutIZ3D/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StOutIZ3D/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutIZ3D/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutIZ3D/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutIZ3D/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutIZ3D/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutIZ3D/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StOutIZ3D/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutIZ3D:
	rm -f $(BUILD_ROOT)/$(aStOutIZ3D)
	rm -rf StOutIZ3D/*.o
	rm -rf $(BUILD_ROOT)/shaders/*
	rm -rf $(BUILD_ROOT)/lang/*/StOutIZ3D.lng

# StOutInterlace library (Interlaced output)
aStOutInterlace_SRCS := $(sort $(wildcard $(SRCDIR)/StOutInterlace/*.cpp))
aStOutInterlace_OBJS := ${aStOutInterlace_SRCS:.cpp=.o}
aStOutInterlace_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutInterlace) : pre_StOutInterlace $(aStCore) $(aStOutInterlace_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStOutInterlace_OBJS) $(aStOutInterlace_LIB) -o $(BUILD_ROOT)/$@
pre_StOutInterlace:
	mkdir -p $(BUILD_ROOT)/shaders/StOutInterlace/
	cp -f -r StOutInterlace/shaders/*      $(BUILD_ROOT)/shaders/StOutInterlace/
	cp -f -r StOutInterlace/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutInterlace/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StOutInterlace/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutInterlace/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutInterlace/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutInterlace/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutInterlace/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutInterlace/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StOutInterlace/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutInterlace:
	rm -f $(BUILD_ROOT)/$(aStOutInterlace)
	rm -rf StOutInterlace/*.o
	rm -rf $(BUILD_ROOT)/shaders/*
	rm -rf $(BUILD_ROOT)/lang/*/StOutInterlace.lng

# StOutPageFlip library (Shutter glasses output)
aStOutPageFlip_SRCS1 := $(sort $(wildcard $(SRCDIR)/StOutPageFlip/*.cpp))
aStOutPageFlip_OBJS1 := ${aStOutPageFlip_SRCS1:.cpp=.o}
aStOutPageFlip_SRCS2 :=
aStOutPageFlip_OBJS2 :=
ifeq ($(TARGET_OS),osx)
aStOutPageFlip_SRCS2 := $(sort $(wildcard $(SRCDIR)/StOutPageFlip/*.mm))
aStOutPageFlip_OBJS2 := ${aStOutPageFlip_SRCS2:.mm=.o}
endif
aStOutPageFlip_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutPageFlip) : pre_StOutPageFlip $(aStCore) $(aStOutPageFlip_OBJS1) $(aStOutPageFlip_OBJS2)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStOutPageFlip_OBJS1) $(aStOutPageFlip_OBJS2) $(aStOutPageFlip_LIB) -o $(BUILD_ROOT)/$@
pre_StOutPageFlip:
	cp -f -r StOutPageFlip/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutPageFlip/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StOutPageFlip/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutPageFlip/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutPageFlip/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutPageFlip/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutPageFlip/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutPageFlip/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StOutPageFlip/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutPageFlip:
	rm -f $(BUILD_ROOT)/$(aStOutPageFlip)
	rm -rf StOutPageFlip/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StOutPageFlip.lng

# StOutDistorted library
aStOutDistorted_SRCS := $(sort $(wildcard $(SRCDIR)/StOutDistorted/*.cpp))
aStOutDistorted_OBJS := ${aStOutDistorted_SRCS:.cpp=.o}
aStOutDistorted_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutDistorted) : pre_StOutDistorted $(aStCore) $(aStOutDistorted_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStOutDistorted_OBJS) $(aStOutDistorted_LIB) -o $(BUILD_ROOT)/$@
pre_StOutDistorted:
	cp -f -r StOutDistorted/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutDistorted/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StOutDistorted/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutDistorted/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutDistorted/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutDistorted/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutDistorted/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutDistorted/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StOutDistorted/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutDistorted:
	rm -f $(BUILD_ROOT)/$(aStOutDistorted)
	rm -rf StOutDistorted/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StOutDistorted.lng

# StImageViewer library (Image Viewer)
aStImageViewer_SRCS := $(sort $(wildcard $(SRCDIR)/StImageViewer/*.cpp))
aStImageViewer_OBJS := ${aStImageViewer_SRCS:.cpp=.o}
aStImageViewer_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStImageViewer) : pre_StImageViewer $(aStGLWidgets) outputs_all $(aStImageViewer_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStImageViewer_OBJS) $(aStImageViewer_LIB) -o $(BUILD_ROOT)/$@
pre_StImageViewer:
	cp -f -r StImageViewer/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StImageViewer/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StImageViewer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StImageViewer/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StImageViewer/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StImageViewer/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StImageViewer/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StImageViewer/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StImageViewer/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StImageViewer:
	rm -f $(BUILD_ROOT)/$(aStImageViewer)
	rm -rf StImageViewer/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StImageViewer.lng

# StMoviePlayer library (Image Viewer)
aStMoviePlayer_SRCS1 := $(sort $(wildcard $(SRCDIR)/StMoviePlayer/*.cpp))
aStMoviePlayer_OBJS1 := ${aStMoviePlayer_SRCS1:.cpp=.o}
aStMoviePlayer_SRCS2 := $(sort $(wildcard $(SRCDIR)/StMoviePlayer/StVideo/*.cpp))
aStMoviePlayer_OBJS2 := ${aStMoviePlayer_SRCS2:.cpp=.o}
aStMoviePlayer_SRCS3 := $(sort $(wildcard $(SRCDIR)/StMoviePlayer/*.c))
aStMoviePlayer_OBJS3 := ${aStMoviePlayer_SRCS3:.c=.o}
aStMoviePlayer_LIB   := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_GLX) $(LIB_GTK) -lavutil -lavformat -lavcodec -lswscale $(LIB_OPENAL) $(LIB_COREVIDEO) $(LIB_PTHREAD)
$(aStMoviePlayer) : pre_StMoviePlayer $(aStGLWidgets) outputs_all $(aStMoviePlayer_OBJS1) $(aStMoviePlayer_OBJS2) $(aStMoviePlayer_OBJS3)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStMoviePlayer_OBJS1) $(aStMoviePlayer_OBJS2) $(aStMoviePlayer_OBJS3) $(aStMoviePlayer_LIB) -o $(BUILD_ROOT)/$@
pre_StMoviePlayer:
	cp -f -r StMoviePlayer/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StMoviePlayer/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StMoviePlayer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StMoviePlayer/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StMoviePlayer/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StMoviePlayer/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StMoviePlayer/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StMoviePlayer/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StMoviePlayer/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
	cp -f -r StMoviePlayer/web/*          $(BUILD_ROOT)/web/
clean_StMoviePlayer:
	rm -f $(BUILD_ROOT)/$(aStMoviePlayer)
	rm -rf StMoviePlayer/*.o
	rm -rf StMoviePlayer/StVideo/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StMoviePlayer.lng
	rm -rf $(BUILD_ROOT)/web/*

# StDiagnostics library
aStDiagnostics_SRCS := $(sort $(wildcard $(SRCDIR)/StDiagnostics/*.cpp))
aStDiagnostics_OBJS := ${aStDiagnostics_SRCS:.cpp=.o}
aStDiagnostics_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStDiagnostics) : pre_StDiagnostics $(aStGLWidgets) outputs_all $(aStDiagnostics_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStDiagnostics_OBJS) $(aStDiagnostics_LIB) -o $(BUILD_ROOT)/$@
pre_StDiagnostics:
	cp -f -r StDiagnostics/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StDiagnostics/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StDiagnostics/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StDiagnostics/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StDiagnostics/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StDiagnostics/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StDiagnostics/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StDiagnostics/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StDiagnostics/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StDiagnostics:
	rm -f $(BUILD_ROOT)/$(aStDiagnostics)
	rm -rf StDiagnostics/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StDiagnostics.lng

# sView CAD Android JNI executable
sViewAndroidCad_SRCS := $(sort $(wildcard $(SRCDIR)/StCADViewer/*.cpp))
sViewAndroidCad_OBJS := ${sViewAndroidCad_SRCS:.cpp=.o}
sViewAndroidCad_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_OCCT) -lfreetype -llog -landroid -lEGL -lGLESv2 -lc
$(sViewAndroidCad) : pre_StCADViewer $(aStGLWidgets) outputs_all $(sViewAndroidCad_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(sViewAndroidCad_OBJS) $(sViewAndroidCad_LIB) -o $(BUILD_ROOT)/$(sViewAndroidCad)
clean_sViewAndroidCad:
	rm -f $(BUILD_ROOT)/$(sViewAndroidCad)
	rm -rf StCADViewer/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StCADViewer.lng

# StCADViewer library
aStCADViewer_SRCS := $(sort $(wildcard $(SRCDIR)/StCADViewer/*.cpp))
aStCADViewer_OBJS := ${aStCADViewer_SRCS:.cpp=.o}
aStCADViewer_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
aStCADViewer_LIB  += $(LIB_OCCT)
$(aStCADViewer) : pre_StCADViewer $(aStGLWidgets) outputs_all $(aStCADViewer_OBJS)
	$(LD) -shared $(call libinstname,$@) $(LDFLAGS) $(LIBDIR) $(aStCADViewer_OBJS) $(aStCADViewer_LIB) -o $(BUILD_ROOT)/$@
pre_StCADViewer:
	cp -f -r StCADViewer/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StCADViewer/lang/spanish/* $(BUILD_ROOT)/lang/Español/
	cp -f -r StCADViewer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StCADViewer/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StCADViewer/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StCADViewer/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StCADViewer/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StCADViewer/lang/chineset/* $(BUILD_ROOT)/lang/ChineseT/
	cp -f -r StCADViewer/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StCADViewer:
	rm -f $(BUILD_ROOT)/$(aStCADViewer)
	rm -rf StCADViewer/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StCADViewer.lng

# sView Android JNI executable
sViewAndroid_SRCS := $(sort $(wildcard $(SRCDIR)/sview/jni/*.cpp))
sViewAndroid_OBJS := ${sViewAndroid_SRCS:.cpp=.o}
sViewAndroid_LIB  := $(LIB) -lStShared -lStCore -lStImageViewer -lStMoviePlayer -llog -landroid -lEGL -lGLESv2 -lc
$(sViewAndroid) : $(aStImageViewer) $(aStMoviePlayer) $(sViewAndroid_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(sViewAndroid_OBJS) $(sViewAndroid_LIB) -o $(BUILD_ROOT)/$(sViewAndroid)
clean_sViewAndroid:
	rm -f $(BUILD_ROOT)/$(sViewAndroid)
	rm -rf sview/jni/*.o
	rm -rf $(BUILD_ROOT)/java/classes/com/sview/*.class
	rm -rf $(SRCDIR)/sview/src/com/sview/*class
	rm -rf $(aDestAndroid)
	rm -f $(sViewApkManifest)
	rm -f $(sViewDex)
	rm -f $(sViewApkUnsigned)
	rm -f $(sViewApkSigned)
	rm -f $(sViewApk)
	rm -rf $(BUILD_ROOT)/java/gen/com/sview/*.java

# sView executable
sView_SRCS1 := $(sort $(wildcard $(SRCDIR)/sview/*.cpp))
sView_OBJS1 := ${sView_SRCS1:.cpp=.o}
sView_SRCS2 :=
sView_OBJS2 :=
sView_LIB_DEPS = -lX11 -ldl -lgthread-2.0
ifeq ($(TARGET_OS),osx)
sView_SRCS2 := $(sort $(wildcard $(SRCDIR)/sview/*.mm))
sView_OBJS2 := ${sView_SRCS2:.mm=.o}
sView_LIB_DEPS = -framework Appkit
endif
sView_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore -lStImageViewer -lStMoviePlayer -lStDiagnostics $(LIB_OUTPUTS) $(LIB_GTK) $(sView_LIB_DEPS) $(LIB_PTHREAD)
$(sView) : $(aStImageViewer) $(aStMoviePlayer) $(aStDiagnostics) $(sView_OBJS1) $(sView_OBJS2)
	$(LD) $(LDFLAGS) $(LIBDIR) $(sView_OBJS1) $(sView_OBJS2) $(sView_LIB) -o $(BUILD_ROOT)/$(sView)
ifeq ($(TARGET_OS),osx)
	mkdir -p $(BUILD_ROOT_BUNDLE)/Contents/Frameworks/
	mkdir -p $(BUILD_ROOT_BUNDLE)/Contents/Resources/English.lproj/
	cp -f -r sview/Contents/*                 $(BUILD_ROOT_BUNDLE)/Contents/
	cp -f    sview/Resources/sView.icns       $(BUILD_ROOT_BUNDLE)/Contents/Resources/
	cp -f    sview/Resources/sView_Media.icns $(BUILD_ROOT_BUNDLE)/Contents/Resources/
	ibtool --compile $(BUILD_ROOT_BUNDLE)/Contents/Resources/English.lproj/MainMenu.nib sview/Resources/English.lproj/MainMenu.xib
ifneq ($(FREETYPE_ROOT),)
	cp -R -f $(FREETYPE_ROOT)/$(LIBSUBFOLDER)/libfreetype*.dylib $(BUILD_ROOT_BUNDLE)/Contents/Frameworks
endif
ifneq ($(FFMPEG_ROOT),)
	cp -R -f $(FFMPEG_ROOT)/$(LIBSUBFOLDER)/libavcodec*.dylib    $(BUILD_ROOT_BUNDLE)/Contents/Frameworks
	cp -R -f $(FFMPEG_ROOT)/$(LIBSUBFOLDER)/libavdevice*.dylib   $(BUILD_ROOT_BUNDLE)/Contents/Frameworks
	cp -R -f $(FFMPEG_ROOT)/$(LIBSUBFOLDER)/libavformat*.dylib   $(BUILD_ROOT_BUNDLE)/Contents/Frameworks
	cp -R -f $(FFMPEG_ROOT)/$(LIBSUBFOLDER)/libavutil*.dylib     $(BUILD_ROOT_BUNDLE)/Contents/Frameworks
	cp -R -f $(FFMPEG_ROOT)/$(LIBSUBFOLDER)/libswresample*.dylib $(BUILD_ROOT_BUNDLE)/Contents/Frameworks
	cp -R -f $(FFMPEG_ROOT)/$(LIBSUBFOLDER)/libswscale*.dylib    $(BUILD_ROOT_BUNDLE)/Contents/Frameworks
endif
ifneq ($(OPENAL_ROOT),)
	mkdir -p $(BUILD_ROOT_BUNDLE)/Contents/MacOS/openal/hrtf/
	cp -R -f $(OPENAL_ROOT)/$(LIBSUBFOLDER)/libopenal*.dylib     $(BUILD_ROOT_BUNDLE)/Contents/Frameworks
	cp -f -r $(OPENAL_ROOT)/hrtf/*.mhr                           $(BUILD_ROOT_BUNDLE)/Contents/MacOS/openal/hrtf/
endif
endif
	@echo sView building is DONE


$(sViewApkManifest):
	cp -f -r $(SRCDIR)/sview/AndroidManifest.xml.in $(SRCDIR)/build/AndroidManifest.xml
	sed -i "s/__SVIEW_APK_VER_CODE__/$(SVIEW_APK_CODE)/gi"          $(SRCDIR)/build/AndroidManifest.xml
	sed -i "s/__SVIEW_SDK_VER_STRING__/$(SVIEW_SDK_VER_STRING)/gi"  $(SRCDIR)/build/AndroidManifest.xml

$(sViewApk): $(sViewApkSigned)
	$(ANDROID_BUILD_TOOLS)/zipalign -v -f 4 $< $(sViewApk)

# There are three options:
# 1) Passwords are specified within Makefile / passed as make arguments
# 2) Passwords are asked by jarsigner itself (requires console input)
# 3) Passwords are asked using zenity message window (requires GUI input)
ifeq ($(MAKECMDGOALS), android)

ifeq ($(ANDROID_KEY_GUI), 1)
$(sViewApkSigned): $(sViewApkUnsigned) sView_keystore_debug
	$(eval ANDROID_KEYSTORE_PASSWORD := $(shell zenity --password --title="Android keystore"))
	$(eval ANDROID_KEY_PASSWORD      := $(shell zenity --password --title="Android key"))
	$(JAVA_HOME)/bin/jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore $(ANDROID_KEYSTORE) -storepass $(ANDROID_KEYSTORE_PASSWORD) -keypass $(ANDROID_KEY_PASSWORD) -signedjar $(sViewApkSigned) $< $(ANDROID_KEY)
else ifeq ($(ANDROID_KEY_PASSWORD),)
$(sViewApkSigned): $(sViewApkUnsigned) sView_keystore_debug
	$(JAVA_HOME)/bin/jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore $(ANDROID_KEYSTORE) -signedjar $(sViewApkSigned) $< $(ANDROID_KEY)
else
$(sViewApkSigned): $(sViewApkUnsigned) sView_keystore_debug
	$(JAVA_HOME)/bin/jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore $(ANDROID_KEYSTORE) -storepass $(ANDROID_KEYSTORE_PASSWORD) -keypass $(ANDROID_KEY_PASSWORD) -signedjar $(sViewApkSigned) $< $(ANDROID_KEY)
endif
endif

$(sViewApkUnsigned): $(sViewDex) $(sViewApkManifest) install_android_libs
	rm -f $(sViewApkUnsigned)
	$(ANDROID_BUILD_TOOLS)/aapt package -v -f -M $(sViewApkManifest) -S $(SRCDIR)/sview/res -I $(ANDROID_PLATFORM) -F $(sViewApkUnsigned) $(aDestAndroid)

sView_SRCS_JAVA1 := $(sort $(wildcard $(SRCDIR)/sview/src/com/sview/*.java))
sView_OBJS_JAVA1 := ${sView_SRCS_JAVA1:.java=.class}
$(sViewDex): $(BUILD_ROOT)/java/gen/com/sview/R.class $(sView_OBJS_JAVA1)
	$(ANDROID_BUILD_TOOLS)/dx --dex --verbose --output=$(sViewDex) $(BUILD_ROOT)/java/classes

$(BUILD_ROOT)/java/gen/com/sview/R.java: install_android $(sViewApkManifest) $(shell find $(SRCDIR)/sview/res -type f)
	$(ANDROID_BUILD_TOOLS)/aapt package -v -f -m -S $(SRCDIR)/sview/res -J $(BUILD_ROOT)/java/gen -M $(sViewApkManifest) -I $(ANDROID_PLATFORM)

# This target generates a dummy signing key for debugging purposes.
# Executed only when ANDROID_KEYSTORE points to non-existing file.
ifeq (,$(wildcard $(ANDROID_KEYSTORE)))
sView_keystore_debug:
	$(JAVA_HOME)/bin/keytool -genkeypair -validity 1000 -dname "CN=sview_dummy,O=Android,C=JPN" -keystore $(ANDROID_KEYSTORE) \
	-storepass $(ANDROID_KEYSTORE_PASSWORD) -keypass $(ANDROID_KEY_PASSWORD) -alias $(ANDROID_KEY) -keyalg RSA -v
else
sView_keystore_debug:
endif

clean_sView:
	rm -f $(BUILD_ROOT)/$(sView)
	rm -rf sview/*.o

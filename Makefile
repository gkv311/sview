# Makefile for sView project

WORKDIR = `pwd`

LBITS := $(shell getconf LONG_BIT)
HAVE_MONGOOSE := -DST_HAVE_MONGOOSE

#ANDROID_NDK = /home/kirill/develop/android-ndk-r10
BUILD_ROOT = build

LIB_PTHREAD = -lpthread
LIB_GLX = -lGL -lX11 -lXext
LIB_GTK = `pkg-config gtk+-2.0 --libs` -lgthread-2.0 -ldl
LIB_OCCT  = -lTKMeshVS -lTKXDESTEP -lTKSTEP -lTKSTEPAttr -lTKSTEP209 -lTKSTEPBase -lTKXDEIGES -lTKIGES -lTKXSBase -lTKOpenGl -lTKXCAF -lTKCAF -lTKV3d -lTKHLR -lTKMesh -lTKService -lTKOffset -lTKFillet -lTKShHealing
LIB_OCCT += -lTKBool -lTKBO -lTKPrim -lTKTopAlgo -lTKGeomAlgo -lTKBRep -lTKGeomBase -lTKG3d -lTKG2d -lTKMath -lTKLCAF -lTKCDF -lTKernel
LIB_XLIB = -lXrandr -lXpm
LIB_CONFIG = -lconfig++
LIB_ANDROID =
LIB_OUTPUTS = -lStOutAnaglyph -lStOutDual -lStOutInterlace -lStOutPageFlip -lStOutIZ3D -lStOutDistorted

TOOLCHAIN =
ifdef ANDROID_NDK
TOOLCHAIN = $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/arm-linux-androideabi/bin/
HAVE_MONGOOSE =
LIB_PTHREAD = -lc
LIB_GLX = -lEGL -lGLESv2
LIB_GTK = -llog
LIB_XLIB =
LIB_ANDROID = -landroid
LIB_OUTPUTS = -lStOutAnaglyph -lStOutInterlace -lStOutDistorted
endif

CC  = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AR  = $(TOOLCHAIN)ar
LD  = $(TOOLCHAIN)g++

LDSTRIP = -s -z defs
EXTRA_CFLAGS   =
EXTRA_CXXFLAGS =
EXTRA_LDFLAGS  =

# to activate debug build
#EXTRA_CXXFLAGS = -DST_DEBUG_LOG_TO_FILE=\"/sdcard/Android/data/com.sview/files/sview.log\" -DST_DEBUG
#LDSTRIP =

ifdef ANDROID_NDK
EXTRA_CFLAGS   += --sysroot=$(ANDROID_NDK)/platforms/android-15/arch-arm -march=armv7-a -mfloat-abi=softfp
EXTRA_CXXFLAGS += --sysroot=$(ANDROID_NDK)/platforms/android-15/arch-arm -march=armv7-a -mfloat-abi=softfp -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/include -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include -DST_HAVE_EGL -DST_NO_UPDATES_CHECK
EXTRA_LDFLAGS  += --sysroot=$(ANDROID_NDK)/platforms/android-15/arch-arm -L$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a -lstdc++ -lgnustl_shared
else
EXTRA_CFLAGS   += -mmmx -msse
EXTRA_CXXFLAGS += -mmmx -msse `pkg-config gtk+-2.0 --cflags`
endif

INC =  -I3rdparty/include -Iinclude
CFLAGS   = -fPIC $(HAVE_MONGOOSE) $(INC) $(EXTRA_CFLAGS)
CXXFLAGS = -O3 -std=c++0x -Wall -fPIC $(HAVE_MONGOOSE) $(INC) $(EXTRA_CXXFLAGS)
LIBDIR = -L$(BUILD_ROOT)
LIB =
LDFLAGS = $(LDSTRIP) -z defs $(EXTRA_LDFLAGS)
BUILD_ROOT = build
USR_LIB = lib

aStShared       := libStShared.so
aStGLWidgets    := libStGLWidgets.so
aStCore         := libStCore.so
aStOutAnaglyph  := libStOutAnaglyph.so
aStOutDual      := libStOutDual.so
aStOutInterlace := libStOutInterlace.so
aStOutPageFlip  := libStOutPageFlip.so
aStOutIZ3D      := libStOutIZ3D.so
aStOutDistorted := libStOutDistorted.so
aStImageViewer  := libStImageViewer.so
aStMoviePlayer  := libStMoviePlayer.so
aStDiagnostics  := libStDiagnostics.so
aStCADViewer    := libStCADViewer.so
sViewAndroidCad := libsviewcad.so
sView           := sView
sViewAndroid    := libsview.so

aDestAndroid    := sview

all:         pre_all $(aStShared) $(aStGLWidgets) $(aStCore) $(aStOutAnaglyph) $(aStOutDual) $(aStOutInterlace) $(aStOutPageFlip) $(aStOutIZ3D) $(aStOutDistorted) $(aStImageViewer) $(aStMoviePlayer) $(aStDiagnostics) $(sView)
android_cad: aDestAndroid = StCADViewer
android_cad: pre_all $(aStShared) $(aStGLWidgets) $(aStCore) $(aStOutAnaglyph) $(aStOutInterlace) $(aStOutDistorted) $(aStImageViewer) $(aStMoviePlayer) $(sViewAndroidCad) install_android install_android_cad_libs
android:     pre_all $(aStShared) $(aStGLWidgets) $(aStCore) $(aStOutAnaglyph) $(aStOutInterlace) $(aStOutDistorted) $(aStImageViewer) $(aStMoviePlayer) $(sViewAndroid)    install_android install_android_libs
clean:       clean_StShared clean_StGLWidgets clean_StCore clean_sView clean_StOutAnaglyph clean_StOutDual clean_StOutInterlace clean_StOutPageFlip clean_StOutIZ3D clean_StOutDistorted clean_StImageViewer clean_StMoviePlayer clean_StDiagnostics clean_StCADViewer clean_sViewAndroid
distclean:   clean

ifdef ANDROID_NDK
outputs_all: $(aStOutAnaglyph) $(aStOutInterlace) $(aStOutDistorted)
else
outputs_all: $(aStOutAnaglyph) $(aStOutDual) $(aStOutInterlace) $(aStOutPageFlip) $(aStOutIZ3D) $(aStOutDistorted)
endif

install:
	mkdir -p $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/$(USR_LIB)/sView
	mkdir -p $(DESTDIR)/usr/$(USR_LIB)/firefox/plugins
	mkdir -p $(DESTDIR)/usr/$(USR_LIB)/mozilla/plugins
	mkdir -p $(DESTDIR)/usr/share
	mkdir -p $(DESTDIR)/usr/share/sView/info
	mkdir -p $(DESTDIR)/usr/share/sView/lang
	mkdir -p $(DESTDIR)/usr/share/sView/shaders
	mkdir -p $(DESTDIR)/usr/share/sView/textures
	mkdir -p $(DESTDIR)/usr/share/sView/web
	cp -f -r share/*                     $(DESTDIR)/usr/share/
	cp -f -r $(BUILD_ROOT)/lang/*        $(DESTDIR)/usr/share/sView/lang/
	cp -f -r $(BUILD_ROOT)/shaders/*     $(DESTDIR)/usr/share/sView/shaders/
	cp -f -r $(BUILD_ROOT)/textures/*    $(DESTDIR)/usr/share/sView/textures/
	cp -f -r $(BUILD_ROOT)/web/*         $(DESTDIR)/usr/share/sView/web/
	cp -f    license-gpl-3.0.txt         $(DESTDIR)/usr/share/sView/info/license.txt
	cp -f -r $(BUILD_ROOT)/*.so          $(DESTDIR)/usr/$(USR_LIB)/
	cp -f    $(BUILD_ROOT)/sView         $(DESTDIR)/usr/$(USR_LIB)/sView/sView
	ln --force --symbolic ../$(USR_LIB)/sView/sView       $(DESTDIR)/usr/bin/sView
	ln --force --symbolic ../../share/sView/demo/demo.jps $(DESTDIR)/usr/$(USR_LIB)/sView/demo.jps
	rm -f    $(DESTDIR)/usr/$(USR_LIB)/sView/*.a

install_android:
	mkdir -p $(aDestAndroid)/assets/info
	mkdir -p $(aDestAndroid)/assets/lang/German
	mkdir -p $(aDestAndroid)/assets/lang/French
	mkdir -p $(aDestAndroid)/assets/lang/English
	mkdir -p $(aDestAndroid)/assets/lang/Russian
	mkdir -p $(aDestAndroid)/assets/lang/Czech
	mkdir -p $(aDestAndroid)/assets/lang/ChineseS
	mkdir -p $(aDestAndroid)/assets/lang/Korean
	mkdir -p $(aDestAndroid)/assets/shaders
	mkdir -p $(aDestAndroid)/assets/textures
	cp -f -r $(BUILD_ROOT)/lang/Deutsch/*  $(aDestAndroid)/assets/lang/German/
	cp -f -r $(BUILD_ROOT)/lang/français/* $(aDestAndroid)/assets/lang/French/
	cp -f -r $(BUILD_ROOT)/lang/English/*  $(aDestAndroid)/assets/lang/English/
	cp -f -r $(BUILD_ROOT)/lang/русский/*  $(aDestAndroid)/assets/lang/Russian/
	cp -f -r $(BUILD_ROOT)/lang/Czech/*    $(aDestAndroid)/assets/lang/Czech/
	cp -f -r $(BUILD_ROOT)/lang/ChineseS/* $(aDestAndroid)/assets/lang/ChineseS/
	cp -f -r $(BUILD_ROOT)/lang/Korean/*   $(aDestAndroid)/assets/lang/Korean/
	cp -f -r $(BUILD_ROOT)/shaders/*       $(aDestAndroid)/assets/shaders/
	cp -f -r $(BUILD_ROOT)/textures/*      $(aDestAndroid)/assets/textures/
	cp -f    license-gpl-3.0.txt           $(aDestAndroid)/assets/info/license.txt

install_android_libs:
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStShared)       $(aDestAndroid)/libs/armeabi-v7a/$(aStShared)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStGLWidgets)    $(aDestAndroid)/libs/armeabi-v7a/$(aStGLWidgets)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStCore)         $(aDestAndroid)/libs/armeabi-v7a/$(aStCore)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStOutAnaglyph)  $(aDestAndroid)/libs/armeabi-v7a/$(aStOutAnaglyph)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStOutInterlace) $(aDestAndroid)/libs/armeabi-v7a/$(aStOutInterlace)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStOutDistorted) $(aDestAndroid)/libs/armeabi-v7a/$(aStOutDistorted)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStImageViewer)  $(aDestAndroid)/libs/armeabi-v7a/$(aStImageViewer)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStMoviePlayer)  $(aDestAndroid)/libs/armeabi-v7a/$(aStMoviePlayer)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(sViewAndroid)    $(aDestAndroid)/libs/armeabi-v7a/$(sViewAndroid)

install_android_cad_libs:
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStShared)       $(aDestAndroid)/libs/armeabi-v7a/$(aStShared)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStGLWidgets)    $(aDestAndroid)/libs/armeabi-v7a/$(aStGLWidgets)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStCore)         $(aDestAndroid)/libs/armeabi-v7a/$(aStCore)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStOutAnaglyph)  $(aDestAndroid)/libs/armeabi-v7a/$(aStOutAnaglyph)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStOutInterlace) $(aDestAndroid)/libs/armeabi-v7a/$(aStOutInterlace)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(aStOutDistorted) $(aDestAndroid)/libs/armeabi-v7a/$(aStOutDistorted)
	ln --force --symbolic ../../../$(BUILD_ROOT)/$(sViewAndroidCad) $(aDestAndroid)/libs/armeabi-v7a/$(sViewAndroid)

pre_all:
	mkdir -p $(BUILD_ROOT)/lang/English
	mkdir -p $(BUILD_ROOT)/lang/русский
	mkdir -p $(BUILD_ROOT)/lang/français
	mkdir -p $(BUILD_ROOT)/lang/Deutsch
	mkdir -p $(BUILD_ROOT)/lang/Czech
	mkdir -p $(BUILD_ROOT)/lang/ChineseS
	mkdir -p $(BUILD_ROOT)/lang/Korean
	mkdir -p $(BUILD_ROOT)/textures
	mkdir -p $(BUILD_ROOT)/web
	mkdir -p sview/libs/armeabi-v7a
	mkdir -p StCADViewer/libs/armeabi-v7a
	cp -f -r textures/* $(BUILD_ROOT)/textures/

# StShared static shared library
aStShared_SRCS := $(wildcard StShared/*.cpp)
aStShared_OBJS := ${aStShared_SRCS:.cpp=.o}
aStShared_LIB  := $(LIB) $(LIB_GLX) $(LIB_GTK) $(LIB_ANDROID) -lavutil -lavformat -lavcodec -lswscale -lfreetype $(LIB_CONFIG) $(LIB_PTHREAD)
$(aStShared) : $(aStShared_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStShared_OBJS) $(aStShared_LIB) -o $(BUILD_ROOT)/$(aStShared)
clean_StShared:
	rm -f $(BUILD_ROOT)/$(aStShared)
	rm -rf StShared/*.o

# StGLWidgets static shared library
aStGLWidgets_SRCS := $(wildcard StGLWidgets/*.cpp)
aStGLWidgets_OBJS := ${aStGLWidgets_SRCS:.cpp=.o}
aStGLWidgets_LIB  := $(LIB) -lStShared $(LIB_GLX)
$(aStGLWidgets) : pre_StGLWidgets $(aStShared) $(aStGLWidgets_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStGLWidgets_OBJS) $(aStGLWidgets_LIB) -o $(BUILD_ROOT)/$(aStGLWidgets)
pre_StGLWidgets:

clean_StGLWidgets:
	rm -f $(BUILD_ROOT)/$(aStGLWidgets)
	rm -rf StGLWidgets/*.o

# StCore library
aStCore_SRCS := $(wildcard StCore/*.cpp)
aStCore_OBJS := ${aStCore_SRCS:.cpp=.o}
aStCore_LIB  := $(LIB) -lStShared $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD) $(LIB_XLIB) $(LIB_ANDROID)
$(aStCore) : $(aStShared) $(aStCore_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStCore_OBJS) $(aStCore_LIB) -o $(BUILD_ROOT)/$(aStCore)
clean_StCore:
	rm -f $(BUILD_ROOT)/$(aStCore)
	rm -rf StCore/*.o

# StOutAnaglyph library (Anaglyph output)
aStOutAnaglyph_SRCS := $(wildcard StOutAnaglyph/*.cpp)
aStOutAnaglyph_OBJS := ${aStOutAnaglyph_SRCS:.cpp=.o}
aStOutAnaglyph_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutAnaglyph) : pre_StOutAnaglyph $(aStCore) $(aStOutAnaglyph_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutAnaglyph_OBJS) $(aStOutAnaglyph_LIB) -o $(BUILD_ROOT)/$(aStOutAnaglyph)
pre_StOutAnaglyph:
	mkdir -p $(BUILD_ROOT)/shaders/StOutAnaglyph/
	cp -f -r StOutAnaglyph/shaders/*      $(BUILD_ROOT)/shaders/StOutAnaglyph/
	cp -f -r StOutAnaglyph/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutAnaglyph/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutAnaglyph/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutAnaglyph/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutAnaglyph/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutAnaglyph/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutAnaglyph/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutAnaglyph:
	rm -f $(BUILD_ROOT)/$(aStOutAnaglyph)
	rm -rf StOutAnaglyph/*.o
	rm -rf $(BUILD_ROOT)/shaders/*
	rm -rf $(BUILD_ROOT)/lang/*/StOutAnaglyph.lng

# StOutDual library (Dual output)
aStOutDual_SRCS := $(wildcard StOutDual/*.cpp)
aStOutDual_OBJS := ${aStOutDual_SRCS:.cpp=.o}
aStOutDual_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutDual) : pre_StOutDual $(aStCore) $(aStOutDual_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutDual_OBJS) $(aStOutDual_LIB) -o $(BUILD_ROOT)/$(aStOutDual)
pre_StOutDual:
	cp -f -r StOutDual/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutDual/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutDual/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutDual/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutDual/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutDual/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutDual/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutDual:
	rm -f $(BUILD_ROOT)/$(aStOutDual)
	rm -rf StOutDual/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StOutDual.lng

# StOutIZ3D library (iZ3D monitor)
aStOutIZ3D_SRCS := $(wildcard StOutIZ3D/*.cpp)
aStOutIZ3D_OBJS := ${aStOutIZ3D_SRCS:.cpp=.o}
aStOutIZ3D_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutIZ3D) : pre_StOutIZ3D $(aStCore) $(aStOutIZ3D_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutIZ3D_OBJS) $(aStOutIZ3D_LIB) -o $(BUILD_ROOT)/$(aStOutIZ3D)
pre_StOutIZ3D:
	mkdir -p $(BUILD_ROOT)/shaders/StOutIZ3D/
	cp -f -r StOutIZ3D/shaders/*      $(BUILD_ROOT)/shaders/StOutIZ3D/
	cp -f -r StOutIZ3D/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutIZ3D/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutIZ3D/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutIZ3D/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutIZ3D/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutIZ3D/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutIZ3D/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutIZ3D:
	rm -f $(BUILD_ROOT)/$(aStOutIZ3D)
	rm -rf StOutIZ3D/*.o
	rm -rf $(BUILD_ROOT)/shaders/*
	rm -rf $(BUILD_ROOT)/lang/*/StOutIZ3D.lng

# StOutInterlace library (Interlaced output)
aStOutInterlace_SRCS := $(wildcard StOutInterlace/*.cpp)
aStOutInterlace_OBJS := ${aStOutInterlace_SRCS:.cpp=.o}
aStOutInterlace_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutInterlace) : pre_StOutInterlace $(aStCore) $(aStOutInterlace_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutInterlace_OBJS) $(aStOutInterlace_LIB) -o $(BUILD_ROOT)/$(aStOutInterlace)
pre_StOutInterlace:
	mkdir -p $(BUILD_ROOT)/shaders/StOutInterlace/
	cp -f -r StOutInterlace/shaders/*      $(BUILD_ROOT)/shaders/StOutInterlace/
	cp -f -r StOutInterlace/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutInterlace/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutInterlace/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutInterlace/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutInterlace/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutInterlace/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutInterlace/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutInterlace:
	rm -f $(BUILD_ROOT)/$(aStOutInterlace)
	rm -rf StOutInterlace/*.o
	rm -rf $(BUILD_ROOT)/shaders/*
	rm -rf $(BUILD_ROOT)/lang/*/StOutInterlace.lng

# StOutPageFlip library (Shutter glasses output)
aStOutPageFlip_SRCS := $(wildcard StOutPageFlip/*.cpp)
aStOutPageFlip_OBJS := ${aStOutPageFlip_SRCS:.cpp=.o}
aStOutPageFlip_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutPageFlip) : pre_StOutPageFlip $(aStCore) $(aStOutPageFlip_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutPageFlip_OBJS) $(aStOutPageFlip_LIB) -o $(BUILD_ROOT)/$(aStOutPageFlip)
pre_StOutPageFlip:
	cp -f -r StOutPageFlip/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutPageFlip/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutPageFlip/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutPageFlip/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutPageFlip/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutPageFlip/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutPageFlip/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutPageFlip:
	rm -f $(BUILD_ROOT)/$(aStOutPageFlip)
	rm -rf StOutPageFlip/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StOutPageFlip.lng

# StOutDistorted library
aStOutDistorted_SRCS := $(wildcard StOutDistorted/*.cpp)
aStOutDistorted_OBJS := ${aStOutDistorted_SRCS:.cpp=.o}
aStOutDistorted_LIB  := $(LIB) -lStShared -lStCore $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStOutDistorted) : pre_StOutDistorted $(aStCore) $(aStOutDistorted_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutDistorted_OBJS) $(aStOutDistorted_LIB) -o $(BUILD_ROOT)/$(aStOutDistorted)
pre_StOutDistorted:
	cp -f -r StOutDistorted/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StOutDistorted/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutDistorted/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StOutDistorted/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StOutDistorted/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StOutDistorted/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StOutDistorted/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StOutDistorted:
	rm -f $(BUILD_ROOT)/$(aStOutDistorted)
	rm -rf StOutDistorted/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StOutDistorted.lng

# StImageViewer library (Image Viewer)
aStImageViewer_SRCS := $(wildcard StImageViewer/*.cpp)
aStImageViewer_OBJS := ${aStImageViewer_SRCS:.cpp=.o}
aStImageViewer_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStImageViewer) : pre_StImageViewer $(aStGLWidgets) outputs_all $(aStImageViewer_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStImageViewer_OBJS) $(aStImageViewer_LIB) -o $(BUILD_ROOT)/$(aStImageViewer)
pre_StImageViewer:
	cp -f -r StImageViewer/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StImageViewer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StImageViewer/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StImageViewer/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StImageViewer/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StImageViewer/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StImageViewer/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StImageViewer:
	rm -f $(BUILD_ROOT)/$(aStImageViewer)
	rm -rf StImageViewer/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StImageViewer.lng

# StMoviePlayer library (Image Viewer)
aStMoviePlayer_SRCS1 := $(wildcard StMoviePlayer/*.cpp)
aStMoviePlayer_OBJS1 := ${aStMoviePlayer_SRCS1:.cpp=.o}
aStMoviePlayer_SRCS2 := $(wildcard StMoviePlayer/StVideo/*.cpp)
aStMoviePlayer_OBJS2 := ${aStMoviePlayer_SRCS2:.cpp=.o}
aStMoviePlayer_SRCS3 := $(wildcard StMoviePlayer/*.c)
aStMoviePlayer_OBJS3 := ${aStMoviePlayer_SRCS3:.c=.o}
aStMoviePlayer_LIB   := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_GLX) $(LIB_GTK) -lavutil -lavformat -lavcodec -lswscale -lopenal $(LIB_PTHREAD)
$(aStMoviePlayer) : pre_StMoviePlayer $(aStGLWidgets) outputs_all $(aStMoviePlayer_OBJS1) $(aStMoviePlayer_OBJS2) $(aStMoviePlayer_OBJS3)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStMoviePlayer_OBJS1) $(aStMoviePlayer_OBJS2) $(aStMoviePlayer_OBJS3) $(aStMoviePlayer_LIB) -o $(BUILD_ROOT)/$(aStMoviePlayer)
pre_StMoviePlayer:
	cp -f -r StMoviePlayer/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StMoviePlayer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StMoviePlayer/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StMoviePlayer/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StMoviePlayer/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StMoviePlayer/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StMoviePlayer/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
	cp -f -r StMoviePlayer/web/*          $(BUILD_ROOT)/web/
clean_StMoviePlayer:
	rm -f $(BUILD_ROOT)/$(aStMoviePlayer)
	rm -rf StMoviePlayer/*.o
	rm -rf StMoviePlayer/StVideo/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StMoviePlayer.lng
	rm -rf $(BUILD_ROOT)/web/*

# StDiagnostics library
aStDiagnostics_SRCS := $(wildcard StDiagnostics/*.cpp)
aStDiagnostics_OBJS := ${aStDiagnostics_SRCS:.cpp=.o}
aStDiagnostics_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
$(aStDiagnostics) : pre_StDiagnostics $(aStGLWidgets) outputs_all $(aStDiagnostics_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStDiagnostics_OBJS) $(aStDiagnostics_LIB) -o $(BUILD_ROOT)/$(aStDiagnostics)
pre_StDiagnostics:
	cp -f -r StDiagnostics/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StDiagnostics/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StDiagnostics/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StDiagnostics/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StDiagnostics/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StDiagnostics/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StDiagnostics/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StDiagnostics:
	rm -f $(BUILD_ROOT)/$(aStDiagnostics)
	rm -rf StDiagnostics/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StDiagnostics.lng

# sView CAD Android JNI executable
sViewAndroidCad_SRCS := $(wildcard StCADViewer/*.cpp)
sViewAndroidCad_OBJS := ${sViewAndroidCad_SRCS:.cpp=.o}
sViewAndroidCad_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_OCCT) -lfreetype -llog -landroid -lEGL -lGLESv2 -lc
$(sViewAndroidCad) : pre_StCADViewer $(aStGLWidgets) outputs_all $(sViewAndroidCad_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(sViewAndroidCad_OBJS) $(sViewAndroidCad_LIB) -o $(BUILD_ROOT)/$(sViewAndroidCad)
clean_sViewAndroidCad:
	rm -f $(BUILD_ROOT)/$(sViewAndroidCad)
	rm -rf StCADViewer/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StCADViewer.lng

# StCADViewer library
aStCADViewer_SRCS := $(wildcard StCADViewer/*.cpp)
aStCADViewer_OBJS := ${aStCADViewer_SRCS:.cpp=.o}
aStCADViewer_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore $(LIB_OUTPUTS) $(LIB_GLX) $(LIB_GTK) $(LIB_PTHREAD)
aStCADViewer_LIB  += $(LIB_OCCT)
$(aStCADViewer) : pre_StCADViewer $(aStGLWidgets) outputs_all $(aStCADViewer_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStCADViewer_OBJS) $(aStCADViewer_LIB) -o $(BUILD_ROOT)/$(aStCADViewer)
pre_StCADViewer:
	cp -f -r StCADViewer/lang/english/* $(BUILD_ROOT)/lang/English/
	cp -f -r StCADViewer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StCADViewer/lang/french/*  $(BUILD_ROOT)/lang/français/
	cp -f -r StCADViewer/lang/german/*  $(BUILD_ROOT)/lang/Deutsch/
	cp -f -r StCADViewer/lang/czech/*   $(BUILD_ROOT)/lang/Czech/
	cp -f -r StCADViewer/lang/chinese/* $(BUILD_ROOT)/lang/ChineseS/
	cp -f -r StCADViewer/lang/korean/*  $(BUILD_ROOT)/lang/Korean/
clean_StCADViewer:
	rm -f $(BUILD_ROOT)/$(aStCADViewer)
	rm -rf StCADViewer/*.o
	rm -rf $(BUILD_ROOT)/lang/*/StCADViewer.lng

# sView Android JNI executable
sViewAndroid_SRCS := $(wildcard sview/jni/*.cpp)
sViewAndroid_OBJS := ${sViewAndroid_SRCS:.cpp=.o}
sViewAndroid_LIB  := $(LIB) -lStShared -lStCore -lStImageViewer -lStMoviePlayer -llog -landroid -lEGL -lGLESv2 -lc
$(sViewAndroid) : $(aStImageViewer) $(aStMoviePlayer) $(sViewAndroid_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(sViewAndroid_OBJS) $(sViewAndroid_LIB) -o $(BUILD_ROOT)/$(sViewAndroid)
clean_sViewAndroid:
	rm -f $(BUILD_ROOT)/$(sViewAndroid)
	rm -rf sview/jni/*.o

# sView executable
sView_SRCS := $(wildcard sview/*.cpp)
sView_OBJS := ${sView_SRCS:.cpp=.o}
sView_LIB  := $(LIB) -lStGLWidgets -lStShared -lStCore -lStImageViewer -lStMoviePlayer -lStDiagnostics $(LIB_OUTPUTS) $(LIB_GTK) -lX11 -ldl -lgthread-2.0 $(LIB_PTHREAD)
$(sView) : $(aStImageViewer) $(aStMoviePlayer) $(aStDiagnostics) $(sView_OBJS)
	$(LD) $(LDFLAGS) $(LIBDIR) $(sView_OBJS) $(sView_LIB) -o $(BUILD_ROOT)/$(sView)
clean_sView:
	rm -f $(BUILD_ROOT)/$(sView)
	rm -rf sview/*.o

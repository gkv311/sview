# Makefile for sView project

WORKDIR = `pwd`

LBITS := $(shell getconf LONG_BIT)

CC = gcc
CXX = g++
AR = ar
LD = g++

INC =  -I3rdparty/include -Iinclude
CXXFLAGS = -O3 -std=c++0x -Wall -fPIC -mmmx -msse `pkg-config gtk+-2.0 --cflags` $(INC)
LIBDIR = -L$(BUILD_ROOT)
LIB =
LDFLAGS = -s
LIB_GLX = -lGL -lX11 -lXext
LIB_GTK = `pkg-config gtk+-2.0 --libs` -lgthread-2.0 -ldl
BUILD_ROOT = build
USR_LIB = lib

aStShared       := $(BUILD_ROOT)/libStShared.so
aStGLWidgets    := $(BUILD_ROOT)/libStGLWidgets.so
aStSettings     := $(BUILD_ROOT)/libStSettings.so
aStCore         := $(BUILD_ROOT)/libStCore.so
aStOutAnaglyph  := $(BUILD_ROOT)/libStOutAnaglyph.so
aStOutDual      := $(BUILD_ROOT)/libStOutDual.so
aStOutInterlace := $(BUILD_ROOT)/libStOutInterlace.so
aStOutPageFlip  := $(BUILD_ROOT)/libStOutPageFlip.so
aStOutIZ3D      := $(BUILD_ROOT)/libStOutIZ3D.so
aStOutDistorted := $(BUILD_ROOT)/libStOutDistorted.so
aStImageViewer  := $(BUILD_ROOT)/libStImageViewer.so
aStMoviePlayer  := $(BUILD_ROOT)/libStMoviePlayer.so
aStDiagnostics  := $(BUILD_ROOT)/libStDiagnostics.so
aStCADViewer    := $(BUILD_ROOT)/libStCADViewer.so
sView           := $(BUILD_ROOT)/sView

all:   pre_all $(aStShared) $(aStGLWidgets) $(aStSettings) $(aStCore) $(aStOutAnaglyph) $(aStOutDual) $(aStOutInterlace) $(aStOutPageFlip) $(aStOutIZ3D) $(aStOutDistorted) $(aStImageViewer) $(aStMoviePlayer) $(aStDiagnostics) $(aStCADViewer) $(sView)
clean: clean_StShared clean_StGLWidgets clean_StSettings clean_StCore clean_sView clean_StOutAnaglyph clean_StOutDual clean_StOutInterlace clean_StOutPageFlip clean_StOutIZ3D clean_StOutDistorted clean_StImageViewer clean_StMoviePlayer clean_StDiagnostics clean_StCADViewer
distclean: clean

install:
	mkdir -p $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/$(USR_LIB)/sView/info
	mkdir -p $(DESTDIR)/usr/$(USR_LIB)/firefox/plugins
	mkdir -p $(DESTDIR)/usr/$(USR_LIB)/mozilla/plugins
	mkdir -p $(DESTDIR)/usr/share
	cp -f -r share/*                     $(DESTDIR)/usr/share/
	cp -f    license-gpl-3.0.txt         $(DESTDIR)/usr/$(USR_LIB)/sView/info/license.txt
	cp -f -r $(BUILD_ROOT)/*             $(DESTDIR)/usr/$(USR_LIB)/sView/
	ln --force --symbolic ../$(USR_LIB)/sView/sView      $(DESTDIR)/usr/bin/sView
	ln --force --symbolic ../../share/sView/demo/demo.jps $(DESTDIR)/usr/$(USR_LIB)/sView/demo.jps
	rm -f    $(DESTDIR)/usr/$(USR_LIB)/sView/*.a

pre_all:
	mkdir -p $(BUILD_ROOT)/lang/english
	mkdir -p $(BUILD_ROOT)/lang/русский
	mkdir -p $(BUILD_ROOT)/lang/français
	mkdir -p $(BUILD_ROOT)/textures
	cp -f -r textures/*.std $(BUILD_ROOT)/textures/

# StShared static shared library
aStShared_SRCS := $(wildcard StShared/*.cpp)
aStShared_OBJS := ${aStShared_SRCS:.cpp=.o}
aStShared_LIB  := $(LIB) $(LIB_GLX) $(LIB_GTK) -lavutil -lavformat -lavcodec -lswscale -lfreetype -lpthread
$(aStShared) : $(aStShared_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStShared_OBJS) $(aStShared_LIB) -o $(aStShared)
clean_StShared:
	rm -f $(aStShared)
	rm -rf StShared/*.o

# StGLWidgets static shared library
aStGLWidgets_SRCS := $(wildcard StGLWidgets/*.cpp)
aStGLWidgets_OBJS := ${aStGLWidgets_SRCS:.cpp=.o}
aStGLWidgets_LIB  := $(LIB) -lStShared $(LIB_GLX)
$(aStGLWidgets) : pre_StGLWidgets $(aStGLWidgets_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStGLWidgets_OBJS) $(aStGLWidgets_LIB) -o $(aStGLWidgets)
pre_StGLWidgets:
	mkdir -p $(BUILD_ROOT)/shaders/StGLWidgets/
	cp -f -r StGLWidgets/shaders/* $(BUILD_ROOT)/shaders/StGLWidgets/
clean_StGLWidgets:
	rm -f $(aStGLWidgets)
	rm -rf StGLWidgets/*.o

# StSetting shared library
aStSettings_SRCS := $(wildcard StSettings/*.cpp)
aStSettings_OBJS := ${aStSettings_SRCS:.cpp=.o}
aStSettings_LIB  := $(LIB) -lStShared $(LIB_GTK) -lconfig++
$(aStSettings) : $(aStSettings_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStSettings_OBJS) $(aStSettings_LIB) -o $(aStSettings)
clean_StSettings:
	rm -f $(aStSettings)
	rm -rf StSettings/*.o

# StCore library
aStCore_SRCS := $(wildcard StCore/*.cpp)
aStCore_OBJS := ${aStCore_SRCS:.cpp=.o}
aStCore_LIB  := $(LIB) -lStShared -lStSettings $(LIB_GLX) $(LIB_GTK) -lpthread -lXrandr -lXpm
$(aStCore) : $(aStCore_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStCore_OBJS) $(aStCore_LIB) -o $(aStCore)
clean_StCore:
	rm -f $(aStCore)
	rm -rf StCore/*.o

# StOutAnaglyph library (Anaglyph output)
aStOutAnaglyph_SRCS := $(wildcard StOutAnaglyph/*.cpp)
aStOutAnaglyph_OBJS := ${aStOutAnaglyph_SRCS:.cpp=.o}
aStOutAnaglyph_LIB  := $(LIB) -lStShared -lStSettings -lStCore $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStOutAnaglyph) : pre_StOutAnaglyph $(aStOutAnaglyph_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStOutAnaglyph_OBJS) $(aStOutAnaglyph_LIB) -o $(aStOutAnaglyph)
pre_StOutAnaglyph:
	mkdir -p $(BUILD_ROOT)/shaders/StOutAnaglyph/
	cp -f -r StOutAnaglyph/shaders/*      $(BUILD_ROOT)/shaders/StOutAnaglyph/
	cp -f -r StOutAnaglyph/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StOutAnaglyph/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutAnaglyph/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StOutAnaglyph:
	rm -f $(aStOutAnaglyph)
	rm -rf StOutAnaglyph/*.o

# StOutDual library (Dual output)
aStOutDual_SRCS := $(wildcard StOutDual/*.cpp)
aStOutDual_OBJS := ${aStOutDual_SRCS:.cpp=.o}
aStOutDual_LIB  := $(LIB) -lStShared -lStSettings -lStCore $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStOutDual) : pre_StOutDual $(aStOutDual_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStOutDual_OBJS) $(aStOutDual_LIB) -o $(aStOutDual)
pre_StOutDual:
	cp -f -r StOutDual/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StOutDual/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutDual/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StOutDual:
	rm -f $(aStOutDual)
	rm -rf StOutDual/*.o

# StOutIZ3D library (iZ3D monitor)
aStOutIZ3D_SRCS := $(wildcard StOutIZ3D/*.cpp)
aStOutIZ3D_OBJS := ${aStOutIZ3D_SRCS:.cpp=.o}
aStOutIZ3D_LIB  := $(LIB) -lStShared -lStSettings -lStCore $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStOutIZ3D) : pre_StOutIZ3D $(aStOutIZ3D_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStOutIZ3D_OBJS) $(aStOutIZ3D_LIB) -o $(aStOutIZ3D)
pre_StOutIZ3D:
	mkdir -p $(BUILD_ROOT)/shaders/StOutIZ3D/
	cp -f -r StOutIZ3D/shaders/*      $(BUILD_ROOT)/shaders/StOutIZ3D/
	cp -f -r StOutIZ3D/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StOutIZ3D/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutIZ3D/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StOutIZ3D:
	rm -f $(aStOutIZ3D)
	rm -rf StOutIZ3D/*.o

# StOutInterlace library (Interlaced output)
aStOutInterlace_SRCS := $(wildcard StOutInterlace/*.cpp)
aStOutInterlace_OBJS := ${aStOutInterlace_SRCS:.cpp=.o}
aStOutInterlace_LIB  := $(LIB) -lStShared -lStSettings -lStCore $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStOutInterlace) : pre_StOutInterlace $(aStOutInterlace_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStOutInterlace_OBJS) $(aStOutInterlace_LIB) -o $(aStOutInterlace)
pre_StOutInterlace:
	mkdir -p $(BUILD_ROOT)/shaders/StOutInterlace/
	cp -f -r StOutInterlace/shaders/*      $(BUILD_ROOT)/shaders/StOutInterlace/
	cp -f -r StOutInterlace/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StOutInterlace/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutInterlace/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StOutInterlace:
	rm -f $(aStOutInterlace)
	rm -rf StOutInterlace/*.o

# StOutPageFlip library (Shutter glasses output)
aStOutPageFlip_SRCS := $(wildcard StOutPageFlip/*.cpp)
aStOutPageFlip_OBJS := ${aStOutPageFlip_SRCS:.cpp=.o}
aStOutPageFlip_LIB  := $(LIB) -lStShared -lStSettings -lStCore $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStOutPageFlip) : pre_StOutPageFlip $(aStOutPageFlip_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStOutPageFlip_OBJS) $(aStOutPageFlip_LIB) -o $(aStOutPageFlip)
pre_StOutPageFlip:
	cp -f -r StOutPageFlip/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StOutPageFlip/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutPageFlip/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StOutPageFlip:
	rm -f $(aStOutPageFlip)
	rm -rf StOutPageFlip/*.o

# StOutDistorted library
aStOutDistorted_SRCS := $(wildcard StOutDistorted/*.cpp)
aStOutDistorted_OBJS := ${aStOutDistorted_SRCS:.cpp=.o}
aStOutDistorted_LIB  := $(LIB) -lStShared -lStSettings -lStCore $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStOutDistorted) : pre_StOutDistorted $(aStOutDistorted_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStOutDistorted_OBJS) $(aStOutDistorted_LIB) -o $(aStOutDistorted)
pre_StOutDistorted:
	cp -f -r StOutDistorted/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StOutDistorted/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutDistorted/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StOutDistorted:
	rm -f $(aStOutDistorted)
	rm -rf StOutDistorted/*.o

# StImageViewer library (Image Viewer)
aStImageViewer_SRCS := $(wildcard StImageViewer/*.cpp)
aStImageViewer_OBJS := ${aStImageViewer_SRCS:.cpp=.o}
aStImageViewer_LIB  := $(LIB) -lStGLWidgets -lStShared -lStSettings -lStCore -lStOutAnaglyph -lStOutDual -lStOutInterlace -lStOutPageFlip -lStOutIZ3D -lStOutDistorted $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStImageViewer) : pre_StImageViewer $(aStImageViewer_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStImageViewer_OBJS) $(aStImageViewer_LIB) -o $(aStImageViewer)
pre_StImageViewer:
	cp -f -r StImageViewer/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StImageViewer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StImageViewer/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StImageViewer:
	rm -f $(aStImageViewer)
	rm -rf StImageViewer/*.o

# StMoviePlayer library (Image Viewer)
aStMoviePlayer_SRCS1 := $(wildcard StMoviePlayer/*.cpp)
aStMoviePlayer_OBJS1 := ${aStMoviePlayer_SRCS1:.cpp=.o}
aStMoviePlayer_SRCS2 := $(wildcard StMoviePlayer/StVideo/*.cpp)
aStMoviePlayer_OBJS2 := ${aStMoviePlayer_SRCS2:.cpp=.o}
aStMoviePlayer_LIB   := $(LIB) -lStGLWidgets -lStShared -lStSettings -lStCore -lStOutAnaglyph -lStOutDual -lStOutInterlace -lStOutPageFlip -lStOutIZ3D -lStOutDistorted $(LIB_GLX) $(LIB_GTK) -lavutil -lavformat -lavcodec -lswscale -lopenal -lpthread
$(aStMoviePlayer) : pre_StMoviePlayer $(aStMoviePlayer_OBJS1) $(aStMoviePlayer_OBJS2)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStMoviePlayer_OBJS1) $(aStMoviePlayer_OBJS2) $(aStMoviePlayer_LIB) -o $(aStMoviePlayer)
pre_StMoviePlayer:
	cp -f -r StMoviePlayer/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StMoviePlayer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StMoviePlayer/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StMoviePlayer:
	rm -f $(aStMoviePlayer)
	rm -rf StMoviePlayer/*.o

# StDiagnostics library
aStDiagnostics_SRCS := $(wildcard StDiagnostics/*.cpp)
aStDiagnostics_OBJS := ${aStDiagnostics_SRCS:.cpp=.o}
aStDiagnostics_LIB  := $(LIB) -lStGLWidgets -lStShared -lStSettings -lStCore -lStOutAnaglyph -lStOutDual -lStOutInterlace -lStOutPageFlip -lStOutIZ3D -lStOutDistorted $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStDiagnostics) : pre_StDiagnostics $(aStDiagnostics_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStDiagnostics_OBJS) $(aStDiagnostics_LIB) -o $(aStDiagnostics)
pre_StDiagnostics:
	cp -f -r StDiagnostics/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StDiagnostics/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StDiagnostics/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StDiagnostics:
	rm -f $(aStDiagnostics)
	rm -rf StDiagnostics/*.o

# StCADViewer library
aStCADViewer_SRCS := $(wildcard StCADViewer/*.cpp)
aStCADViewer_OBJS := ${aStCADViewer_SRCS:.cpp=.o}
aStCADViewer_LIB  := $(LIB) -lStGLWidgets -lStShared -lStSettings -lStCore -lStOutAnaglyph -lStOutDual -lStOutInterlace -lStOutPageFlip -lStOutIZ3D -lStOutDistorted $(LIB_GLX) $(LIB_GTK) -lpthread
$(aStCADViewer) : pre_StCADViewer $(aStCADViewer_OBJS)
	$(LD) -shared -z defs $(LDFLAGS) $(LIBDIR) $(aStCADViewer_OBJS) $(aStCADViewer_LIB) -o $(aStCADViewer)
pre_StCADViewer:
	cp -f -r StCADViewer/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StCADViewer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StCADViewer/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StCADViewer:
	rm -f $(aStCADViewer)
	rm -rf StCADViewer/*.o

# sView executable
sView_SRCS := $(wildcard sview/*.cpp)
sView_OBJS := ${sView_SRCS:.cpp=.o}
sView_LIB  := $(LIB) -lStGLWidgets -lStShared -lStSettings -lStCore -lStImageViewer -lStMoviePlayer -lStDiagnostics -lStCADViewer -lStOutAnaglyph -lStOutDual -lStOutInterlace -lStOutPageFlip -lStOutIZ3D -lStOutDistorted $(LIB_GTK) -lX11 -ldl -lgthread-2.0 -lpthread
$(sView) : $(sView_OBJS)
	$(LD) $(LDFLAGS) $(LIBDIR) $(sView_OBJS) $(sView_LIB) -o $(sView)
clean_sView:
	rm -f $(sView)
	rm -rf sview/*.o

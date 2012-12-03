# Makefile for sView project

WORKDIR = `pwd`

LBITS := $(shell getconf LONG_BIT)

CC = gcc
CXX = g++
AR = ar
LD = g++

INC =  -I3rdparty/include -Iinclude
CXXFLAGS = -O3 -std=c++0x -Wall -fPIC -mmmx -msse `pkg-config gtk+-2.0 --cflags` $(INC)
LIBDIR =
LIB =
LDFLAGS = -s
LIB_GLEW = -lGL -lGLEW -lX11 -lXext
BUILD_ROOT = build
USR_LIB = lib

aStShared       := $(BUILD_ROOT)/libStShared.a
aStGLWidgets    := $(BUILD_ROOT)/libStGLWidgets.a
aStSettings     := $(BUILD_ROOT)/StSettings.so
aStCore         := $(BUILD_ROOT)/StCore.so
aStOutAnaglyph  := $(BUILD_ROOT)/StRenderers/StOutAnaglyph.so
aStOutDual      := $(BUILD_ROOT)/StRenderers/StOutDual.so
aStOutInterlace := $(BUILD_ROOT)/StRenderers/StOutInterlace.so
aStOutPageFlip  := $(BUILD_ROOT)/StRenderers/StOutPageFlip.so
aStImageViewer  := $(BUILD_ROOT)/StDrawers/StImageViewer.so
aStMoviePlayer  := $(BUILD_ROOT)/StDrawers/StMoviePlayer.so
sView           := $(BUILD_ROOT)/sView

all:   pre_all $(aStShared) $(aStGLWidgets) $(aStSettings) $(aStCore) $(sView) $(aStOutAnaglyph) $(aStOutDual) $(aStOutInterlace) $(aStOutPageFlip) $(aStImageViewer) $(aStMoviePlayer)
clean: clean_StShared clean_StGLWidgets clean_StSettings clean_StCore clean_sView clean_StOutAnaglyph clean_StOutDual clean_StOutInterlace clean_StOutPageFlip clean_StImageViewer clean_StMoviePlayer
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
	mkdir -p $(BUILD_ROOT)/StRenderers
	mkdir -p $(BUILD_ROOT)/StDrawers
	mkdir -p $(BUILD_ROOT)/StBrowserPlugins
	mkdir -p $(BUILD_ROOT)/lang/english
	mkdir -p $(BUILD_ROOT)/lang/русский
	mkdir -p $(BUILD_ROOT)/lang/français
	mkdir -p $(BUILD_ROOT)/textures
	cp -f -r textures/*.std $(BUILD_ROOT)/textures/

# StShared static shared library
aStShared_SRCS := $(wildcard StShared/*.cpp)
aStShared_OBJS := ${aStShared_SRCS:.cpp=.o}
$(aStShared) : $(aStShared_OBJS)
	$(AR) rcs $(aStShared) $(aStShared_OBJS)
clean_StShared:
	rm -f $(aStShared)
	rm -rf StShared/*.o

# StGLWidgets static shared library
aStGLWidgets_SRCS := $(wildcard StGLWidgets/*.cpp)
aStGLWidgets_OBJS := ${aStGLWidgets_SRCS:.cpp=.o}

$(aStGLWidgets) : pre_StGLWidgets $(aStGLWidgets_OBJS)
	$(AR) rcs $(aStGLWidgets) $(aStGLWidgets_OBJS)
pre_StGLWidgets:
	mkdir -p $(BUILD_ROOT)/shaders/StGLWidgets/
	cp -f -r StGLWidgets/shaders/* $(BUILD_ROOT)/shaders/StGLWidgets/
clean_StGLWidgets:
	rm -f $(aStGLWidgets)
	rm -rf StGLWidgets/*.o

# StSetting shared library
aStSettings_SRCS := $(wildcard StSettings/*.cpp)
aStSettings_OBJS := ${aStSettings_SRCS:.cpp=.o}
aStSettings_LIB  := $(LIB) $(aStShared) -lconfig++
$(aStSettings) : $(aStSettings_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStSettings_OBJS) $(aStSettings_LIB) -o $(aStSettings)
clean_StSettings:
	rm -f $(aStSettings)
	rm -rf StSettings/*.o

# StCore library
aStCore_SRCS := $(wildcard StCore/*.cpp)
aStCore_OBJS := ${aStCore_SRCS:.cpp=.o}
aStCore_LIB  := $(LIB) $(aStShared) $(LIB_GLEW) -lpthread -lXrandr -lXpm
$(aStCore) : $(aStCore_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStCore_OBJS) $(aStCore_LIB) -o $(aStCore)
clean_StCore:
	rm -f $(aStCore)
	rm -rf StCore/*.o

# StOutAnaglyph library (Anaglyph output)
aStOutAnaglyph_SRCS := $(wildcard StOutAnaglyph/*.cpp)
aStOutAnaglyph_OBJS := ${aStOutAnaglyph_SRCS:.cpp=.o}
aStOutAnaglyph_LIB  := $(LIB) $(aStShared) $(LIB_GLEW) -lpthread
$(aStOutAnaglyph) : pre_StOutAnaglyph $(aStOutAnaglyph_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutAnaglyph_OBJS) $(aStOutAnaglyph_LIB) -o $(aStOutAnaglyph)
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
aStOutDual_LIB  := $(LIB) $(aStShared) $(LIB_GLEW) -lpthread
$(aStOutDual) : pre_StOutDual $(aStOutDual_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutDual_OBJS) $(aStOutDual_LIB) -o $(aStOutDual)
pre_StOutDual:
	cp -f -r StOutDual/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StOutDual/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutDual/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StOutDual:
	rm -f $(aStOutDual)
	rm -rf StOutDual/*.o

# StOutInterlace library (Interlaced output)
aStOutInterlace_SRCS := $(wildcard StOutInterlace/*.cpp)
aStOutInterlace_OBJS := ${aStOutInterlace_SRCS:.cpp=.o}
aStOutInterlace_LIB  := $(LIB) $(aStShared) $(LIB_GLEW) -lpthread
$(aStOutInterlace) : pre_StOutInterlace $(aStOutInterlace_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutInterlace_OBJS) $(aStOutInterlace_LIB) -o $(aStOutInterlace)
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
aStOutPageFlip_LIB  := $(LIB) $(aStShared) $(LIB_GLEW) -lpthread
$(aStOutPageFlip) : pre_StOutPageFlip $(aStOutPageFlip_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStOutPageFlip_OBJS) $(aStOutPageFlip_LIB) -o $(aStOutPageFlip)
pre_StOutPageFlip:
	cp -f -r StOutPageFlip/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StOutPageFlip/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StOutPageFlip/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StOutPageFlip:
	rm -f $(aStOutPageFlip)
	rm -rf StOutPageFlip/*.o

# StImageViewer library (Image Viewer)
aStImageViewer_SRCS := $(wildcard StImageViewer/*.cpp)
aStImageViewer_OBJS := ${aStImageViewer_SRCS:.cpp=.o}
aStImageViewer_LIB  := $(LIB) $(aStGLWidgets) $(aStShared) $(LIB_GLEW) -lpthread -lavutil -lavformat -lavcodec -lswscale -lfreetype
$(aStImageViewer) : pre_StImageViewer $(aStImageViewer_OBJS)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStImageViewer_OBJS) $(aStImageViewer_LIB) -o $(aStImageViewer)
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
aStMoviePlayer_LIB   := $(LIB) $(aStGLWidgets) $(aStShared) $(LIB_GLEW) -lpthread -lavutil -lavformat -lavcodec -lswscale -lopenal -lfreetype
$(aStMoviePlayer) : pre_StMoviePlayer $(aStMoviePlayer_OBJS1) $(aStMoviePlayer_OBJS2)
	$(LD) -shared $(LDFLAGS) $(LIBDIR) $(aStMoviePlayer_OBJS1) $(aStMoviePlayer_OBJS2) $(aStMoviePlayer_LIB) -o $(aStMoviePlayer)
pre_StMoviePlayer:
	cp -f -r StMoviePlayer/lang/english/* $(BUILD_ROOT)/lang/english/
	cp -f -r StMoviePlayer/lang/russian/* $(BUILD_ROOT)/lang/русский/
	cp -f -r StMoviePlayer/lang/french/*  $(BUILD_ROOT)/lang/français/
clean_StMoviePlayer:
	rm -f $(aStMoviePlayer)
	rm -rf StMoviePlayer/*.o

# sView executable
sView_SRCS := $(wildcard sview/*.cpp)
sView_OBJS := ${sView_SRCS:.cpp=.o}
sView_LIB  := $(LIB) $(aStShared) `pkg-config gtk+-2.0 --libs` -lX11 -ldl -lgthread-2.0 -lpthread
$(sView) : $(sView_OBJS)
	$(LD) $(LDFLAGS) $(LIBDIR) $(sView_OBJS) $(sView_LIB) -o $(sView)
clean_sView:
	rm -f $(sView)
	rm -rf sview/*.o

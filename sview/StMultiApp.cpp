/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2013-2015
 */

#include "StMultiApp.h"

#include "../StImageViewer/StImageViewer.h"
#include "../StImageViewer/StImagePluginInfo.h"
#include "../StMoviePlayer/StMoviePlayer.h"
#include "../StDiagnostics/StDiagnostics.h"

#include <StStrings/stConsole.h>
#include <StVersion.h>

static StString getAbout() {
    StString anAboutString =
        StString("sView ") + StVersionInfo::getSDKVersionString() + '\n'
        + "Copyright (C) 2007-2020 Kirill Gavrilov (kirill@sview.ru).\n"
        + "Usage: sView [options] - file\n"
        + "Available options:\n"
          "  --fullscreen         Open fullscreen\n"
          "  --toShowMenu=off     Hide main menu\n"
          "  --toShowTopbar=off   Hide top toolbar\n"
          "  --monitorId=ID       open window on specified monitor\n"
          "  --windowLeft=L       window left   position\n"
          "  --windowTop=T        window top    position\n"
          "  --windowWidth=W      window width  position\n"
          "  --windowHeight=H     window height position\n"
          "  --slideshow          Start slideshow\n"
          "  --last               Open last file\n"
          "  --paused             Open file in paused state\n"
          "  --seek=SECONDS       Seek to specified position\n"
          "  --in=image,video     Application to open (predefined values: image, video, diag)\n"
          "  --out=RENDERER       Stereoscopic output module (auto, StOutAnaglyph, StOutDual,...)\n"
          "  --imageLib=IMGLIB    Setup 3rd-party library for image processing (FFmpeg, FreeImage, DevIL)\n"
          "  --viewMode=MODE      View mode (flat, sphere)\n"
          "  --srcFormat=FORMAT   Setup source format:\n"
          "                       "
            + st::formatToString(StFormat_AUTO)          + ", "
            + st::formatToString(StFormat_Mono)          + ", "
            + st::formatToString(StFormat_SideBySide_LR) + ", "
            + st::formatToString(StFormat_SideBySide_RL) + ",\n"
        + "                       "
            + st::formatToString(StFormat_TopBottom_LR)  + ", "
            + st::formatToString(StFormat_TopBottom_RL)  + ", "
            + st::formatToString(StFormat_Rows)          + ", "
            + st::formatToString(StFormat_FrameSequence) + "\n"
        + "  --left=PATH          Specify source for left view\n"
          "  --right=PATH         Specify source for right view\n"
          "  --webuiCmdPort=PORT  Use http://localhost:PORT for remote control (see --invokeAction).\n"
          "  --invokeAction=ACT   Invoke action on http://localhost:PORT.\n"
          "                       play - play/pause\n"
          "                       mute - mute/unmute audio\n"
          "                       vol?VOLUME - specify volume in percents\n"
          "                       prev,next - play previous/next item in playlist\n"
          "                       seek?SECONDS - specify volume in percents\n"
          "                       fastbwd,fastfwd - seek backward/forward\n"
          "                       quit - close the program\n"
          "                       current?title - print title of currently played item\n"
          "                       fullscreen - switch fullscreen/windowed.\n"
          "                       The full list also includes actions having hot-keys";
    return anAboutString;
}

StHandle<StApplication> StMultiApp::getInstance(const StHandle<StResourceManager>& theResMgr,
                                                const StHandle<StOpenInfo>&        theInfo) {
    StHandle<StOpenInfo> anInfo = theInfo;
    if(anInfo.isNull()
    || (!anInfo->hasPath() && !anInfo->hasArgs())) {
        anInfo = StApplication::parseProcessArguments();
    }
    if(anInfo.isNull()) {
        // show help
        StString aShowHelpString = getAbout();
        st::cout << aShowHelpString;
        stInfo(aShowHelpString);
        return NULL;
    }

    // command interface
    const StArgumentsMap anArgs = anInfo->getArgumentsMap();
    StArgument anArgCmd  = anArgs["invokeAction"];
    StArgument anArgCmd2 = anArgs["action"];
    StString anAction;
    if(anArgCmd2.isValid()) {
        anAction = anArgCmd2.getValue();
    }
    if(anArgCmd.isValid()) {
        anAction = anArgCmd.getValue();
    }
    if(!anAction.isEmpty()) {
        StString anActLow = anAction;
        anActLow.toLowerCase();
        if(anActLow == "play"
        || anActLow == "pause") {
            anAction = "play_pause";
        } else if(anActLow == "stop") {
            anAction = "stop";
        } else if(anActLow == "mute") {
            anAction = "mute";
        } else if(anActLow == "next"
               || anActLow == "nextitem") {
            anAction = "next";
        } else if(anActLow == "prev"
               || anActLow == "previous"
               || anActLow == "previtem") {
            anAction = "prev";
        } else if(anActLow == "currentid") {
            anAction = "current?id";
        } else if(anActLow == "currenttitle") {
            anAction = "current?title";
        } else if(anAction.isContains('?')) {
            //anAction = anAction;
        } else if(anActLow == "fastbwd") {
            anAction = "action?DoSeekLeft";
        } else if(anActLow == "fastfwd") {
            anAction = "action?DoSeekRight";
        } else if(anActLow == "quit"
               || anActLow == "exit") {
            anAction = "action?DoQuit";
        } else if(anActLow == "mono") {
            anAction = "action?DoSrcMono";
        } else if(anActLow == "overunder") {
            anAction = "action?DoSrcOverUnder";
        } else if(anActLow == "sidebyside") {
            anAction = "action?DoSrcSideBySide";
        } else if(anActLow == "fullscreen") {
            anAction = "action?DoFullscreen";
        } else {
            anAction = StString("action?") + anAction;
        }
    }
    if(!anAction.isEmpty()) {
        StArgument anArgCmdPort = anArgs["webuiCmdPort"];
        StString   aPort        = "8080";
        if(anArgCmdPort.isValid()) {
            aPort = anArgCmdPort.getValue();
        }
        StString anUrl     = StString("http://localhost:") + aPort + "/" + anAction;
        StString aResponse = StRawFile::readTextFile(anUrl);
        st::cout << aResponse << "\n";
        return NULL;
    }

    // select application
    const StString ARGUMENT_DRAWER = "in";
    StArgument anArgDrawer = anArgs[ARGUMENT_DRAWER];
    if(!anInfo->hasPath() && !anArgDrawer.isValid()) {
        StApplication::readDefaultDrawer(anInfo);
        anArgDrawer = anInfo->getArgumentsMap()[ARGUMENT_DRAWER];
    }
    if(anArgDrawer.isValid()) {
        if(anArgDrawer.getValue() == "image"
        || anArgDrawer.getValue() == "StImageViewer") {
            return new StImageViewer(theResMgr, NULL, anInfo);
        } else if(anArgDrawer.getValue() == "video"
               || anArgDrawer.getValue() == "StMoviePlayer") {
            return new StMoviePlayer(theResMgr, NULL, anInfo);
        } else if(anArgDrawer.getValue() == "diag"
               || anArgDrawer.getValue() == "StDiagnostics") {
            return new StDiagnostics(theResMgr, NULL, anInfo);
        } else {
            // show help
            StString aShowHelpString = getAbout();
            st::cout << aShowHelpString;
            stInfo(aShowHelpString);
            return NULL;
        }
    }

    const StString aPath          = anInfo->getPath();
    const StString aFileExtension = StFileNode::getExtension(aPath);

    const StMIMEList aMimeImg(ST_IMAGE_PLUGIN_MIME_CHAR);
    for(size_t aMimeIter = 0; aMimeIter < aMimeImg.size(); ++aMimeIter) {
        if(aFileExtension.isEqualsIgnoreCase(aMimeImg[aMimeIter].getExtension())) {
            return new StImageViewer(theResMgr, NULL, anInfo);
        }
    }

    return new StMoviePlayer(theResMgr, NULL, anInfo);
}

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
        + "Copyright (C) 2007-2015 Kirill Gavrilov (kirill@sview.ru).\n"
        + "Usage: sView [options] - file\n"
        + "Available options:\n"
          "  --fullscreen         Open fullscreen\n"
          "  --slideshow          Start slideshow\n"
          "  --last               Open last file\n"
          "  --paused             Open file in paused state\n"
          "  --in=image,video     Application to open (predefined values: image, video, diag, cad)\n"
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
          "  --right=PATH         Specify source for right view\n";
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

    const StArgumentsMap anArgs = anInfo->getArgumentsMap();
    const StString ARGUMENT_DRAWER = "in";
    StArgument anArgDrawer = anArgs[ARGUMENT_DRAWER];
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

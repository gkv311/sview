/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2013
 */

#include "StMultiApp.h"

#include "../StImageViewer/StImageViewer.h"
#include "../StImageViewer/StImagePluginInfo.h"
#include "../StMoviePlayer/StMoviePlayer.h"
#include "../StDiagnostics/StDiagnostics.h"
#include "../StCADViewer/StCADViewer.h"
#include "../StCADViewer/StCADPluginInfo.h"

#include <StStrings/stConsole.h>
#include <StVersion.h>

static StString getAbout() {
    StString anAboutString =
        StString("sView ") + StVersionInfo::getSDKVersionString() + '\n'
        + "Copyright (C) 2007-2013 Kirill Gavrilov (kirill@sview.ru).\n"
        + "Usage: sView [options] - file\n"
        + "Available options:\n"
          "  --fullscreen         Open fullscreen\n"
          "  --slideshow          Start slideshow\n"
          "  --last               Open last file\n"
          "  --in=image,video,cad Application to open (predefined values: image, video, diag, cad)\n"
          "  --out=RENDERER_PATH  Stereoscopic output module (auto, StOutAnaglyph, StOutDual,...)\n"
          "  --imageLib=IMGLIB    Setup 3rd-party library for image processing (FFmpeg, FreeImage, DevIL)\n"
          "  --viewMode=MODE      View mode (flat, sphere)\n"
          "  --srcFormat=FORMAT   Setup source format:\n"
          "                       "
            + st::formatToString(ST_V_SRC_AUTODETECT)    + ", "
            + st::formatToString(ST_V_SRC_MONO)          + ", "
            + st::formatToString(ST_V_SRC_SIDE_BY_SIDE)  + ", "
            + st::formatToString(ST_V_SRC_PARALLEL_PAIR) + ",\n"
        + "                       "
            + st::formatToString(ST_V_SRC_OVER_UNDER_LR) + ", "
            + st::formatToString(ST_V_SRC_OVER_UNDER_RL) + ", "
            + st::formatToString(ST_V_SRC_ROW_INTERLACE) + ", "
            + st::formatToString(ST_V_SRC_PAGE_FLIP)     + '\n';
    return anAboutString;
}

StHandle<StApplication> StMultiApp::getInstance(const StHandle<StOpenInfo>& theInfo) {
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
            return new StImageViewer(NULL, anInfo);
        } else if(anArgDrawer.getValue() == "video"
               || anArgDrawer.getValue() == "StMoviePlayer") {
            return new StMoviePlayer(NULL, anInfo);
        } else if(anArgDrawer.getValue() == "diag"
               || anArgDrawer.getValue() == "StDiagnostics") {
            return new StDiagnostics(NULL, anInfo);
        } else if(anArgDrawer.getValue() == "cad"
               || anArgDrawer.getValue() == "StCADViewer") {
            return new StCADViewer(NULL, anInfo);
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
            return new StImageViewer(NULL, anInfo);
        }
    }
    const StMIMEList aMimeCad(ST_CAD_PLUGIN_MIME_CHAR);
    for(size_t aMimeIter = 0; aMimeIter < aMimeCad.size(); ++aMimeIter) {
        if(aFileExtension.isEqualsIgnoreCase(aMimeCad[aMimeIter].getExtension())) {
            return new StCADViewer(NULL, anInfo);
        }
    }

    return new StMoviePlayer(NULL, anInfo);
}

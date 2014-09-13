/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2014
 */

#if defined(__ANDROID__)

#include <jni.h>

#include <StCore/StAndroidGlue.h>

#include "../../StImageViewer/StImageViewer.h"
#include "../StImageViewer/StImagePluginInfo.h"
#include "../../StMoviePlayer/StMoviePlayer.h"

/**
 * Entry point for dedicated thread which will execute main application code
 * (e.g. OpenGL rendering and emulated event-handling loop).
 */
static void app_entry_my(StAndroidGlue* theApp) {
    //const StString aProcessPath = StProcess::getProcessFolder();
    //StProcess::setEnv(ST_ENV_NAME_STCORE_PATH, aProcessPath);
    //StProcess::setEnv("StShare",               aProcessPath);

    StHandle<StOpenInfo> anInfo = new StOpenInfo();
    const StString aPath          = theApp->getDataPath();
    const StString aFileExtension = StFileNode::getExtension(aPath);
    anInfo->setPath(theApp->getDataPath());
    //anInfo = StApplication::parseProcessArguments();

    StHandle<StApplication> anApp;
    const StMIMEList aMimeImg(ST_IMAGE_PLUGIN_MIME_CHAR);
    for(size_t aMimeIter = 0; aMimeIter < aMimeImg.size(); ++aMimeIter) {
        if(aFileExtension.isEqualsIgnoreCase(aMimeImg[aMimeIter].getExtension())) {
            anApp = new StImageViewer(theApp, anInfo);
            break;
        }
    }
    if(anApp.isNull()) {
        anApp = new StMoviePlayer(theApp, anInfo);
    }
    if(anApp.isNull() || !anApp->open()) {
        return;
    }

    anApp->exec();
}

/**
 * Main entry point - called from Java on creation.
 * This function defines JNI callbacks and creates dedicated thread for main application code.
 */
ST_CEXPORT void ANativeActivity_onCreate(ANativeActivity* theActivity,
                                         void*            theSavedState,
                                         size_t           theSavedStateSize) {
    StAndroidGlue* anApp = new StAndroidGlue(theActivity, theSavedState, theSavedStateSize);
    anApp->onAppEntry = app_entry_my;
    anApp->start();
}

#endif // __ANDROID__

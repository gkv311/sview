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
 * Player glue.
 */
class StMainGlue : public StAndroidGlue {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StMainGlue(ANativeActivity* theActivity,
                        void*            theSavedState,
                        size_t           theSavedStateSize)
    : StAndroidGlue(theActivity, theSavedState, theSavedStateSize) {
        //
    }

    /**
     * Choose and instantiate StApplication.
     */
    ST_LOCAL virtual void createApplication() override {
        StMutexAuto aLock(myDndLock);
        const StString aFileExtension = StFileNode::getExtension(myDndPath);

        StHandle<StOpenInfo> anInfo = new StOpenInfo();
        anInfo->setPath(myDndPath);
        myDndPath.clear();

        StHandle<StResourceManager> aResMgr = new StResourceManager(myActivity->assetManager);

        const StMIMEList aMimeImg(ST_IMAGE_PLUGIN_MIME_CHAR);
        for(size_t aMimeIter = 0; aMimeIter < aMimeImg.size(); ++aMimeIter) {
            if(aFileExtension.isEqualsIgnoreCase(aMimeImg[aMimeIter].getExtension())) {
                myApp = new StImageViewer(aResMgr, this, anInfo);
                return;
            }
        }

        myApp = new StMoviePlayer(aResMgr, this, anInfo);
    }

};

/**
 * Main entry point - called from Java on creation.
 * This function defines JNI callbacks and creates dedicated thread for main application code.
 */
ST_CEXPORT void ANativeActivity_onCreate(ANativeActivity* theActivity,
                                         void*            theSavedState,
                                         size_t           theSavedStateSize) {
    StMainGlue* anApp = new StMainGlue(theActivity, theSavedState, theSavedStateSize);
    anApp->start();
}

#endif // __ANDROID__

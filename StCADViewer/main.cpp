/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

namespace
{
  /**
   * The list of OCCT environment variable to be reset.
   */
  const char* THE_OCCT_ENV_DEF[] =
  {
    "CSF_ShadersDirectory",
    "CSF_SHMessage",
    "CSF_XSMessage",
    "CSF_StandardDefaults",
    "CSF_PluginDefaults",
    "CSF_XCAFDefaults",
    "CSF_TObjDefaults",
    "CSF_StandardLiteDefaults",
    "CSF_IGESDefaults",
    "CSF_STEPDefaults",
    "CSF_XmlOcafResource",
    "CSF_MIGRATION_TYPES",
    0
  };
}

#if defined(__APPLE__)
    //
#elif defined(__ANDROID__)

#include <jni.h>
#include <StCore/StAndroidGlue.h>
#include <StFile/StRawFile.h>
#include "../StCADViewer/StCADViewer.h"

/**
 * Viewer glue.
 */
class StCADViewerGlue : public StAndroidGlue {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StCADViewerGlue(ANativeActivity* theActivity,
                             void*            theSavedState,
                             size_t           theSavedStateSize)
    : StAndroidGlue(theActivity, theSavedState, theSavedStateSize) {}

    /**
     * Instantiate StApplication.
     */
    ST_LOCAL virtual void createApplication() override {
        StMutexAuto aLock(myFetchLock);
        const StString aFileExtension = StFileNode::getExtension(myCreatePath);

        StHandle<StOpenInfo> anInfo = new StOpenInfo();
        anInfo->setPath(myDndPath);
        myDndPath.clear();

        StHandle<StResourceManager> aResMgr = new StResourceManager(myActivity->assetManager);
        //StHandle<StResourceManager> aResMgr = new StResourceManager(myActivity->assetManager"com.sview.cad");
        aResMgr->setFolder(StResourceManager::FolderId_SdCard,
                           getStoragePath(myThJniEnv, "sdcard"));
        aResMgr->setFolder(StResourceManager::FolderId_Downloads,
                           getStoragePath(myThJniEnv, "Download"));
        aResMgr->setFolder(StResourceManager::FolderId_Pictures,
                           getStoragePath(myThJniEnv, "Pictures"));
        aResMgr->setFolder(StResourceManager::FolderId_Photos,
                           getStoragePath(myThJniEnv, "DCIM"));

        // force using embedded OCCT resources
        for(const char** anEnvIter = THE_OCCT_ENV_DEF; *anEnvIter != NULL; ++anEnvIter) {
            StProcess::setEnv(*anEnvIter, "");
        }

        if(myStAppClass.isEmpty()) {
            myStAppClass = "cad";
        }

        if(anInfo->isEmpty()) {
            // open recent file by default
            StArgumentsMap anArgs = anInfo->getArgumentsMap();
            anArgs.set(StDictEntry("last", "true"));
            anArgs.set(StDictEntry("toSaveRecent","true"));
            anInfo->setArgumentsMap(anArgs);
        }

        myApp = new StCADViewer(aResMgr, this, anInfo);
    }

};

/**
 * Main entry point - called from Java on creation.
 * This function defines JNI callbacks and creates dedicated thread for main application code.
 */
ST_CEXPORT void ANativeActivity_onCreate(ANativeActivity* theActivity,
                                         void*            theSavedState,
                                         size_t           theSavedStateSize) {
    StCADViewerGlue* anApp = new StCADViewerGlue(theActivity, theSavedState, theSavedStateSize);
    anApp->start();
}

#else

#include <StVersion.h>
#include <StFile/StFolder.h>
#include <StStrings/stConsole.h>

#include "../StOutPageFlip/StOutPageFlip.h"
#include "../StCADViewer/StCADViewer.h"

static StString getAbout() {
    StString anAboutString =
        StString("StCADViewer ") + StVersionInfo::getSDKVersionString() + '\n'
        + "Copyright (C) 2007-2016 Kirill Gavrilov (kirill@sview.ru).\n"
        + "Usage: StCADViewer [options] - file\n";
    return anAboutString;
}

#ifdef _WIN32
#ifdef ST_DEBUG
int main(int , char** ) { // force console output
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { // prevent console output
#endif
    setlocale(LC_ALL, ".OCP"); // we set default locale for console output (useful only for debug)
#else
int main(int , char** ) {
#endif
    StOutPageFlip::initGlobalsAsync();
    if(!StVersionInfo::checkTimeBomb("sView")) {
        return 1;
    }

    // setup environment variables
    const StString ST_ENV_NAME_STCORE_PATH =
    #if defined(_WIN64) || defined(_LP64) || defined(__LP64__)
        "StCore64";
    #else
        "StCore32";
    #endif
    const StString aProcessPath = StProcess::getProcessFolder();
    StString aProcessUpPath = StFileNode::getFolderUp(aProcessPath);
    if(!aProcessUpPath.isEmpty()) {
        aProcessUpPath += SYS_FS_SPLITTER;
    }
    StProcess::setEnv(ST_ENV_NAME_STCORE_PATH, aProcessPath);
    if(StFolder::isFolder(aProcessPath + "textures")) {
        StProcess::setEnv("StShare", aProcessPath);
    } else if(StFolder::isFolder(aProcessUpPath + "textures")) {
        StProcess::setEnv("StShare", aProcessUpPath);
    }

    // force using embedded OCCT resources
    for(const char** anEnvIter = THE_OCCT_ENV_DEF; *anEnvIter != NULL; ++anEnvIter) {
        StProcess::setEnv(*anEnvIter, "");
    }

    StHandle<StOpenInfo> anInfo;
    if(anInfo.isNull()
    || (!anInfo->hasPath() && !anInfo->hasArgs())) {
        anInfo = StApplication::parseProcessArguments();
    }
    if(anInfo.isNull()) {
        // show help
        StString aShowHelpString = getAbout();
        st::cout << aShowHelpString;
        stInfo(aShowHelpString);
        return 0;
    }

    StHandle<StResourceManager> aResMgr = new StResourceManager();
    StHandle<StCADViewer> anApp  = new StCADViewer(aResMgr, NULL, anInfo);
    if(!anApp->open()) {
        return 1;
    }
    return anApp->exec();
}

#endif // __APPLE__

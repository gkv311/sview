/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2007-2025
 */

#ifndef __APPLE__

#include "StMultiApp.h"

#include <StVersion.h>
#include <StFile/StFolder.h>

#include "../StOutPageFlip/StOutPageFlip.h"

int sView_main() {
#ifdef _MSC_VER
    // turn ON thread-safe locale management (MSVCRT-specific)
    //_configthreadlocale(-1); // conflicts with C++ locales...
#endif
#ifdef _WIN32
    setlocale(LC_ALL, ".OCP"); // we set default locale for console output (useful only for debug)
#endif

    StProcess::setupProcessSignals();

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

    std::shared_ptr<StResourceManager> aResMgr = std::make_shared<StResourceManager>();
    std::shared_ptr<StApplication>     anApp   = StMultiApp::getInstance(aResMgr);
    for(;;) {
        if (anApp.get() == nullptr || !anApp->open()) {
            return 1;
        }

        int aResult = anApp->exec();
        std::shared_ptr<StOpenInfo> anOther = anApp->getOpenFileInOtherDrawer();
        if (anOther.get() == nullptr) {
            return aResult;
        }

        anApp.reset();
        anApp = StMultiApp::getInstance(aResMgr, anOther);
    }
}

int main(int theNbArgs, char** theArgVec) {
    (void)theNbArgs; (void)theArgVec;
    return sView_main();
}

// GUI app without console output
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return sView_main();
}
#endif

#endif // __APPLE__

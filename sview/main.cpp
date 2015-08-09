/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2007-2015
 */

#ifndef __APPLE__

#include "StMultiApp.h"
#include <StVersion.h>
#include <StFile/StFolder.h>
#include "../StOutPageFlip/StOutPageFlip.h"

#ifdef _WIN32
#ifdef ST_DEBUG
int main(int , char** ) { // force console output
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { // prevent console output
#endif

#ifdef _MSC_VER
    // turn ON thread-safe locale management (MSVCRT-specific)
    //_configthreadlocale(-1); // conflicts with C++ locales...
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

    StHandle<StResourceManager> aResMgr = new StResourceManager();
    StHandle<StApplication>     anApp   = StMultiApp::getInstance(aResMgr);
    if(anApp.isNull() || !anApp->open()) {
        return 1;
    }
    return anApp->exec();
}

#endif // __APPLE__

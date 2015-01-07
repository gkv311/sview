/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2007-2015
 */

#ifndef __APPLE__

#include "StMultiApp.h"
#include <StVersion.h>
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

#ifdef ST_DEBUG
    // override environment variable if sView is installed in the system
    #if defined(_WIN64) || defined(_LP64) || defined(__LP64__)
        const StString ST_ENV_NAME_STCORE_PATH = "StCore64";
    #else
        const StString ST_ENV_NAME_STCORE_PATH = "StCore32";
    #endif
    const StString aProcessPath = StProcess::getProcessFolder();
    StProcess::setEnv(ST_ENV_NAME_STCORE_PATH, aProcessPath);
    StProcess::setEnv("StShare",               aProcessPath);
#endif

    StHandle<StResourceManager> aResMgr = new StResourceManager();
    StHandle<StApplication>     anApp   = StMultiApp::getInstance(aResMgr);
    if(anApp.isNull() || !anApp->open()) {
        return 1;
    }
    return anApp->exec();
}

#endif // __APPLE__

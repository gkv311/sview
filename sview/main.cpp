/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2007-2013
 */

#ifndef __APPLE__

#include "StMultiApp.h"
#include <StVersion.h>

#ifdef _WIN32
#ifdef __ST_DEBUG__
int main(int , char** ) { // force console output
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { // prevent console output
#endif

#ifdef _MSC_VER
    // turn ON thread-safe locale management (MSVCRT-specific)
    _configthreadlocale(-1);
#endif
    setlocale(LC_ALL, ".OCP"); // we set default locale for console output (useful only for debug)
#else
int main(int , char** ) {
#endif
    if(!StVersionInfo::checkTimeBomb("sView")) {
        return 1;
    }

#ifdef __ST_DEBUG__
    // TODO (Kirill Gavrilov#9) debug environment
    #if (defined(_WIN64) || defined(__WIN64__))\
     || (defined(_LP64)  || defined(__LP64__))
        const StString ST_ENV_NAME_STCORE_PATH = "StCore64";
    #else
        const StString ST_ENV_NAME_STCORE_PATH = "StCore32";
    #endif
    StProcess::setEnv(ST_ENV_NAME_STCORE_PATH, StProcess::getProcessFolder());
#endif

    StHandle<StApplication> anApp = StMultiApp::getInstance();
    if(anApp.isNull() || !anApp->open()) {
        return 1;
    }
    return anApp->exec();
}

#endif // __APPLE__

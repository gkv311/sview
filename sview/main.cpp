/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2007-2012
 */

#ifndef __APPLE__

#include <StCore/StApplication.h>
#include <StCore/StCore.h>

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("sView");
};

#if(defined(_WIN32) || defined(__WIN32__))
#ifdef __ST_DEBUG__
int main(int , char** ) { // force console output
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { // prevent console output
#endif
    setlocale(LC_ALL, ".OCP"); // we set default locale for console output (useful only for debug)
#else
int main(int , char** ) {
#endif

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

    if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError("StCore Library initialization FAILED!");
        return 1;
    }
    ST_DEBUG_LOG("StCore Library init success!");

    StHandle<StApplication> anApp = new StApplication();
    if(!anApp->create()) {
        anApp.nullify();
        StCore::FREE();
        return 2;
    }
    if(!anApp->open()) {
        anApp.nullify();
        StCore::FREE();
        return 3;
    }

    for(;;) {
        if(!anApp->isOpened()) {
            anApp.nullify();
            StCore::FREE();
            return 0;
        }
        anApp->callback();
    }
}

#endif // __APPLE__

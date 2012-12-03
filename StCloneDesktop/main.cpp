/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#include "StCloneDesktop.h"

#if(defined(_WIN32) || defined(__WIN32__))
int main(int , char** ) {      // force console output
    setlocale(LC_ALL, ".OCP"); // we set default locale for console output (useful only for debug)
#else
int main(int argc, stUtf_t** argv) {
#endif

    StString welcomeMessage = ST_STRING("StCloneDesktop ") + StVersionInfo::getSDKVersionString()
                            + ST_STRING(" by Kirill Gavrilov (kirill@sview.ru)\n\n");
    st::cout << st::COLOR_FOR_GREEN << welcomeMessage << st::COLOR_FOR_WHITE;

    StCloneDesktop aCloneDesk;
    if(!aCloneDesk.create()) {
        return 1;
    }
    aCloneDesk.mainLoop();

    st::cout << ST_TEXT("Press any key to exit...") << st::SYS_PAUSE_EMPTY;
    return 0;
}

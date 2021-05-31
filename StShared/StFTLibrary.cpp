/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StFT/StFTLibrary.h>

StFTLibrary::StFTLibrary()
: myFTLib(NULL) {
    if(FT_Init_FreeType(&myFTLib) != 0) {
        myFTLib = NULL;
    }
}

StFTLibrary::~StFTLibrary() {
    if(isValid()) {
        FT_Done_FreeType(myFTLib);
    }
}

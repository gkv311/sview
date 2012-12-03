/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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

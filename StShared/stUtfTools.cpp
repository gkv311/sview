/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StStrings/StString.h>
#include <StStrings/stUtfTools.h>

namespace {
    static const stUtf32_t ST_NUMBERS_ARRAY[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    bool isNumChar(const stUtf32_t theChar) {
        for(size_t aNumId = 0; aNumId < 10; ++aNumId) {
            if(theChar == ST_NUMBERS_ARRAY[aNumId]) {
                return true;
            }
        }
        return false;
    }
};

bool stUtfTools::isInteger(const StString& theString) {
    StUtf8Iter anIter = theString.iterator();
    // TODO (Kirill Gavrilov#9) + and - should be followed by numbers!
    if(!isNumChar(*anIter) && (*anIter != stUtf32_t('+')) && (*anIter != stUtf32_t('-'))) {
        return false;
    }
    ++anIter;
    for(; *anIter != 0; ++anIter) {
        if(!isNumChar(*anIter)) {
            return false;
        }
    }
    return true;
}

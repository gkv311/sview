/**
 * For more information and updates: http://www.firstpr.com.au/dsp/rand31/
 * Copyright © 2005 Robin Whittle <rw@firstpr.com.au>
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StThreads/StMinGen.h>

#include <cmath>

namespace {
    // Multiplier constant = 16807 = 7^5.
    // This is 15 bits.
    static const uint32_t CONSTI_A = 16807;
};

StMinGen::StMinGen()
: mySeed31(1) {
    //
}

uint32_t StMinGen::nextInt() {
    uint32_t lo = CONSTI_A * (mySeed31 & 0xFFFF);
    uint32_t hi = CONSTI_A * (mySeed31 >> 16);
    lo += (hi & 0x7FFF) << 16;
    lo += hi >> 15;
    if(lo > 0x7FFFFFFF) {
        lo -= 0x7FFFFFFF;
    }
    mySeed31 = lo;
    return mySeed31;
}

double StMinGen::next() {
    return double(nextInt()) / 2147483647.0;
}

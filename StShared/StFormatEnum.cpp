/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLStereo/StFormatEnum.h>

namespace {
    static const char ST_V_SRC_AUTODETECT_STRING[]    = "auto";
    static const char ST_V_SRC_SIDE_BY_SIDE_STRING[]  = "crossEyed";
    static const char ST_V_SRC_PARALLEL_PAIR_STRING[] = "parallelPair";
    static const char ST_V_SRC_OVER_UNDER_RL_STRING[] = "overUnderRL";
    static const char ST_V_SRC_OVER_UNDER_LR_STRING[] = "overUnderLR";
    static const char ST_V_SRC_ROW_INTERLACE_STRING[] = "interlaceRow";
    static const char ST_V_SRC_PAGE_FLIP_STRING[]     = "frameSequential";
    static const char ST_V_SRC_MONO_STRING[]          = "mono";
};

StString st::formatToString(StFormatEnum theFormatEnum) {
    switch(theFormatEnum) {
        case ST_V_SRC_MONO:          return ST_V_SRC_MONO_STRING;
        case ST_V_SRC_SIDE_BY_SIDE:  return ST_V_SRC_SIDE_BY_SIDE_STRING;
        case ST_V_SRC_PARALLEL_PAIR: return ST_V_SRC_PARALLEL_PAIR_STRING;
        case ST_V_SRC_OVER_UNDER_RL: return ST_V_SRC_OVER_UNDER_RL_STRING;
        case ST_V_SRC_OVER_UNDER_LR: return ST_V_SRC_OVER_UNDER_LR_STRING;
        case ST_V_SRC_ROW_INTERLACE: return ST_V_SRC_ROW_INTERLACE_STRING;
        case ST_V_SRC_PAGE_FLIP:     return ST_V_SRC_PAGE_FLIP_STRING;
        case ST_V_SRC_AUTODETECT:
        default:                     return ST_V_SRC_AUTODETECT_STRING;
    }
}

StFormatEnum st::formatFromString(const StString& theFormatString) {
    if(theFormatString.isEqualsIgnoreCase(ST_V_SRC_MONO_STRING)) {
        return ST_V_SRC_MONO;
    } else if(theFormatString.isEqualsIgnoreCase(ST_V_SRC_SIDE_BY_SIDE_STRING)) {
        return ST_V_SRC_SIDE_BY_SIDE;
    } else if(theFormatString.isEqualsIgnoreCase(ST_V_SRC_PARALLEL_PAIR_STRING)) {
        return ST_V_SRC_PARALLEL_PAIR;
    } else if(theFormatString.isEqualsIgnoreCase(ST_V_SRC_OVER_UNDER_RL_STRING)) {
        return ST_V_SRC_OVER_UNDER_RL;
    } else if(theFormatString.isEqualsIgnoreCase(ST_V_SRC_OVER_UNDER_LR_STRING)) {
        return ST_V_SRC_OVER_UNDER_LR;
    } else if(theFormatString.isEqualsIgnoreCase(ST_V_SRC_ROW_INTERLACE_STRING)) {
        return ST_V_SRC_ROW_INTERLACE;
    } else if(theFormatString.isEqualsIgnoreCase(ST_V_SRC_PAGE_FLIP_STRING)) {
        return ST_V_SRC_PAGE_FLIP;
    } else {
        return ST_V_SRC_AUTODETECT;
    }
}

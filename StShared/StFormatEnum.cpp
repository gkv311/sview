/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLStereo/StFormatEnum.h>

#include <StFile/StFileNode.h>

namespace {
    static const StCString ST_V_SRC_AUTODETECT_STRING    = stCString("auto");
    static const StCString ST_V_SRC_SIDE_BY_SIDE_STRING  = stCString("crossEyed");
    static const StCString ST_V_SRC_PARALLEL_PAIR_STRING = stCString("parallelPair");
    static const StCString ST_V_SRC_OVER_UNDER_RL_STRING = stCString("overUnderRL");
    static const StCString ST_V_SRC_OVER_UNDER_LR_STRING = stCString("overUnderLR");
    static const StCString ST_V_SRC_ROW_INTERLACE_STRING = stCString("interlaceRow");
    static const StCString ST_V_SRC_PAGE_FLIP_STRING     = stCString("frameSequential");
    static const StCString ST_V_SRC_MONO_STRING          = stCString("mono");
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

StFormatEnum st::formatReversed(const StFormatEnum theFormatEnum) {
    switch(theFormatEnum) {
        case ST_V_SRC_SIDE_BY_SIDE:  return ST_V_SRC_PARALLEL_PAIR;
        case ST_V_SRC_PARALLEL_PAIR: return ST_V_SRC_SIDE_BY_SIDE;
        case ST_V_SRC_OVER_UNDER_RL: return ST_V_SRC_OVER_UNDER_LR;
        case ST_V_SRC_OVER_UNDER_LR: return ST_V_SRC_OVER_UNDER_RL;
        case ST_V_SRC_ROW_INTERLACE:
        case ST_V_SRC_PAGE_FLIP:
        case ST_V_SRC_MONO:
        case ST_V_SRC_AUTODETECT:
        default:                     return theFormatEnum;
    }
}

StFormatEnum st::formatFromName(const StString& theFileName,
                                bool&           theIsAnamorph) {
    StString aName, anExt;
    StFileNode::getNameAndExtension(theFileName, aName, anExt);
    aName.toLowerCase();
    anExt.toLowerCase();
    if(anExt == stCString("pns")
    || anExt == stCString("jps")) {
        theIsAnamorph = false;
        return ST_V_SRC_SIDE_BY_SIDE;
    }

    // this is not optimized search, but should be OK for most use cases
    if(aName.isContains(stCString("halfou"))
    || aName.isContains(stCString("half-ou"))
    || aName.isContains(stCString("half_ou"))
    || aName.isContains(stCString("half.ou"))
    || aName.isContains(stCString("half ou"))
    || aName.isEndsWith(stCString("-hou"))
    || aName.isEndsWith(stCString("_hou"))
    || aName.isEndsWith(stCString(".hou"))
    || aName.isEndsWith(stCString(" hou"))
    || aName.isEndsWith(stCString("-abq"))) {
        theIsAnamorph = true;
        return ST_V_SRC_OVER_UNDER_LR;
    } else if(aName.isEndsWith(stCString("-baq"))) {
        theIsAnamorph = true;
        return ST_V_SRC_OVER_UNDER_RL;
    } else if(aName.isEndsWith(stCString("-ab"))
           //|| aName.isContains(stCString("-ou")) // too ambiguous
           //|| aName.isContains(stCString("_ou"))
           //|| aName.isContains(stCString(".ou"))
           //|| aName.isContains(stCString(" ou"))
             ) {
        theIsAnamorph = false;
        return ST_V_SRC_OVER_UNDER_LR;
    } else if(aName.isEndsWith(stCString("-ba"))) {
        theIsAnamorph = false;
        return ST_V_SRC_OVER_UNDER_RL;
    } else if(aName.isContains(stCString("halfsbs"))
           || aName.isContains(stCString("half-sbs"))
           || aName.isContains(stCString("half_sbs"))
           || aName.isContains(stCString("half.sbs"))
           || aName.isContains(stCString("half sbs"))
           || aName.isContains(stCString(".hsbs"))
           || aName.isContains(stCString("-hsbs"))
           || aName.isContains(stCString("_hsbs"))
           || aName.isContains(stCString(" hsbs"))
           || aName.isEndsWith(stCString("-lrq"))) {
        theIsAnamorph = true;
        return ST_V_SRC_PARALLEL_PAIR;
    } else if(aName.isEndsWith(stCString("-rlq"))) {
        theIsAnamorph = true;
        return ST_V_SRC_SIDE_BY_SIDE;
    } else if(aName.isContains(stCString("-sbs"))
           || aName.isContains(stCString(".sbs"))
           || aName.isContains(stCString(" sbs"))
           || aName.isEndsWith(stCString("-lr"))) {
        theIsAnamorph = false;
        return ST_V_SRC_PARALLEL_PAIR;
    } else if(aName.isEndsWith(stCString("-rl"))) {
        theIsAnamorph = false;
        return ST_V_SRC_SIDE_BY_SIDE;
    } else if(aName.isEndsWith(stCString("-2d"))) {
        theIsAnamorph = false;
        return ST_V_SRC_MONO;
    }
    theIsAnamorph = false;
    return ST_V_SRC_AUTODETECT;
}

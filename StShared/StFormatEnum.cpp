/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLStereo/StFormatEnum.h>

#include <StFile/StFileNode.h>

namespace {
    static const StCString StFormat_AUTO_STRING          = stCString("auto");
    static const StCString StFormat_SideBySide_RL_STRING = stCString("crossEyed");
    static const StCString StFormat_SideBySide_LR_STRING = stCString("parallelPair");
    static const StCString StFormat_TopBottom_RL_STRING  = stCString("overUnderRL");
    static const StCString StFormat_TopBottom_LR_STRING  = stCString("overUnderLR");
    static const StCString StFormat_Rows_STRING          = stCString("interlaceRow");
    static const StCString StFormat_FrameSequence_STRING = stCString("frameSequential");
    static const StCString StFormat_Mono_STRING          = stCString("mono");
};

StString st::formatToString(StFormat theFormatEnum) {
    switch(theFormatEnum) {
        case StFormat_Mono:          return StFormat_Mono_STRING;
        case StFormat_SideBySide_LR: return StFormat_SideBySide_LR_STRING;
        case StFormat_SideBySide_RL: return StFormat_SideBySide_RL_STRING;
        case StFormat_TopBottom_LR:  return StFormat_TopBottom_LR_STRING;
        case StFormat_TopBottom_RL:  return StFormat_TopBottom_RL_STRING;
        case StFormat_Rows:          return StFormat_Rows_STRING;
        case StFormat_FrameSequence: return StFormat_FrameSequence_STRING;
        case StFormat_AUTO:
        default:                     return StFormat_AUTO_STRING;
    }
}

StFormat st::formatFromString(const StString& theFormatString) {
    if(theFormatString.isEqualsIgnoreCase(StFormat_Mono_STRING)) {
        return StFormat_Mono;
    } else if(theFormatString.isEqualsIgnoreCase(StFormat_SideBySide_RL_STRING)) {
        return StFormat_SideBySide_RL;
    } else if(theFormatString.isEqualsIgnoreCase(StFormat_SideBySide_LR_STRING)) {
        return StFormat_SideBySide_LR;
    } else if(theFormatString.isEqualsIgnoreCase(StFormat_TopBottom_RL_STRING)) {
        return StFormat_TopBottom_RL;
    } else if(theFormatString.isEqualsIgnoreCase(StFormat_TopBottom_LR_STRING)) {
        return StFormat_TopBottom_LR;
    } else if(theFormatString.isEqualsIgnoreCase(StFormat_Rows_STRING)) {
        return StFormat_Rows;
    } else if(theFormatString.isEqualsIgnoreCase(StFormat_FrameSequence_STRING)) {
        return StFormat_FrameSequence;
    } else {
        return StFormat_AUTO;
    }
}

StFormat st::formatReversed(const StFormat theFormatEnum) {
    switch(theFormatEnum) {
        case StFormat_SideBySide_RL: return StFormat_SideBySide_LR;
        case StFormat_SideBySide_LR: return StFormat_SideBySide_RL;
        case StFormat_TopBottom_RL:  return StFormat_TopBottom_LR;
        case StFormat_TopBottom_LR:  return StFormat_TopBottom_RL;
        case StFormat_Rows:
        case StFormat_FrameSequence:
        case StFormat_Mono:
        case StFormat_AUTO:
        default:                     return theFormatEnum;
    }
}

StFormat st::formatFromName(const StString& theFileName,
                            const bool      theToSwapJps,
                            bool&           theIsAnamorph) {
    StString aName, anExt;
    StFileNode::getNameAndExtension(theFileName, aName, anExt);
    aName.toLowerCase();
    anExt.toLowerCase();
    if(anExt == stCString("pns")
    || anExt == stCString("jps")) {
        theIsAnamorph = false;
        return theToSwapJps ? StFormat_SideBySide_LR : StFormat_SideBySide_RL;
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
        return StFormat_TopBottom_LR;
    } else if(aName.isEndsWith(stCString("-baq"))) {
        theIsAnamorph = true;
        return StFormat_TopBottom_RL;
    } else if(aName.isEndsWith(stCString("-ab"))
           //|| aName.isContains(stCString("-ou")) // too ambiguous
           //|| aName.isContains(stCString("_ou"))
           //|| aName.isContains(stCString(".ou"))
           //|| aName.isContains(stCString(" ou"))
             ) {
        theIsAnamorph = false;
        return StFormat_TopBottom_LR;
    } else if(aName.isEndsWith(stCString("-ba"))) {
        theIsAnamorph = false;
        return StFormat_TopBottom_RL;
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
        return StFormat_SideBySide_LR;
    } else if(aName.isEndsWith(stCString("-rlq"))) {
        theIsAnamorph = true;
        return StFormat_SideBySide_RL;
    } else if(aName.isContains(stCString("-sbs"))
           || aName.isContains(stCString(".sbs"))
           || aName.isContains(stCString(" sbs"))
           || aName.isEndsWith(stCString("-lr"))) {
        theIsAnamorph = false;
        return StFormat_SideBySide_LR;
    } else if(aName.isEndsWith(stCString("-rl"))) {
        theIsAnamorph = false;
        return StFormat_SideBySide_RL;
    } else if(aName.isEndsWith(stCString("-2d"))) {
        theIsAnamorph = false;
        return StFormat_Mono;
    }
    theIsAnamorph = false;
    return StFormat_AUTO;
}

StFormat st::formatFromRatio(const GLfloat theRatio) {
    if(stAreEqual(theRatio, st::videoRatio::TV_SIDEBYSIDE,      0.18f)
    || stAreEqual(theRatio, st::videoRatio::WIDE_SIDEBYSIDE,    0.18f)
    || stAreEqual(theRatio, st::videoRatio::USERDEF_SIDEBYSIDE, 0.18f)) {
        return StFormat_SideBySide_RL;
    }
    return StFormat_Mono;
}

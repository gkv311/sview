/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StSettings.h>

bool StSettings::loadBool(const StString& theLabel,
                          bool&           theValue) {
    int32_t anIntValue = theValue ? 1 : 0;
    if(!loadInt32(theLabel, anIntValue)) {
        return false;
    }
    theValue = (anIntValue == 1);
    return true;
}

bool StSettings::saveBool(const StString& theLabel,
                          const bool      theValue) {
    int32_t anIntValue = theValue ? 1 : 0;
    if(!saveInt32(theLabel, anIntValue)) {
        return false;
    }
    return true;
}

bool StSettings::loadInt32Rect(const StString&  theLabel,
                               StRect<int32_t>& theValue) {
    int32_t l = 0, t = 0, r = 0, b = 0;
    if(!loadInt32(theLabel + ".left",   l)
    || !loadInt32(theLabel + ".right",  r)
    || !loadInt32(theLabel + ".top",    t)
    || !loadInt32(theLabel + ".bottom", b)) {
        return false;
    }
    theValue.left()   = l;
    theValue.right()  = r;
    theValue.top()    = t;
    theValue.bottom() = b;
    return true;
}

bool StSettings::saveInt32Rect(const StString&        theLabel,
                               const StRect<int32_t>& theValue) {
    return saveInt32(theLabel + ".left",   (int32_t )theValue.left())
        && saveInt32(theLabel + ".right",  (int32_t )theValue.right())
        && saveInt32(theLabel + ".top",    (int32_t )theValue.top())
        && saveInt32(theLabel + ".bottom", (int32_t )theValue.bottom());
}

bool StSettings::loadParam(const StString&         theLabel,
                           StHandle<StInt32Param>& theInt32Param) {
    int32_t aValue = theInt32Param->getValue();
    if(loadInt32(theLabel, aValue)) {
        theInt32Param->setValue(aValue);
        return true;
    }
    return false;
}

bool StSettings::saveParam(const StString&               theLabel,
                           const StHandle<StInt32Param>& theInt32Param) {
    return saveInt32(theLabel, theInt32Param->getValue());
}

bool StSettings::loadParam(const StString&        theLabel,
                           StHandle<StBoolParam>& theBoolParam) {
    bool aValue = theBoolParam->getValue();
    if(loadBool(theLabel, aValue)) {
        theBoolParam->setValue(aValue);
        return true;
    }
    return false;
}

bool StSettings::saveParam(const StString&              theLabel,
                           const StHandle<StBoolParam>& theBoolParam) {
    return saveBool(theLabel, theBoolParam->getValue());
}

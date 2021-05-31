/**
 * Copyright © 2007-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StSettings/StSettings.h>
#include <StSlots/StAction.h>
#include <StCore/StVirtualKeys.h>

#include <sstream>

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

bool StSettings::loadFloat(const StString& theLabel,
                           double&         theValue) {
    StString aStrValue;
    if(!loadString(theLabel, aStrValue)) {
        return false;
    }

    std::stringstream aStream;
    aStream.imbue(std::locale("C"));
    aStream << aStrValue.toCString();
    aStream >> theValue;
    return true;
}

bool StSettings::saveFloat(const StString& theLabel,
                           const double    theValue) {
    std::stringstream aStream;
    aStream.imbue(std::locale("C"));
    aStream << theValue;
    const StString aStrValue = aStream.str().c_str();
    return saveString(theLabel, aStrValue);
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

bool StSettings::loadFloatVec4(const StString& theLabel,
                               StVec4<float>&  theValue) {
    StVec4<double> aVec(0.0, 0.0, 0.0, 0.0);
    if(!loadFloat(theLabel + ".x", aVec.x())
    || !loadFloat(theLabel + ".y", aVec.y())
    || !loadFloat(theLabel + ".z", aVec.z())
    || !loadFloat(theLabel + ".w", aVec.w())) {
        return false;
    }
    theValue.x() = float(aVec.x());
    theValue.y() = float(aVec.y());
    theValue.z() = float(aVec.z());
    theValue.w() = float(aVec.w());
    return true;
}

bool StSettings::saveFloatVec4(const StString&      theLabel,
                               const StVec4<float>& theValue) {
    return saveFloat(theLabel + ".x", theValue.x())
        && saveFloat(theLabel + ".y", theValue.y())
        && saveFloat(theLabel + ".z", theValue.z())
        && saveFloat(theLabel + ".w", theValue.w());
}

bool StSettings::loadParam(StHandle<StInt32ParamNamed>& theInt32Param) {
    int32_t aValue = theInt32Param->getValue();
    if(loadInt32(theInt32Param->getKey(), aValue)) {
        theInt32Param->setValue(aValue);
        return true;
    }
    return false;
}

bool StSettings::saveParam(const StHandle<StInt32ParamNamed>& theInt32Param) {
    return saveInt32(theInt32Param->getKey(), theInt32Param->getValue());
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

bool StSettings::loadParam(StHandle<StBoolParamNamed>& theBoolParam) {
    bool aValue = theBoolParam->getValue();
    if(loadBool(theBoolParam->getKey(), aValue)) {
        theBoolParam->setValue(aValue);
        return true;
    }
    return false;
}

bool StSettings::saveParam(const StHandle<StBoolParamNamed>& theBoolParam) {
    return saveBool(theBoolParam->getKey(), theBoolParam->getValue());
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

bool StSettings::loadParam(const StString&           theLabel,
                           StHandle<StFloat32Param>& theFloatParam) {
    double aValue = (double )theFloatParam->getValue();
    if(loadFloat(theLabel, aValue)) {
        theFloatParam->setValue((float )aValue);
        return true;
    }
    return false;
}

bool StSettings::loadParam(StHandle<StFloat32Param>& theFloatParam) {
    double aValue = (double )theFloatParam->getValue();
    if(loadFloat(theFloatParam->getKey(), aValue)) {
        theFloatParam->setValue((float )aValue);
        return true;
    }
    return false;
}

bool StSettings::saveParam(const StString&                 theLabel,
                           const StHandle<StFloat32Param>& theFloatParam) {
    return saveFloat(theLabel, theFloatParam->getValue());
}

bool StSettings::saveParam(const StHandle<StFloat32Param>& theFloatParam) {
    return saveFloat(theFloatParam->getKey(), theFloatParam->getValue());
}

bool StSettings::loadHotKey(StHandle<StAction>& theAction) {
    if(theAction->getName().isEmpty()) {
        return false;
    }

    bool isOK = true;
    StString aKeyStr;
    if(loadString(StString("key") + theAction->getName() + StString("1"), aKeyStr)) {
        theAction->setHotKey1(decodeHotKey(aKeyStr));
    } else {
        isOK = false;
    }

    if(loadString(StString("key") + theAction->getName() + StString("2"), aKeyStr)) {
        theAction->setHotKey2(decodeHotKey(aKeyStr));
    } else {
        isOK = false;
    }

    return isOK;
}

bool StSettings::saveHotKey(const StHandle<StAction>& theAction) {
    return !theAction->getName().isEmpty()
        && saveString(StString("key") + theAction->getName() + StString("1"),
                      encodeHotKey(theAction->getHotKey1()))
        && saveString(StString("key") + theAction->getName() + StString("2"),
                      encodeHotKey(theAction->getHotKey2()));
}

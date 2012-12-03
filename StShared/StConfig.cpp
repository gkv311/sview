/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StConfig.h>
#include <StThreads/StProcess.h>
#include <StStrings/StLogger.h>
#include <StLibrary.h>

namespace {
    static const char LIB_NAME[] = "StSettings";
};

StConfig::StConfig(const StString& theSettingsSet)
: myLib(new StLibrary()),
  myLibInstance(NULL),
  StConfig_new(NULL),
  StConfig_del(NULL),
  StConfig_loadInt32(NULL),
  StConfig_saveInt32(NULL),
  StConfig_loadString(NULL),
  StConfig_saveString(NULL) {
    if(!myLib->load(StProcess::getStCoreFolder() + LIB_NAME)) {
        ST_DEBUG_LOG("Settings plugin not available!");
        return;
    }

    myLib->find("StConfig_new",        StConfig_new);
    myLib->find("StConfig_del",        StConfig_del);
    myLib->find("StConfig_loadInt32",  StConfig_loadInt32);
    myLib->find("StConfig_saveInt32",  StConfig_saveInt32);
    myLib->find("StConfig_loadString", StConfig_loadString);
    myLib->find("StConfig_saveString", StConfig_saveString);

    if(StConfig_new == NULL
    || StConfig_del == NULL
    || StConfig_loadInt32  == NULL
    || StConfig_saveInt32  == NULL
    || StConfig_loadString == NULL
    || StConfig_saveString == NULL) {
        ST_DEBUG_LOG("Broken settings plugin!");
        return;
    }

    myLibInstance = (StConfigInterface* )StConfig_new(theSettingsSet.toCString());
}

StConfig::~StConfig() {
    if(StConfig_del != NULL) {
        StConfig_del(myLibInstance);
    }
}

bool StConfig::loadInt32(const StString& theParam,
                         int32_t&        theOutValue) {
    return isValid() && StConfig_loadInt32(myLibInstance, theParam.toCString(), theOutValue);
}

bool StConfig::saveInt32(const StString& theParam,
                         const int32_t&  theValue) {
    return isValid() && StConfig_saveInt32(myLibInstance, theParam.toCString(), theValue);
}

bool StConfig::loadString(const StString& theParam,
                          StString&       theOutValue) {
    stUtf8_t aBuffer[MAX_STRING_LENGHT];
    if(!isValid()
    || !StConfig_loadString(myLibInstance, theParam.toCString(), aBuffer)) {
        return false;
    }
    theOutValue = StString(aBuffer);
    return true;
}

bool StConfig::saveString(const StString& theParam,
                          const StString& theValue) {
    return isValid() && StConfig_saveString(myLibInstance, theParam.toCString(), theValue.toCString());
}

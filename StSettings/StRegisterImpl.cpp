/**
 * Copyright © 2007-2012 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include "StRegisterImpl.h"

#if(defined(_WIN32) || defined(__WIN32__))

#include <windows.h>

StRegisterImpl::StRegisterImpl(const StStringUtfWide& theSettingsSet)
: mySettingsSet(theSettingsSet),
  myRegisterPath(StStringUtfWide("SOFTWARE\\sView\\") + theSettingsSet) {
    //
}

StRegisterImpl::~StRegisterImpl() {
    //
}

bool StRegisterImpl::loadInt32(const StString& theParam,
                               int32_t&        theValue) {
    HKEY hKey = NULL;
    DWORD aValueSize = sizeof(DWORD);
    DWORD aData = 0;
    RegOpenKeyExW(HKEY_CURRENT_USER, myRegisterPath.toCString(), 0, KEY_READ, &hKey);
    if(RegQueryValueExW(hKey, theParam.toUtfWide().toCString(), NULL, NULL, (LPBYTE )&aData, &aValueSize) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        theValue = aData;
        return true;
    }
    RegCloseKey(hKey);
    return false;
}

bool StRegisterImpl::saveInt32(const StString& theParam,
                               const int32_t&  theValue) {
    HKEY hKey = NULL;
    DWORD aData = theValue;
    DWORD aDisp = 0;
    int anErr = 0;
    if(RegCreateKeyExW(HKEY_CURRENT_USER, myRegisterPath.toCString(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &aDisp)) {
        anErr = 1;     // No write registry success!
    } else {
        if(RegSetValueExW(hKey, theParam.toUtfWide().toCString(), 0, REG_DWORD, (LPBYTE )&aData, sizeof(DWORD))) {
            anErr = 2; // Could not set parameter into registry!
        }
    }
    RegCloseKey(hKey);
    return (anErr == 0);
}

bool StRegisterImpl::loadString(const StString& theParam,
                                StString&       theValue) {
    // TODO (Kirill Gavrilov) parse ERROR_MORE_DATA error (increase buffer)
    stUtfWide_t aDataOut[MAX_STRING_LENGHT];
    HKEY hKey = NULL;
    DWORD aValueSize = sizeof(stUtfWide_t) * MAX_STRING_LENGHT;
    RegOpenKeyExW(HKEY_CURRENT_USER, myRegisterPath.toCString(), 0, KEY_READ, &hKey);
    if(RegQueryValueExW(hKey, theParam.toUtfWide().toCString(), NULL, NULL, (LPBYTE )aDataOut, &aValueSize) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        theValue = StString(aDataOut);
        return true;
    }
    RegCloseKey(hKey);
    return false;
}

bool StRegisterImpl::saveString(const StString& theParam,
                                const StString& theValue) {
    HKEY hKey = NULL;
    DWORD aDisp = 0;
    int anErr = 0;
    StStringUtfWide aValue = theValue.toUtfWide();
    if(RegCreateKeyExW(HKEY_CURRENT_USER, myRegisterPath.toCString(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &aDisp)) {
        anErr = 1; // No write registry success!
    } else if(RegSetValueExW(hKey, theParam.toUtfWide().toCString(), 0, REG_SZ, (LPBYTE )aValue.toCString(), DWORD(aValue.getSize() + sizeof(stUtfWide_t)))) {
        anErr = 2; // Could not set parameter into registry!
    }
    RegCloseKey(hKey);
    return (anErr == 0);
}

// Exported class-methods wpappers.
ST_EXPORT StConfigInterface* StConfig_new(const stUtf8_t* theSettingsSet) {
    return new StRegisterImpl(theSettingsSet);
}

ST_EXPORT void StConfig_del(StConfigInterface* theInst) {
    delete (StRegisterImpl* )theInst;
}

ST_EXPORT stBool_t StConfig_loadInt32(StConfigInterface* theInst,
                                      const stUtf8_t*    theParam,
                                      int32_t&           theValue) {
    return ((StRegisterImpl* )theInst)->loadInt32(StString(theParam), theValue);
}

ST_EXPORT stBool_t StConfig_saveInt32(StConfigInterface* theInst,
                                      const stUtf8_t*    theParam,
                                      const int32_t&     theValue) {
    return ((StRegisterImpl* )theInst)->saveInt32(StString(theParam), theValue);
}

ST_EXPORT stBool_t StConfig_loadString(StConfigInterface* theInst,
                                       const stUtf8_t*    theParam,
                                       stUtf8_t*          theValue) {
    StString aBuff;
    if(!((StRegisterImpl* )theInst)->loadString(StString(theParam), aBuff)) {
        return ST_FALSE;
    }
    size_t aSize = stMin(StConfigInterface::MAX_STRING_LENGHT - 1, aBuff.getSize());
    stMemCpy(theValue, aBuff.toCString(), aSize);
    theValue[aSize] = stUtf8_t('\0');
    return ST_TRUE;
}

ST_EXPORT stBool_t StConfig_saveString(StConfigInterface* theInst, const stUtf8_t* theParam, const stUtf8_t* theValue) {
    return ((StRegisterImpl* )theInst)->saveString(StString(theParam), StString(theValue));
}

#endif // defined(_WIN32)

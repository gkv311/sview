/**
 * Copyright © 2007-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StSettings.h>

#ifdef _WIN32

#include <windows.h>

StSettings::StSettings(const StString& theSettingsSet)
: mySettingsSet(theSettingsSet.toUtfWide()),
  myRegisterPath(StStringUtfWide("SOFTWARE\\sView\\") + theSettingsSet.toUtfWide()) {
    //
}

StSettings::~StSettings() {
    //
}

bool StSettings::loadInt32(const StString& theParam,
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

bool StSettings::saveInt32(const StString& theParam,
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

bool StSettings::loadString(const StString& theParam,
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

bool StSettings::saveString(const StString& theParam,
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

#endif // defined(_WIN32)

/**
 * Copyright © 2007-2015 Kirill Gavrilov
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StSettings/StSettings.h>

#ifdef _WIN32

#include <windows.h>

/**
 * Simple wrapper over registry key.
 */
struct StRegKey {

    HKEY Key;

        public:

    operator HKEY() const { return Key; }

    /**
     * Empty constructor.
     */
    StRegKey() : Key(NULL) {}

    /**
     * Destructor - close the key handle.
     */
    ~StRegKey() {
        if (Key != NULL) { RegCloseKey(Key); }
    }

    /**
     * Open (read-only) existing key. Will fail if key does not exists.
     */
    bool open(const StStringUtfWide& thePath) {
        return RegOpenKeyExW(HKEY_CURRENT_USER, thePath.toCString(), 0,
                             KEY_READ, &Key) == ERROR_SUCCESS;
    }

    /**
     * Create/open key for writing.
     */
    bool create(const StStringUtfWide& thePath) {
        return RegCreateKeyExW(HKEY_CURRENT_USER, thePath.toCString(), 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &Key, NULL) == ERROR_SUCCESS;
    }

};

StSettings::StSettings(const StHandle<StResourceManager>& /*theResMgr*/,
                       const StString&                    theSettingsSet)
: mySettingsSet(theSettingsSet.toUtfWide()),
  myRegisterPath(StStringUtfWide("SOFTWARE\\sView\\") + theSettingsSet.toUtfWide()),
  myToFlush(false) {
    //
}

StSettings::~StSettings() {
    //
}

bool StSettings::load() {
    return true; // has no effect
}

bool StSettings::flush() {
    return true; // has no effect
}

bool StSettings::loadInt32(const StString& theParam,
                           int32_t&        theValue) {
    StRegKey aKey;
    if(!aKey.open(myRegisterPath)) {
        return false;
    }

    DWORD aValueSize = sizeof(DWORD);
    DWORD aData = 0;
    if(RegQueryValueExW(aKey, theParam.toUtfWide().toCString(), NULL, NULL, (LPBYTE )&aData, &aValueSize) != ERROR_SUCCESS) {
        return false;
    }

    theValue = aData;
    return true;
}

bool StSettings::saveInt32(const StString& theParam,
                           const int32_t&  theValue) {
    StRegKey aKey;
    if(!aKey.create(myRegisterPath)) {
        return false; // No write registry success!
    }

    DWORD aData = theValue;
    return RegSetValueExW(aKey, theParam.toUtfWide().toCString(), 0,
                          REG_DWORD, (LPBYTE )&aData, sizeof(DWORD)) == ERROR_SUCCESS;
}

bool StSettings::loadString(const StString& theParam,
                            StString&       theValue) {
    const StStringUtfWide aParam = theParam.toUtfWide();
    StRegKey aKey;
    if(!aKey.open(myRegisterPath)) {
        return false;
    }

    // request size
    DWORD aValueSize = 0;
    if(RegQueryValueExW(aKey, aParam.toCString(), NULL, NULL, NULL, &aValueSize) != ERROR_SUCCESS) {
        return false;
    }

    if(aValueSize == 0) {
        theValue = "";
        return true;
    }

    DWORD aValueSizeRead = aValueSize;
    stUtfWide_t* aDataOut = new stUtfWide_t[aValueSize / sizeof(stUtfWide_t) + 1];
    if(RegQueryValueExW(aKey, aParam.toCString(), NULL, NULL, (LPBYTE )aDataOut, &aValueSizeRead) != ERROR_SUCCESS) {
        return false;
    }

    aDataOut[aValueSize / sizeof(stUtfWide_t)] = L'\0'; // NULL-terminate
    theValue = StString(aDataOut);
    return true;
}

bool StSettings::saveString(const StString& theParam,
                            const StString& theValue) {
    StRegKey aKey;
    if(!aKey.create(myRegisterPath)) {
        return false; // No write registry success!
    }

    StStringUtfWide aValue = theValue.toUtfWide();
    return RegSetValueExW(aKey, theParam.toUtfWide().toCString(), 0,
                          REG_SZ, (LPBYTE )aValue.toCString(), DWORD(aValue.getSize() + sizeof(stUtfWide_t))) == ERROR_SUCCESS;
}

#endif // defined(_WIN32)

/**
 * Copyright © 2008-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StLibrary.h>

#include <stAssert.h>

#ifdef _WIN32
StString StLibrary::DLibGetVersion(const StStringUtfWide& theLibPath) {
    DWORD aFVInfoSize = GetFileVersionInfoSizeW(theLibPath.toCString(), NULL);
    if(aFVInfoSize == 0) {
        return StString();
    }
    LPVOID aBlock = stMemAlloc(aFVInfoSize);
    GetFileVersionInfoW(theLibPath.toCString(), NULL, aFVInfoSize, aBlock);
    VS_FIXEDFILEINFO* aFixedInfo;
    UINT aBufferSize;
    stUtfWide_t aPathToSubBlock[] = L"\\";
    if(VerQueryValueW(aBlock, aPathToSubBlock, (LPVOID* )&aFixedInfo, &aBufferSize) == FALSE ||
       aBufferSize < sizeof(VS_FIXEDFILEINFO)) {
        stMemFree(aBlock);
        return StString();
    }
    StString aVersion = StString("ver.")
        + HIWORD(aFixedInfo->dwFileVersionMS) + '.'
        + LOWORD(aFixedInfo->dwFileVersionMS) + '.'
        + HIWORD(aFixedInfo->dwFileVersionLS) + '.'
        + LOWORD(aFixedInfo->dwFileVersionLS);
    stMemFree(aBlock);
    return aVersion;
}
#endif

StLibrary::StLibrary()
: myLibH(NULL),
  myPath() {}

StLibrary::~StLibrary() {
    close();
}

const StString& StLibrary::getPath() const {
    return myPath;
}

bool StLibrary::load(const StString& thePath) {
    // this is probably some logical error in the code if close() wasn't explicitly called before!
    ST_ASSERT(!isOpened(), "StLibrary::load() - library is already opened!");
    close();
    StString aDinLibExt = StString(ST_DLIB_SUFFIX);
    if(thePath.isEndsWithIgnoreCase(aDinLibExt)) {
        // got the full path?
        myPath = thePath;
        myLibH = DLibLoadFull(myPath);
        if(myLibH == NULL) {
            // try to remove an extension
            myPath = thePath.subString(0, thePath.getLength() - aDinLibExt.getLength());
            myLibH = DLibLoad(myPath);
        }
    } else {
        // got short name?
        myPath = thePath;
        myLibH = DLibLoad(myPath);
    }
    return isOpened();
}

bool StLibrary::loadSimple(const StString& thePath) {
    // this is probably some logical error in the code if close() wasn't explicitly called before!
    ST_ASSERT(!isOpened(), "StLibrary::load() - library is already opened!");
    close();
    myPath = thePath;
    myLibH = DLibLoadFull(myPath);
    return isOpened();
}

void StLibrary::close() {
    if(myLibH != NULL) {
        DLibFree(myLibH);
        myLibH = NULL;
    }
}

void StLibrary::suppressSystemErrors(bool toSuppress) {
#ifdef _WIN32
    SetErrorMode(toSuppress ? SEM_FAILCRITICALERRORS : 0);
    ST_DEBUG_LOG("WinAPI, Critical errors " + (toSuppress ? "suppressed!" : "unsuppressed."));
#endif
}

HMODULE StLibrary::DLibLoadFull(const StString& theLibName) {
#ifdef _WIN32
    HMODULE aModule = LoadLibraryW(theLibName.toUtfWide().toCString());
    if(aModule == NULL) {
        ST_DEBUG_LOG("Failed to load library: \"" + theLibName + "\" (" + (int )GetLastError() + ')');
    } else {
    #ifdef ST_DEBUG_LIBS
        ST_DEBUG_LOG("Loaded library: \"" + theLibName + "\" " + DLibGetVersion(theLibName.toUtfWide()));
    #endif
    }
    return aModule;
#else
    HMODULE aModule = dlopen(theLibName.toCString(), RTLD_NOW);
    if(aModule == NULL) {
        ST_DEBUG_LOG("Failed to load library: \"" + theLibName + "\" (" + dlerror() + ')');
    } else {
    #ifdef ST_DEBUG_LIBS
        ST_DEBUG_LOG("Loaded library: \"" + theLibName + '\"');
    #endif
    }
    return aModule;
#endif
}

HMODULE StLibrary::DLibLoad(const StString& theLibNameText) {
    StString aName = theLibNameText + ST_DLIB_SUFFIX;
    // try current/system folders on Windows or system folders on Linux
    HMODULE hLib = DLibLoadFull(aName);
    if(hLib != NULL) {
        return hLib;
    }
    // try folder up
    hLib = DLibLoadFull(StString("../") + aName);
    if(hLib != NULL) {
        return hLib;
    }
    // try current folder on Linux
    return DLibLoadFull(StString("./") + aName);
}

void* StLibrary::DLibGetfunction(HMODULE theLibHandle, const char* theFuncName) {
#ifdef _WIN32
    return (HMODULE )GetProcAddress(theLibHandle, theFuncName);
#else
    return dlsym(theLibHandle, theFuncName);
#endif
}

void StLibrary::DLibFree(HMODULE theLibHandle) {
#ifdef _WIN32
    FreeLibrary(theLibHandle);
#else
    dlclose(theLibHandle);
#endif
}

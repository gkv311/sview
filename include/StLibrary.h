/**
 * This is a header for dynamic library wrappers.
 * Copyright © 2008-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StLibrary_h_
#define __StLibrary_h_

#include "StVersion.h" // header for version structure
#if (defined(_WIN32) || defined(__WIN32__))
    #include <windows.h> // needed for mingw
    static const stUtf8_t ST_DLIB_SUFFIX[]   = ".dll";
    static const stUtf8_t ST_DLIB_EXTENSION[] = "dll";
#elif (defined(__APPLE__))
    #include <dlfcn.h>
    #define HMODULE void*
    static const stUtf8_t ST_DLIB_SUFFIX[]   = ".dylib";
    static const stUtf8_t ST_DLIB_EXTENSION[] = "dylib";
#else
    #include <dlfcn.h>
    #define HMODULE void*
    static const stUtf8_t ST_DLIB_SUFFIX[]   = ".so";
    static const stUtf8_t ST_DLIB_EXTENSION[] = "so";
#endif

#if (defined(__ST_DEBUG__)) && (defined(_WIN32) || defined(__WIN32__))
inline StString DLibGetVersion(const stUtfWide_t* theLibPath) {
    DWORD aFVInfoSize = GetFileVersionInfoSizeW(theLibPath, NULL);
    if(aFVInfoSize == 0) {
        return StString();
    }
    LPVOID aBlock = stMemAlloc(aFVInfoSize);
    GetFileVersionInfoW(theLibPath, NULL, aFVInfoSize, aBlock);
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

// std error code, returned by sView libraries
enum {
    STERROR_LIBNOERROR = 0,
    STERROR_LIBLOADFAILED = -1,
    STERROR_LIBFUNCTIONNOTFOUND_VER = -2,
    STERROR_LIBVERSIONMISSMATCH = -3,
    STERROR_LIBFUNCTIONNOTFOUND = -100,
};

static const stUtf8_t STERROR_LIBNOERROR_STR[]              = "No library errors";
static const stUtf8_t STERROR_LIBLOADFAILED_STR[]           = "Library not found";
static const stUtf8_t STERROR_LIBFUNCTIONNOTFOUND_VER_STR[] = "Library testVersion function not found (bad library?)";
static const stUtf8_t STERROR_LIBVERSIONMISSMATCH_STR[]     = "Library version check failed";
static const stUtf8_t STERROR_LIBFUNCTIONNOTFOUND_STR[]     = "Library functions not found (maybe internal library error)!";
static const stUtf8_t STERROR_LIBUNKNOWN_STR[]              = "Library error not recognized (reserved?)";

/**
 * Function return error description.
 * @param theErrorCode (int ) - error code;
 * @return errorDesc (const stUtf8_t* ) - description.
 */
inline const stUtf8_t* stGetErrorDesc(int theErrorCode){
    switch(theErrorCode){
        case STERROR_LIBNOERROR:
            { return STERROR_LIBNOERROR_STR; }
        case STERROR_LIBLOADFAILED:
            { return STERROR_LIBLOADFAILED_STR; }
        case STERROR_LIBFUNCTIONNOTFOUND_VER:
            { return STERROR_LIBFUNCTIONNOTFOUND_VER_STR; }
        case STERROR_LIBVERSIONMISSMATCH:
            { return STERROR_LIBVERSIONMISSMATCH_STR; }
        case STERROR_LIBFUNCTIONNOTFOUND:
            { return STERROR_LIBFUNCTIONNOTFOUND_STR; }
        default:
            { return STERROR_LIBUNKNOWN_STR; }
    }
}

#ifdef __cplusplus

/**
 * @class This class is a wrapper for dynamic library load procedures.
 */
class ST_LOCAL StLibrary {

        public:

    /**
     * Empty constructor.
     */
    StLibrary()
    : myLibH(NULL),
      myPath() {}

    virtual ~StLibrary() {
        close();
    }

    /**
     * Load the library.
     * If the given path is not absolute than system-dependent search rules will be used.
     * @param thePath (const StString& ) - path to the library;
     * @return true if library was loaded.
     */
    bool load(const StString& thePath) {
        // this is probably some logical error in the code if close() wasn't explicitly called before!
        ST_DEBUG_ASSERT(!isOpened());
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

    /**
     * Load the library.
     * The same as load() but do not perform any checks - just try to load given path within current environment.
     * Thus name should contains the extension.
     * @param thePath (const StString& ) - path to the library;
     * @return true if library was loaded.
     */
    bool loadSimple(const StString& thePath) {
        // this is probably some logical error in the code if close() wasn't explicitly called before!
        ST_DEBUG_ASSERT(!isOpened());
        close();
        myPath = thePath;
        myLibH = DLibLoadFull(myPath);
        return isOpened();
    }

    /**
     * Close the library.
     */
    void close() {
        if(myLibH != NULL) {
            DLibFree(myLibH);
            myLibH = NULL;
        }
    }

    /**
     * @return true if dynamic library is in loaded state.
     */
    bool isOpened() const {
        return myLibH != NULL;
    }

    /**
     * Function search the function in library by its name.
     * Notice that the function type is NOT controlled in any case!
     * If your function has wrong definition its probably halt
     * application on first call to given pointer.
     * Returned pointer is valid only while library is opened.
     * @param theFuncName (const char* ) - the function name to find;
     * @param theFuncPtr (FuncType& ) - the function pointer to set;
     * @return true if function with given name was found in the library.
     */
    template <typename FuncType>
    bool find(const char* theFuncName, FuncType& theFuncPtr) const {
        theFuncPtr = (FuncType )DLibGetfunction(myLibH, theFuncName);
    #ifdef __ST_DEBUG__
        if(theFuncPtr == NULL) {
            ST_DEBUG_LOG("StLibrary, function \"" + theFuncName + "\" not found");
            return false;
        }
        return true;
    #else
        return (theFuncPtr != NULL);
    #endif
    }

    /**
     * Low-level function.
     */
    void* find(const char* theFuncName) const {
        return DLibGetfunction(myLibH, theFuncName);
    }

    /**
     * Fast link to the find().
     */
    template <typename FuncType>
    bool operator()(const char* theFuncName, FuncType& theFuncPtr) const {
        return find(theFuncName, theFuncPtr);
    }

    /**
     * @return the path to the library (as was set when loaded).
     */
    const StString& getPath() const {
        return myPath;
    }

    /**
     * Function control the system errors mechanism.
     * @param toSuppress (bool ) - to suppress or not the error messages.
     */
    static void suppressSystemErrors(bool toSuppress) {
    #if (defined(_WIN32) || defined(__WIN32__))
        SetErrorMode(toSuppress ? SEM_FAILCRITICALERRORS : 0);
        ST_DEBUG_LOG("WinAPI, Critical errors " + (toSuppress ? "suppressed!" : "unsuppressed."));
    #endif
    }

        private:

    static HMODULE DLibLoad(const StString& theLibNameText) {
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

    static HMODULE DLibLoadFull(const StString& theLibName) {
    #if (defined(_WIN32) || defined(__WIN32__))
        HMODULE aModule = LoadLibraryW(theLibName.toUtfWide().toCString());
        if(aModule == NULL) {
            ST_DEBUG_LOG("Failed to load library: \"" + theLibName + "\" (" + (int )GetLastError() + ')');
        } else {
        #ifdef __ST_DEBUG_LIBS__
            ST_DEBUG_LOG("Loaded library: \"" + theLibName + "\" " + DLibGetVersion(theLibName.toUtfWide().toCString()));
        #endif
        }
        return aModule;
    #else
        HMODULE aModule = dlopen(theLibName.toCString(), RTLD_NOW);
        if(aModule == NULL) {
            ST_DEBUG_LOG("Failed to load library: \"" + theLibName + "\" (" + dlerror() + ')');
        } else {
        #ifdef __ST_DEBUG_LIBS__
            ST_DEBUG_LOG("Loaded library: \"" + theLibName + '\"');
        #endif
        }
        return aModule;
    #endif
    }

    static void* DLibGetfunction(HMODULE theLibHandle, const char* theFuncName) {
    #if (defined(_WIN32) || defined(__WIN32__))
        return (HMODULE )GetProcAddress(theLibHandle, theFuncName);
    #else
        return dlsym(theLibHandle, theFuncName);
    #endif
    }

    static void DLibFree(HMODULE theLibHandle) {
    #if (defined(_WIN32) || defined(__WIN32__))
        FreeLibrary(theLibHandle);
    #else
        dlclose(theLibHandle);
    #endif
    }

        private:

    HMODULE  myLibH; //!< handle to the opened library
    StString myPath; //!< path to the opened library

};

#endif //__cplusplus
#endif //__StLibrary_h_

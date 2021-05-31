/**
 * This is a header for dynamic library wrappers.
 * Copyright © 2008-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StLibrary_h_
#define __StLibrary_h_

#include "StVersion.h" // header for version structure
#if defined(_WIN32)
    #include <windows.h> // needed for mingw
    static const stUtf8_t ST_DLIB_SUFFIX[]   = ".dll";
    static const stUtf8_t ST_DLIB_EXTENSION[] = "dll";
#elif defined(__APPLE__)
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

/**
 * @class This class is a wrapper for dynamic library load procedures.
 */
class StLibrary {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StLibrary();

    ST_CPPEXPORT virtual ~StLibrary();

    /**
     * Load the library.
     * If the given path is not absolute than system-dependent search rules will be used.
     * @param thePath path to the library
     * @return true if library was loaded
     */
    ST_CPPEXPORT bool load(const StString& thePath);

    /**
     * Load the library.
     * The same as load() but do not perform any checks - just try to load given path within current environment.
     * Thus name should contains the extension.
     * @param thePath path to the library
     * @return true if library was loaded
     */
    ST_CPPEXPORT bool loadSimple(const StString& thePath);

    /**
     * Close the library.
     */
    ST_CPPEXPORT void close();

    /**
     * @return true if dynamic library is in loaded state.
     */
    inline bool isOpened() const {
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
    inline bool find(const char* theFuncName, FuncType& theFuncPtr) const {
        theFuncPtr = (FuncType )DLibGetfunction(myLibH, theFuncName);
    #ifdef ST_DEBUG
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
    inline void* find(const char* theFuncName) const {
        return DLibGetfunction(myLibH, theFuncName);
    }

    /**
     * Fast link to the find().
     */
    template <typename FuncType>
    inline bool operator()(const char* theFuncName, FuncType& theFuncPtr) const {
        return find(theFuncName, theFuncPtr);
    }

    /**
     * @return the path to the library (as was set when loaded)
     */
    ST_CPPEXPORT const StString& getPath() const;

    /**
     * Function control the system errors mechanism.
     * @param toSuppress to suppress or not the error messages
     */
    ST_CPPEXPORT static void suppressSystemErrors(bool toSuppress);

        private: //! @name C-interface

    ST_LOCAL static HMODULE DLibLoad(const StString& theLibNameText);

    ST_LOCAL static HMODULE DLibLoadFull(const StString& theLibName);

    ST_CPPEXPORT static void* DLibGetfunction(HMODULE     theLibHandle,
                                              const char* theFuncName);

    ST_LOCAL static void DLibFree(HMODULE theLibHandle);

#ifdef _WIN32
    ST_LOCAL static StString DLibGetVersion(const StStringUtfWide& theLibPath);
#endif

        private:

    HMODULE  myLibH; //!< handle to the opened library
    StString myPath; //!< path to the opened library

};

#endif //__StLibrary_h_

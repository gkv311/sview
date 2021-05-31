/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StProcess_h_
#define __StProcess_h_

#include <StStrings/StDictionary.h>

#include "StMutex.h"

// compatibility with old definitions
typedef StDictEntry  StArgument;
typedef StDictionary StArgumentsMap;

/**
 * Helper class to get process/system variables.
 */
class StProcess {

        public:

    /**
     * @return absolute process path WITH executable name
     */
    ST_CPPEXPORT static StString getProcessFullPath();

    /**
     * @return process name
     */
    ST_CPPEXPORT static StString getProcessName();

    /**
     * @return absolute process path WITHOUT executable name (including trailing separator)
     */
    ST_CPPEXPORT static StString getProcessFolder();

    /**
     * @return current working folder (including trailing separator)
     */
    ST_CPPEXPORT static StString getWorkingFolder();

    /**
     * @return folder for temporary files (including trailing separator)
     */
    ST_CPPEXPORT static StString getTempFolder();

    /**
     * Make absolute path (using current working folder as root) from relative.
     * @param thePath original path
     * @return absolute path
     */
    ST_CPPEXPORT static StString getAbsolutePath(const StString& thePath);

    /**
     * @return process ID
     */
    ST_CPPEXPORT static size_t getPID();

#ifdef _WIN32
    /**
     * @return absolute path to Windows directory
     */
    ST_CPPEXPORT static StString getWindowsFolder();
#endif

    /**
     * @return command line arguments as flat list
     */
    ST_CPPEXPORT static StArrayList<StString> getArguments();

    /**
     * @return command line arguments list as map of key+value
     */
    //static StArgumentsMap getArgumentsMap();

    /**
     * Function returns current environment variable value.
     * @param theName environment variable name
     * @return variable value
     */
    static StString getEnv(const StString& theName) {
    #if defined(_WIN32)
        return StString(_wgetenv(theName.toUtfWide().toCString()));
    #else
        return StString(::getenv(theName.toCString()));
    #endif
    }

    /**
     * Function edit value for environment variable or create the new one.
     * @param theName  environment variable name
     * @param theValue variable value
     */
    static void setEnv(const StString& theName, const StString& theValue) {
    #if defined(_WIN32)
        StString setVariable = theName + '=' + theValue;
        _wputenv(setVariable.toUtfWide().toCString());
    #else
        ::setenv(theName.toCString(), theValue.toCString(), 1); // 1 means overwrite
    #endif
    }

    /**
     * This method returns absolute path to the folder where all shared resources are installed.
     * These resources include textures, translation files, shaders sources.
     * @return absolute path to shared resources
     */
    ST_CPPEXPORT static StString getStShareFolder();

    /**
     * This method returns absolute path to the folder where sView executables and libraries are placed.
     * It will be either:
     *  - path returned by $StCore32/$StCore64 environment variable
     *  - predefined path (default installation path)
     *  - current process path
     *  - empty string
     * Notice that this path should be used carefully.
     * @return absolute path to the folder with sView executable
     */
    ST_CPPEXPORT static StString getStCoreFolder();

    /**
     * Launch specified process without waiting its termination.
     * @param theExecutablePath absolute path to the executable
     * @param theArguments      arguments list
     * @return true if executable has been launched
     */
    ST_CPPEXPORT static bool execProcess(const StString&          theExecutablePath,
                                         const StArray<StString>& theArguments);

    /**
     * Open specified URL using external application.
     */
    ST_CPPEXPORT static void openURL(const StString& theUrl);

};

#endif // __StProcess_h_

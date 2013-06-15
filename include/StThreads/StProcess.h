/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StProcess_h_
#define __StProcess_h_

#include <StStrings/StString.h>
#include <StTemplates/StArrayList.h>

#include "StMutex.h"

/**
 * This class represents an argument (to program for example) in style key+value.
 */
class StArgument {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StArgument();

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StArgument(const StString& theKey,
                            const StString& theValue);

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StArgument();

    /**
     * Check key is not empty.
     */
    bool isValid() const {
        return !key.isEmpty();
    }

    /**
     * Default values 'on' or 'true' are preferred for switch.
     */
    ST_CPPEXPORT bool isValueOn() const;

    /**
     * Default values 'off' or 'false' are preferred for switch.
     */
    ST_CPPEXPORT bool isValueOff() const;

    /**
     * Parse string and create key/value pair.
     * String should be in format 'KEY="VALUE"'.
     * Newline '\n' symbol is disallowed in the key string.
     */
    ST_CPPEXPORT void parseString(const StString& theString);

    const StString& getKey() const {
        return key;
    }

    void setKey(const StString& newKey) {
        this->key = newKey;
    }

    const StString& getValue() const {
        return val;
    }

    StString& changeValue() {
        return val;
    }

    void setValue(const StString& val) {
        this->val = val.unquoted();
    }

    /**
     * Arguments keys are NOT case sensitive!
     */
    bool operator==(const StArgument& theCompare) const {
        if(&theCompare == this) {
            return true;
        }
        return theCompare.key.isEqualsIgnoreCase(this->key);
    }

    bool operator!=(const StArgument& theCompare) const {
        if(&theCompare == this) {
            return false;
        }
        return !theCompare.key.isEqualsIgnoreCase(this->key);
    }

    bool operator> (const StArgument& theCompare) const { return this->key >  theCompare.key; }
    bool operator< (const StArgument& theCompare) const { return this->key <  theCompare.key; }
    bool operator>=(const StArgument& theCompare) const { return this->key >= theCompare.key; }
    bool operator<=(const StArgument& theCompare) const { return this->key <= theCompare.key; }

    ST_CPPEXPORT StString toString() const;

        private:

    static const StString ST_ARG_ON;
    static const StString ST_ARG_TRUE;
    static const StString ST_ARG_OFF;
    static const StString ST_ARG_FALSE;

        private:

    StString key; // key
    StString val; // value

};

/**
 * Simple array-map for StArguments. Could be exported/imported into/from string
 * (default '\n' symbol will be used as delimiter).
 */
class StArgumentsMap : public StArrayList<StArgument> {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StArgumentsMap();
    ST_CPPEXPORT virtual ~StArgumentsMap();

    /**
     * Parse string and create arguments list.
     */
    ST_CPPEXPORT void parseList(const StArrayList<StString>& stringList);

    /**
     * Parse string and create arguments list.
     */
    ST_CPPEXPORT void parseString(const StString& string);

    /**
     * Access to the argument throw the key.
     * Returns an empty argument if key not found.
     */
    ST_CPPEXPORT StArgument operator[](const StString& key) const;

    /**
     * Access to the arguments throw indexes.
     * Use parent ::size() method to retrieve map's size.
     */
    inline const StArgument& getFromIndex(const size_t index) const {
        return StArrayList<StArgument>::getValue(index);
    }

    ST_CPPEXPORT virtual StString toString() const;

};

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
     * @return absolute Fonts path
     */
    ST_CPPEXPORT static StString getFontsRoot();

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

};

#endif // __StProcess_h_

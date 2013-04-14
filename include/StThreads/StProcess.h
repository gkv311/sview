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

        private:

    StString key; // key
    StString val; // value

        private:

    static const StString ST_ARG_ON;
    static const StString ST_ARG_TRUE;
    static const StString ST_ARG_OFF;
    static const StString ST_ARG_FALSE;

        public:

    /**
     * Empty constructor.
     */
    StArgument()
    : key(),
      val() {
        //
    }

    StArgument(const StString& key, const StString& value)
    : key(key),
      val(value.unquoted()) {
        //
    }

    ~StArgument() {
        //
    }

    /**
     * Check key is not empty.
     */
    bool isValid() const {
        return !key.isEmpty();
    }

    /**
     * Default values 'on' or 'true' are preffered for switch.
     */
    bool isValueOn() const {
        return val.isEqualsIgnoreCase(ST_ARG_ON) || val.isEqualsIgnoreCase(ST_ARG_TRUE);
    }

    /**
     * Default values 'off' or 'false' are preffered for switch.
     */
    bool isValueOff() const {
        return val.isEqualsIgnoreCase(ST_ARG_OFF) || val.isEqualsIgnoreCase(ST_ARG_FALSE);
    }

    /**
     * Parse string and create key/value pair.
     * String should be in format 'KEY="VALUE"'.
     * Newline '\n' symbol is disallowed in the key string.
     */
    void parseString(const StString& string);

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

    StString toString() const {
        return key + "=\"" + val + '\"';
    }

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

    virtual StString toString() const {
        return StArrayList<StArgument>::toString();
    }

};

/**
 * Helper class to get process/system variables.
 */
class StProcess {

        public:

    /**
     * @return (StString ) absolute process path WITH executable name.
     */
    ST_CPPEXPORT static StString getProcessFullPath();

    /**
     * @return (StString ) process name.
     */
    ST_CPPEXPORT static StString getProcessName();

    /**
     * @return (StString ) absolute process path WITHOUT executable name (including trailing separator).
     */
    ST_CPPEXPORT static StString getProcessFolder();

    /**
     * @return (StString ) current working folder (including trailing separator).
     */
    ST_CPPEXPORT static StString getWorkingFolder();

    /**
     * @return (StString ) folder for temporary files (including trailing separator).
     */
    ST_CPPEXPORT static StString getTempFolder();

    /**
     * Make absolute path (using current working folder as root) from relative.
     * @param path (const StString& ) - original path;
     * @return (StString ) - absolute path.
     */
    ST_CPPEXPORT static StString getAbsolutePath(const StString& thePath);

    /**
     * @return (size_t ) process ID.
     */
    ST_CPPEXPORT static size_t getPID();

#ifdef _WIN32
    /**
     * @return (StString ) absolute Windows path.
     */
    ST_CPPEXPORT static StString getWindowsFolder();
#endif

    /**
     * @return (StString ) absolute Fonts path.
     */
    ST_CPPEXPORT static StString getFontsRoot();

    /**
     * @return (StArrayList<StString> ) - command line arguments flat list.
     */
    ST_CPPEXPORT static StArrayList<StString> getArguments();

    /**
     * @return (StArgumentsMap ) - command line arguments list as map of key+value.
     */
    //static StArgumentsMap getArgumentsMap();

    /**
     * Function returns current environment variable value.
     * @param theName (const StString& ) - environment variable name;
     * @return variable value;
     */
    static StString getEnv(const StString& theName) {
    #if (defined(_WIN32) || defined(__WIN32__))
        return StString(_wgetenv(theName.toUtfWide().toCString()));
    #else
        return StString(::getenv(theName.toCString()));
    #endif
    }

    /**
     * Function edit value for environment variable or create the new one.
     * @param theName  (const StString& ) - environment variable name;
     * @param theValue (const StString& ) - variable value;
     */
    static void setEnv(const StString& theName, const StString& theValue) {
    #if (defined(_WIN32) || defined(__WIN32__))
        StString setVariable = theName + '=' + theValue;
        _wputenv(setVariable.toUtfWide().toCString());
    #else
        ::setenv(theName.toCString(), theValue.toCString(), 1); // 1 means overwrite
    #endif
    }

    ST_CPPEXPORT static StString getStCoreFolder();

    ST_CPPEXPORT static bool execProcess(const StString&          theExecutablePath,
                                         const StArray<StString>& theArguments);

};

#endif // __StProcess_h_

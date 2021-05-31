/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StDictionary_h_
#define __StDictionary_h_

#include <StStrings/StString.h>
#include <StTemplates/StArrayList.h>

/**
 * This class represents dictionary entry key+value.
 */
class StDictEntry {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StDictEntry();

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StDictEntry(const StString& theKey);

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StDictEntry(const StString& theKey,
                             const StString& theValue);

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StDictEntry();

    /**
     * Check key is not empty.
     */
    ST_LOCAL bool isValid() const {
        return !myKey.isEmpty();
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

    /**
     * @return key
     */
    ST_LOCAL const StString& getKey() const {
        return myKey;
    }

    /**
     * Change key.
     */
    ST_LOCAL void setKey(const StString& theKey) {
        myKey = theKey;
    }

    /**
     * @return key name
     */
    ST_LOCAL const StString& getName() const {
        return myName;
    }

    /**
     * @return key name
     */
    ST_LOCAL StString& changeName() {
        return myName;
    }

    /**
     * Change name.
     */
    ST_LOCAL void setName(const StString& theName) {
        myName = theName;
    }

    /**
     * Access value.
     */
    ST_LOCAL const StString& getValue() const { return myValue; }
    ST_LOCAL       StString& changeValue()    { return myValue; }

    ST_LOCAL void setValue(const StString& theValue) {
        myValue = theValue.unquoted();
    }

    /**
     * Arguments keys are NOT case sensitive!
     */
    ST_LOCAL bool operator==(const StDictEntry& theCompare) const {
        if(&theCompare == this) {
            return true;
        }
        return theCompare.myKey.isEqualsIgnoreCase(myKey);
    }

    ST_LOCAL bool operator!=(const StDictEntry& theCompare) const {
        if(&theCompare == this) {
            return false;
        }
        return !theCompare.myKey.isEqualsIgnoreCase(myKey);
    }

    ST_LOCAL bool operator> (const StDictEntry& theCompare) const { return myKey >  theCompare.myKey; }
    ST_LOCAL bool operator< (const StDictEntry& theCompare) const { return myKey <  theCompare.myKey; }
    ST_LOCAL bool operator>=(const StDictEntry& theCompare) const { return myKey >= theCompare.myKey; }
    ST_LOCAL bool operator<=(const StDictEntry& theCompare) const { return myKey <= theCompare.myKey; }

    ST_CPPEXPORT StString toString() const;

        private:

    static const StString ST_ARG_ON;
    static const StString ST_ARG_TRUE;
    static const StString ST_ARG_OFF;
    static const StString ST_ARG_FALSE;

        private:

    StString myKey;   //!< key
    StString myValue; //!< value
    StString myName;  //!< key name (optional)

};

/**
 * Simple array-map for StArguments. Could be exported/imported into/from string
 * (default '\n' symbol will be used as delimiter).
 */
class StDictionary : public StArrayList<StDictEntry> {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StDictionary();
    ST_CPPEXPORT virtual ~StDictionary();

    /**
     * Parse string and create arguments list.
     */
    ST_CPPEXPORT void parseList(const StArrayList<StString>& theStringList);

    /**
     * Parse string and create arguments list.
     */
    ST_CPPEXPORT void parseString(const StString& theString);

    /**
     * Access to the argument throw the key.
     * Returns an empty argument if key not found.
     */
    ST_CPPEXPORT const StDictEntry& operator[](const StString& theKey) const;

    /**
     * Access to the arguments throw indexes.
     * Use parent ::size() method to retrieve map's size.
     */
    inline const StDictEntry& getFromIndex(const size_t index) const {
        return StArrayList<StDictEntry>::getValue(index);
    }

    /**
     * Add/modifies value for specified key.
     */
    ST_CPPEXPORT void set(const StDictEntry& thePair);

    ST_CPPEXPORT StDictEntry& addChange(const StString& theKey);

    ST_CPPEXPORT virtual StString toString() const;

};

#endif // __StDictionary_h_

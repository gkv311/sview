/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StLangMap_h__
#define __StLangMap_h__

#include <map> // STL map template

#include <StStrings/StString.h>

/**
 * Key -> string map for translation files.
 * Implements reading from the simple file format:
 * @code
 *   # comment line
 *   1010=Text for key 1010
 *   1011=Text for another key
 * @endcode
 */
class StLangMap {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StLangMap();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StLangMap();

    /**
     * Read the translation file from specified buffer.
     * @param theContent buffer
     * @param theLen     length of the buffer
     */
    ST_CPPEXPORT bool read(const char* theContent,
                           const int   theLen);

    /**
     * If id does not match the key of any element in the container, the function
     * inserts a new element with that key and returns a reference to its mapped value.
     * Notice that this always increases the map size by one, even if no mapped
     * value is assigned to the element (the element is constructed using its default constructor)
     */
    ST_CPPEXPORT StString& changeValue(const size_t theId);
    ST_LOCAL inline StString& operator[](const size_t theId) {
        return changeValue(theId);
    }

    /**
     * @return string for specified key, or empty string if key not found
     */
    ST_CPPEXPORT const StString& getValue(const size_t theId) const;
    ST_LOCAL inline const StString& operator[](const size_t theId) const {
        return getValue(theId);
    }

    /**
     * If value not exists - default value is used.
     */
    ST_CPPEXPORT StString& changeValueId(const size_t theId, const char* theDefaultValue);
    ST_CPPEXPORT StString& operator()   (const size_t theId, const char* theDefaultValue);

    ST_CPPEXPORT size_t size() const;
    ST_CPPEXPORT void clear();

    /**
     * Add string key alias.
     */
    ST_CPPEXPORT void addAlias(const StString& theStringKey,
                               const size_t    theIntKey);

    /**
     * Get value from string key (if alias has been registered).
     */
    ST_CPPEXPORT const StString& getValue(const StString& theStringKey) const;

        public:

    typedef std::map<size_t, StString> stMapInt2String_t;
    typedef std::map<StString, size_t> stMapString2Int_t;

        private:

    StString          myLngFile;    //!< path to the language file
    StString          myEmptyStr;   //!< empty string to return for invalid keys
    stMapInt2String_t myMap;        //!< key -> string map
    stMapString2Int_t myMapStrKeys; //!< auxiliary map of string key -> int key
    bool              myToShowId;

};

#endif //__StLangMap_h__

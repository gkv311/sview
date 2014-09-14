/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StLangMap_h__
#define __StLangMap_h__

#include <map> // STL map template

#include <StStrings/StString.h>

class StLangMap {

        public:

    ST_CPPEXPORT StLangMap();

    ST_CPPEXPORT virtual ~StLangMap();

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

    ST_LOCAL void parseLine(const StString& theLine);

        private:

    StString          myLngFile;         //!< path to the language file
    StString          myEmptyStr;
    stMapInt2String_t myMap;             //!< key -> string map
    stMapString2Int_t myMapStrKeys;      //!< auxiliary map of string key -> int key
    bool              myIsHeaderSection;
    bool              myToShowId;

};

#endif //__StLangMap_h__

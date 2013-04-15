/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StLangMap_h__
#define __StLangMap_h__

#include <map> // STL map template

#include "StString.h"

class StLangMap {

        public:

    static const StString DEFAULT_EXTENSION;
    static const StString DEFAULT_SUFFIX;

        public:

    ST_CPPEXPORT StLangMap();
    ST_CPPEXPORT StLangMap(const StString& theLngFilePath);

    ST_CPPEXPORT virtual ~StLangMap();

    ST_CPPEXPORT bool open(const StString& theLngFilePath);

    /**
     * If id does not match the key of any element in the container, the function
     * inserts a new element with that key and returns a reference to its mapped value.
     * Notice that this always increases the map size by one, even if no mapped
     * value is assigned to the element (the element is constructed using its default constructor)
     */
    ST_CPPEXPORT StString& changeValue(const size_t theId);
    ST_CPPEXPORT StString& operator[] (const size_t theId);

    /**
     * If value not exists - default value is used.
     */
    ST_CPPEXPORT StString& changeValueId(const size_t theId, const StString& theDefaultValue);
    ST_CPPEXPORT StString& operator()   (const size_t theId, const StString& theDefaultValue);

    ST_CPPEXPORT size_t size() const;
    ST_CPPEXPORT void clear();

        private:

    ST_LOCAL void parseLine(const StString& theLine);

        private:

    typedef std::map<size_t, StString> stMapInt2String_t;

    StString      myLngFile; //!< path to the language file
    stMapInt2String_t myMap; //!< key -> string map
    bool  myIsHeaderSection;
    bool         myToShowId;

};

#endif //__StLangMap_h__

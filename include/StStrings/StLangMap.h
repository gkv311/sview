/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StLangMap_h__
#define __StLangMap_h__

#include <map> // STL map template

#include "StString.h"

class ST_LOCAL StLangMap {

        public:

    static const StString DEFAULT_EXTENSION;
    static const StString DEFAULT_SUFFIX;

        private:

    typedef std::map<size_t, StString> stMapInt2String_t;

    StString      myLngFile; //!< path to the language file
    stMapInt2String_t myMap; //!< key -> string map
    bool  myIsHeaderSection;
    bool         myToShowId;

        private:

    void parseLine(const StString& theLine);

        public:

    StLangMap();
    StLangMap(const StString& theLngFilePath);

    virtual ~StLangMap();

    bool open(const StString& theLngFilePath);

    /**
     * If id does not match the key of any element in the container, the function
     * inserts a new element with that key and returns a reference to its mapped value.
     * Notice that this always increases the map size by one, even if no mapped
     * value is assigned to the element (the element is constructed using its default constructor)
     */
    StString& changeValue(const size_t theId);
    StString& operator[](const size_t theId);

    /**
     * If value not exists - default value is used.
     */
    StString& changeValueId(const size_t theId, const StString& theDefaultValue);
    StString& operator()(const size_t theId, const StString& theDefaultValue);

    size_t size() const;
    void clear();

};

#endif //__StLangMap_h__

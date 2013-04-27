/**
 * Copyright © 2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StEnumParam_h_
#define __StEnumParam_h_

#include <StSettings/StParam.h>

/**
 * Enumeration parameter.
 * It is Integer value within 0...N
 * Each value has associated name to create appropriate control in GUI.
 */
class StEnumParam : public StInt32Param {

        public:

    /**
     * Simple constructor.
     */
    StEnumParam(const int32_t   theValue,
                const StString& theParamName = "")
    : StInt32Param(theValue),
      myParamName(theParamName) {
        //
    }

    /**
     * Change the value.
     * @param theValue new value
     * @return true if value was changed
     */
    inline virtual bool setValue(const int32_t theValue) {
        if(theValue < 0
        || size_t(theValue) >= myList.size()) {
            return false; // prevent out-of-range
        }
        return StInt32Param::setValue(theValue);
    }

    /**
     * @return parameter label
     */
    ST_LOCAL inline StString getName() const {
        return myParamName;
    }

    /**
     * Returns title for active value.
     */
    ST_LOCAL inline StString getActiveValue() const {
        if(myList.isEmpty()) {
            return "";
        }
        const int32_t anActive = getValue();
        return myList[(anActive >= 0 && size_t(anActive) < myList.size()) ? size_t(anActive) : 0];
    }

    /**
     * Return list of available options.
     */
    ST_LOCAL inline const StArrayList<StString>& getValues() const {
        return myList;
    }

    ST_LOCAL inline StArrayList<StString>& changeValues() {
        return myList;
    }

        protected:

    StArrayList<StString> myList;
    StString              myParamName;

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StEnumParam, StInt32Param);

#endif // __StEnumParam_h_

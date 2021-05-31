/**
 * Copyright © 2013-2016 Kirill Gavrilov
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StEnumParam_h_
#define __StEnumParam_h_

#include <StSettings/StParam.h>

/**
 * Enumeration parameter.
 * It is Integer value within 0...N
 * Each value has associated name to create appropriate control in GUI.
 */
class StEnumParam : public StInt32ParamNamed {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StEnumParam(const int32_t    theValue,
                         const StCString& theParamKey,
                         const StCString& theParamName)
    : StInt32ParamNamed(theValue, theParamKey, theParamName) {
        //
    }

    /**
     * Main constructor.
     */
    ST_LOCAL StEnumParam(const int32_t    theValue,
                         const StCString& theParamKey)
    : StInt32ParamNamed(theValue, theParamKey) {
        //
    }

    /**
     * Change the value.
     * @param theValue new value
     * @return true if value was changed
     */
    ST_LOCAL virtual bool setValue(const int32_t theValue) {
        if(theValue < 0
        || size_t(theValue) >= myList.size()) {
            return false; // prevent out-of-range
        }
        return StInt32Param::setValue(theValue);
    }

    /**
     * Returns title for active value.
     */
    ST_LOCAL StString getActiveValue() const {
        if(myList.isEmpty()) {
            return "";
        }
        const int32_t anActive = getValue();
        return myList[(anActive >= 0 && size_t(anActive) < myList.size()) ? size_t(anActive) : 0];
    }

    /**
     * Return the list of available options.
     */
    ST_LOCAL const StArrayList<StString>& getValues() const {
        return myList;
    }

    /**
     * Modify the list of available options.
     */
    ST_LOCAL StArrayList<StString>& changeValues() {
        return myList;
    }

    /**
     * Return option label.
     */
    ST_LOCAL const StString& getOptionLabel(const int32_t theValue) const {
        return myList.getValue(theValue);
    }

    /**
     * Setup option in the list.
     */
    ST_LOCAL void defineOption(const int32_t    theValue,
                               const StCString& theName) {
        if(theValue < 0) {
            return;
        }

        while(myList.size() <= (size_t )theValue) {
            myList.add(stCString(""));
        }
        myList.changeValue(theValue) = theName;
    }

        protected:

    StArrayList<StString> myList;

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StEnumParam, StInt32ParamNamed);

#endif // __StEnumParam_h_

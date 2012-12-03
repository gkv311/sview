/**
 * Copyright © 2011 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StParam_h_
#define __StParam_h_

#include <StSlots/StSignal.h>

/**
 * This is a special template class that represent wrapped primitive parameter.
 * Access to the parameter is protected with getValue()/setValue() methods
 * and on change signal onChanged() will be emitted.
 */
template <typename Type>
class ST_LOCAL StParam {

        protected:

    Type myValue; //!< primitive parameter

        public:

    /**
     * Main constructor.
     */
    StParam(const Type theValue)
    : myValue(theValue) {}

    virtual ~StParam() {}

    /**
     * Just retrieve the current value.
     * @return current value.
     */
    virtual Type getValue() const {
        return myValue;
    }

    /**
     * Change the value.
     * @param theValue (const Type ) - new value;
     * @return true if value was changed.
     */
    virtual bool setValue(const Type theValue) {
        if(getValue() != theValue) {
            myValue = theValue;
            signals.onChanged(theValue);
            return true;
        }
        return false;
    }

        public:  //!< Signals

    struct {
        /**
         * Emit callback Slot on value change.
         * @param theNewValue (const Type ) - new value.
         */
        StSignal<void (const Type )> onChanged;
    } signals;

};

/**
 * Integer (enumeration) parameter.
 */
class ST_LOCAL StInt32Param : public StParam<int32_t> {

        public:

    /**
     * Main constructor.
     */
    StInt32Param(int32_t theValue)
    : StParam<int32_t>(theValue) {
        //
    }

};

/**
 * Boolean parameter.
 */
class ST_LOCAL StBoolParam : public StParam<bool> {

        public:

    /**
     * Main constructor.
     */
    StBoolParam(bool theValue)
    : StParam<bool>(theValue) {
        //
    }

    /**
     * Reverse current value.
     * @return new value.
     */
    bool reverse() {
        bool aNewValue = !getValue();
        setValue(aNewValue);
        return aNewValue;
    }

    /**
     * Slot method for compatibility with some widgets.
     */
    void doReverse(const size_t ) {
        reverse();
    }

};

#endif //__StParam_h_

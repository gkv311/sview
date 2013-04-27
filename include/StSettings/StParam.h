/**
 * Copyright © 2011-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StParam_h_
#define __StParam_h_

#include <StSlots/StSignal.h>
#include <StTemplates/StArrayList.h>

/**
 * Base class for all parameters.
 */
class StParamBase {

      public:

    virtual ~StParamBase() {}

};

template<> inline void StArray< StHandle<StParamBase> >::sort() {}
typedef StArrayList< StHandle<StParamBase> > StParamsList;

/**
 * This is a special template class that represent wrapped primitive parameter.
 * Access to the parameter is protected with getValue()/setValue() methods
 * and on change signal onChanged() will be emitted.
 */
template <typename Type>
class StParam : public StParamBase {

        protected:

    Type myValue; //!< primitive parameter

        public:

    /**
     * Main constructor.
     */
    inline StParam(const Type theValue)
    : myValue(theValue) {}

    /**
     * Just retrieve the current value.
     * @return current value.
     */
    inline virtual Type getValue() const {
        return myValue;
    }

    /**
     * Change the value.
     * @param theValue (const Type ) - new value;
     * @return true if value was changed.
     */
    inline virtual bool setValue(const Type theValue) {
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
class StInt32Param : public StParam<int32_t> {

        public:

    /**
     * Main constructor.
     */
    inline StInt32Param(int32_t theValue)
    : StParam<int32_t>(theValue) {
        //
    }

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StParam<int32_t>, StParamBase);
ST_DEFINE_HANDLE(StInt32Param,     StParam<int32_t>);

/**
 * Boolean parameter.
 */
class StBoolParam : public StParam<bool> {

        public:

    /**
     * Main constructor.
     */
    inline StBoolParam(bool theValue)
    : StParam<bool>(theValue) {
        //
    }

    /**
     * Reverse current value.
     * @return new value.
     */
    inline bool reverse() {
        bool aNewValue = !getValue();
        setValue(aNewValue);
        return aNewValue;
    }

    /**
     * Slot method for compatibility with some widgets.
     */
    inline void doReverse(const size_t ) {
        reverse();
    }

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StParam<bool>, StParamBase);
ST_DEFINE_HANDLE(StBoolParam,   StParam<bool>);

/**
 * Named boolean parameter.
 */
class StBoolParamNamed : public StBoolParam {

        public:

    /**
     * Main constructor.
     */
    inline StBoolParamNamed(const bool      theValue,
                            const StString& theParamName = "")
    : StBoolParam(theValue),
      myParamName(theParamName) {
        //
    }

    /**
     * @return parameter label
     */
    ST_LOCAL inline StString getName() const {
        return myParamName;
    }

        private:

    StString myParamName;

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StBoolParamNamed, StBoolParam);

#endif //__StParam_h_

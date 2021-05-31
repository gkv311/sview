/**
 * Copyright © 2011-2016 Kirill Gavrilov
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
    ST_LOCAL StBoolParamNamed(const bool       theValue,
                              const StCString& theParamKey,
                              const StCString& theParamName)
    : StBoolParam(theValue),
      myParamKey(theParamKey),
      myParamName(theParamName) {
        //
    }

    /**
     * Main constructor.
     */
    ST_LOCAL StBoolParamNamed(const bool       theValue,
                              const StCString& theParamKey)
    : StBoolParam(theValue),
      myParamKey(theParamKey),
      myParamName(theParamKey) {
        //
    }

    /**
     * @return parameter key
     */
    ST_LOCAL const StString& getKey() const {
        return myParamKey;
    }

    /**
     * @return parameter label
     */
    ST_LOCAL const StString& getName() const {
        return myParamName;
    }

    /**
     * Set new parameter label.
     */
    ST_LOCAL void setName(const StString& theName) {
        myParamName = theName;
    }

        private:

    StString myParamKey;
    StString myParamName;

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StBoolParamNamed, StBoolParam);

/**
 * Named integer parameter.
 */
class StInt32ParamNamed : public StInt32Param {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StInt32ParamNamed(const int32_t    theValue,
                               const StCString& theParamKey,
                               const StCString& theParamName)
    : StInt32Param(theValue),
      myParamKey(theParamKey),
      myParamName(theParamName) {
        //
    }

    /**
     * Main constructor.
     */
    ST_LOCAL StInt32ParamNamed(const int32_t    theValue,
                               const StCString& theParamKey)
    : StInt32Param(theValue),
      myParamKey(theParamKey),
      myParamName(theParamKey) {
        //
    }

    /**
     * @return parameter key
     */
    ST_LOCAL const StString& getKey() const {
        return myParamKey;
    }

    /**
     * @return parameter label
     */
    ST_LOCAL const StString& getName() const {
        return myParamName;
    }

    /**
     * Set new parameter label.
     */
    ST_LOCAL void setName(const StString& theName) {
        myParamName = theName;
    }

        private:

    StString myParamKey;
    StString myParamName;

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StInt32ParamNamed, StInt32Param);

#endif // __StParam_h_

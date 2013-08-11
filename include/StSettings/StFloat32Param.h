/**
 * Copyright © 2011-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StFloat32Param_h_
#define __StFloat32Param_h_

#include <StSettings/StParam.h>

class StFloat32Param : public StParam<float> {

        protected:

    float myMinValue;
    float myMaxValue;
    float myDefValue;
    float myValueStep;
    float myTolerance;

        public:

    float getMinValue() const {
        return myMinValue;
    }

    float getMaxValue() const {
        return myMaxValue;
    }

    float getDefValue() const {
        return myDefValue;
    }

    float getStep() const {
        return myValueStep;
    }

    float getTolerance() const {
        return myTolerance;
    }

    /**
     * Simple constructor.
     */
    StFloat32Param(const float theValue)
    : StParam<float>(theValue),
      myMinValue(-1E+37f),
      myMaxValue( 1E+37f),
      myDefValue(0.0f),
      myValueStep(1.0f),
      myTolerance(0.0001f) {
        //
    }

    /**
     * Main constructor.
     */
    StFloat32Param(const float theValue,
                   const float theMinValue,
                   const float theMaxValue,
                   const float theDefValue,
                   const float theStep,
                   const float theTolerance = 0.0001f)
    : StParam<float>(theValue),
      myMinValue(theMinValue),
      myMaxValue(theMaxValue),
      myDefValue(theDefValue),
      myValueStep(theStep),
      myTolerance(theTolerance) {
        //
    }

    /**
     * @return true if currently set value is default
     */
    inline bool isDefaultValue() const {
        return areEqual(getValue(), myDefValue);
    }

    /**
     * @return true if currently set value is maximum
     */
    inline bool isMaxValue() const {
        return areEqual(getValue(), myMaxValue);
    }

    /**
     * @return true if currently set value is minimum
     */
    inline bool isMinValue() const {
        return areEqual(getValue(), myMinValue);
    }

    /**
     * Change the value.
     * @param theValue (const float ) - new value.
     */
    virtual bool setValue(const float theValue) {
        const float anOldValue = getValue();
        const float anNewValue
            =  ((theValue + myTolerance) > myMaxValue) ? myMaxValue
            : (((theValue - myTolerance) < myMinValue) ? myMinValue
            : (areEqual(theValue, myDefValue) ? myDefValue : theValue));
        if(anNewValue != anOldValue) {
            myValue = anNewValue;
            signals.onChanged(anNewValue);
            return true;
        }
        return false;
    }

    /**
     * Reset value to default.
     */
    void reset() {
        setValue(myDefValue);
    }

    void increment() {
        setValue(getValue() + myValueStep);
    }

    void decrement() {
        setValue(getValue() - myValueStep);
    }

    /**
     * Prefix ++
     */
    StParam<float>& operator++() {
        setValue(getValue() + myValueStep);
        return (*this);
    }

    /**
     * Prefix --
     */
    StParam<float>& operator--() {
        setValue(getValue() - myValueStep);
        return (*this);
    }

    /**
     * Compare two float values using configured tolerance.
     */
    bool areEqual(const float theFirst,
                  const float theSecond) const {
        return ::stAreEqual(theFirst, theSecond, myTolerance);
    }

};

#endif //__StFloat32Param_h_

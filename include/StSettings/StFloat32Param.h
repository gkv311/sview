/**
 * Copyright © 2011-2016 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StFloat32Param_h_
#define __StFloat32Param_h_

#include <StSettings/StParam.h>

/**
 * Parameter holding float number.
 */
class StFloat32Param : public StParam<float> {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StFloat32Param(const float      theValue,
                            const StCString& theParamKey)
    : StParam<float>(theValue),
      myMinValue(-1E+37f),
      myMaxValue( 1E+37f),
      myDefValue(0.0f),
      myValueStep(1.0f),
      myTolerance(0.0001f),
      myParamKey (theParamKey),
      myParamName(theParamKey) {
        //
    }

    /**
     * Simple constructor.
     */
    ST_LOCAL StFloat32Param(const float theValue)
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
    ST_LOCAL StFloat32Param(const float theValue,
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

    /**
     * Return true if parameter defines minimum value limit.
     */
    ST_LOCAL bool hasMinValue() const {
        return myMinValue != -1E+37f;
    }

    /**
     * Return minimum allowed value.
     */
    ST_LOCAL float getMinValue() const {
        return myMinValue;
    }

    /**
     * Set minimum allowed value.
     */
    ST_LOCAL void setMinValue(float theValue) {
        myMinValue = theValue;
    }

    /**
     * Return true if parameter defines maximum value limit.
     */
    ST_LOCAL bool hasMaxValue() const {
        return myMaxValue != 1E+37f;
    }

    /**
     * Return maximum allowed value.
     */
    ST_LOCAL float getMaxValue() const {
        return myMaxValue;
    }

    /**
     * Set maximum allowed value.
     */
    ST_LOCAL void setMaxValue(float theValue) {
        myMaxValue = theValue;
    }

    /**
     * Set minimum and maximum allowed values.
     */
    ST_LOCAL void setMinMaxValues(float theMinValue,
                                  float theMaxValue) {
        myMinValue = theMinValue;
        myMaxValue = theMaxValue;
    }

    /**
     * Return default value.
     */
    ST_LOCAL float getDefValue() const {
        return myDefValue;
    }

    /**
     * Set default value.
     */
    ST_LOCAL void setDefValue(float theValue) {
        myDefValue = theValue;
    }

    /**
     * Return increment step.
     */
    ST_LOCAL float getStep() const {
        return myValueStep;
    }

    /**
     * Set increment step.
     */
    ST_LOCAL void setStep(float theStep) {
        myValueStep = theStep;
    }

    /**
     * Return tolerance value for equality check.
     */
    ST_LOCAL float getTolerance() const {
        return myTolerance;
    }

    /**
     * Set tolerance value for equality check.
     */
    ST_LOCAL void setTolerance(float theTol) {
        myTolerance = theTol;
    }

    /**
     * @return true if currently set value is default
     */
    ST_LOCAL bool isDefaultValue() const {
        return areEqual(getValue(), myDefValue);
    }

    /**
     * @return true if currently set value is maximum
     */
    ST_LOCAL bool isMaxValue() const {
        return areEqual(getValue(), myMaxValue);
    }

    /**
     * @return true if currently set value is minimum
     */
    ST_LOCAL bool isMinValue() const {
        return areEqual(getValue(), myMinValue);
    }

    /**
     * Change the value.
     * @param theValue new value
     * @return true if value has been changed
     */
    ST_LOCAL virtual bool setValue(const float theValue) {
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
    ST_LOCAL void reset() {
        setValue(myDefValue);
    }

    /**
     * Increase value using default step.
     */
    ST_LOCAL bool increment() {
        return setValue(getValue() + myValueStep);
    }

    /**
     * Decrease value using default step.
     */
    ST_LOCAL bool decrement() {
        return setValue(getValue() - myValueStep);
    }

    /**
     * Prefix ++
     */
    ST_LOCAL StParam<float>& operator++() {
        setValue(getValue() + myValueStep);
        return (*this);
    }

    /**
     * Prefix --
     */
    ST_LOCAL StParam<float>& operator--() {
        setValue(getValue() - myValueStep);
        return (*this);
    }

    /**
     * Compare two float values using configured tolerance.
     */
    ST_LOCAL bool areEqual(const float theFirst,
                           const float theSecond) const {
        return ::stAreEqual(theFirst, theSecond, myTolerance);
    }

    /**
     * Return value within 0..1 range (taking into account min/max values).
     */
    ST_LOCAL float getNormalizedValue() const {
        return (getValue() - myMinValue) / (myMaxValue - myMinValue);
    }

    /**
     * Setup value within 0..1 range (to be scaled according to min/max values).
     */
    ST_LOCAL bool setNormalizedValue(const float theValue) {
        return setValue(myMinValue + theValue * (myMaxValue - myMinValue));
    }

        protected:

    float    myMinValue;  //!< minimal allowed value
    float    myMaxValue;  //!< maximal allowed value
    float    myDefValue;  //!< default value
    float    myValueStep; //!< default increment step
    float    myTolerance; //!< tolerance for equality check

    StString myParamKey;  //!< parameter key (id)
    StString myParamName; //!< parameter name (label)

};

#endif // __StFloat32Param_h_

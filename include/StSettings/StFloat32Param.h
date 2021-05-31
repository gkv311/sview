/**
 * Copyright © 2011-2017 Kirill Gavrilov
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
      myEffMinValue(-1E+37f),
      myEffMaxValue(-1E+37f),
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
      myEffMinValue(-1E+37f),
      myEffMaxValue(-1E+37f),
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
      myEffMinValue(theMinValue),
      myEffMaxValue(theMaxValue),
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
     * @return parameter format
     */
    ST_LOCAL const StString& getFormat() const {
        return myParamFormat;
    }

    /**
     * Set new parameter format.
     */
    ST_LOCAL void setFormat(const StString& theFormat) {
        myParamFormat = theFormat;
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
        myEffMinValue = theValue;
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
        myEffMaxValue = theValue;
    }

    /**
     * Set minimum and maximum allowed values.
     */
    ST_LOCAL void setMinMaxValues(float theMinValue,
                                  float theMaxValue) {
        myMinValue = theMinValue;
        myMaxValue = theMaxValue;
        myEffMinValue = theMinValue;
        myEffMaxValue = theMaxValue;
    }

    /**
     * Return minimum effective value (can be greater then minimum allowed value).
     */
    ST_LOCAL float getEffectiveMinValue() const {
        return myEffMinValue;
    }

    /**
     * Return maximum effective value (can be less then maximum allowed value).
     */
    ST_LOCAL float getEffectiveMaxValue() const {
        return myEffMaxValue;
    }

    /**
     * Set effective minimum and maximum allowed values.
     */
    ST_LOCAL void setEffectiveMinMaxValues(float theMinValue,
                                           float theMaxValue) {
        myEffMinValue = theMinValue;
        myEffMaxValue = theMaxValue;
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
        return (getValue() - myEffMinValue) / (myEffMaxValue - myEffMinValue);
    }

    /**
     * Setup value within 0..1 range (to be scaled according to min/max values).
     */
    ST_LOCAL bool setNormalizedValue(float theValue,
                                     bool  theToRoundToStep = false) {
        float aValue = myEffMinValue + theValue * (myEffMaxValue - myEffMinValue);
        if(theToRoundToStep) {
            aValue = aValue >= 0.0
                   ? (std::floor(aValue / myValueStep + 0.5f) * myValueStep)
                   : (std::ceil (aValue / myValueStep + 0.5f) * myValueStep);
        }
        return setValue(aValue);
    }

        protected:

    float    myMinValue;    //!< minimal allowed value
    float    myMaxValue;    //!< maximal allowed value
    float    myEffMinValue; //!< effective minimal allowed value
    float    myEffMaxValue; //!< effective maximal allowed value
    float    myDefValue;    //!< default value
    float    myValueStep;   //!< default increment step
    float    myTolerance;   //!< tolerance for equality check

    StString myParamKey;    //!< parameter key (id)
    StString myParamName;   //!< parameter name (label)
    StString myParamFormat; //!< parameter format

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StParam<float>, StParamBase);
ST_DEFINE_HANDLE(StFloat32Param, StParam<float>);

#endif // __StFloat32Param_h_

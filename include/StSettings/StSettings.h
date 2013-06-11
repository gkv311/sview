/**
 * Copyright © 2007-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StSettings_h_
#define __StSettings_h_

#include <StStrings/StString.h>
#include <StTemplates/StRect.h>
#include <StTemplates/StVec4.h>
#include <StSettings/StParam.h>

#ifdef __OBJC__
    @class NSMutableDictionary;
#elif defined(__APPLE__)
    struct NSMutableDictionary;
#else
    namespace libconfig {
        class Config;
    };
#endif

/**
 * This is class implements properties persistence.
 * All methods:
 *      1) Take parameter name as first argument (const StString& param, ...);
 *      2) Take values as second+ argument (by address);
 *      3) Return true/false to indicate load/save state;
 *      4) NOT ever broke output values if load failes (values must be unchanged).
 */
class StSettings {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StSettings(const StString& theSettingsSet);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StSettings();

        public: //!< persistance implementation

    /**
     * Load integer value.
     */
    ST_CPPEXPORT virtual bool loadInt32(const StString& theParam,
                                        int32_t&        theOutValue);

    /**
     * Store integer value.
     */
    ST_CPPEXPORT virtual bool saveInt32(const StString& theParam,
                                        const int32_t&  theValue);

    /**
     * Load string value.
     */
    ST_CPPEXPORT virtual bool loadString(const StString& theParam,
                                         StString&       theOutValue);

    /**
     * Store string value.
     */
    ST_CPPEXPORT virtual bool saveString(const StString& theParam,
                                         const StString& theValue);

        public: //!< standard methods for more types that reuse virtual methods

    /**
     * Save boolean value. Will use int32_t storage.
     */
    ST_CPPEXPORT bool loadBool(const StString& theLabel,
                               bool&           theValue);

    /**
     * Load boolean value.
     */
    ST_CPPEXPORT bool saveBool(const StString& theLabel,
                               const bool      theValue);

    /**
     * Load float value.
     */
    ST_CPPEXPORT bool loadFloat(const StString& theLabel,
                                double&         theValue);

    /**
     * Save float value.
     */
    ST_CPPEXPORT bool saveFloat(const StString& theLabel,
                                const double    theValue);

    /**
     * Load float value.
     */
    ST_LOCAL inline bool loadFloat(const StString& theLabel,
                                   float&          theValue) {
        double aValue = double(theValue);
        if(!loadFloat(theLabel, aValue)) {
            return false;
        }
        theValue = float(aValue);
        return true;
    }

    /**
     * Load float value.
     */
    ST_LOCAL inline bool saveFloat(const StString& theLabel,
                                   const float     theValue) {
        return saveFloat(theLabel, double(theValue));
    }

    /**
     * Load integer rectangle.
     */
    ST_CPPEXPORT bool loadInt32Rect(const StString&  theLabel,
                                    StRect<int32_t>& theValue);

    /**
     * Save integer rectangle.
     */
    ST_CPPEXPORT bool saveInt32Rect(const StString&        theLabel,
                                    const StRect<int32_t>& theValue);

    /**
     * Load float vector.
     */
    ST_CPPEXPORT bool loadFloatVec4(const StString& theLabel,
                                    StVec4<float>&  theValue);

    /**
     * Save float vector.
     */
    ST_CPPEXPORT bool saveFloatVec4(const StString&      theLabel,
                                    const StVec4<float>& theValue);

    /**
     * Method to load int32_t parameter.
     * @param theLabel      parameter label (should be unique)
     * @param theInt32Param handle to the parameter
     * @return true if parameter was loaded
     */
    ST_CPPEXPORT bool loadParam(const StString&         theLabel,
                                StHandle<StInt32Param>& theInt32Param);

    /**
     * Method to save int32_t parameter.
     * @param theLabel      parameter label (should be unique)
     * @param theInt32Param handle to the parameter
     * @return true if parameter was saved
     */
    ST_CPPEXPORT bool saveParam(const StString&               theLabel,
                                const StHandle<StInt32Param>& theInt32Param);

    /**
     * Method to load boolean parameter.
     * @param theLabel     parameter label (should be unique)
     * @param theBoolParam handle to the parameter
     * @return true if parameter was loaded
     */
    ST_CPPEXPORT bool loadParam(const StString&        theLabel,
                                StHandle<StBoolParam>& theBoolParam);

    /**
     * Method to save boolean parameter.
     * @param theLabel     parameter label (should be unique)
     * @param theBoolParam handle to the parameter
     * @return true if parameter was saved
     */
    ST_CPPEXPORT bool saveParam(const StString&              theLabel,
                                const StHandle<StBoolParam>& theBoolParam);

        private:

#ifndef _WIN32
    ST_LOCAL bool load();
    ST_LOCAL bool save();
#endif

        private:

#ifdef _WIN32
    StStringUtfWide      mySettingsSet;
    StStringUtfWide      myRegisterPath;
#elif defined(__APPLE__)
    StString             myFilePath;
    NSMutableDictionary* myDict;
#else
    StString             myFullFileName;
    libconfig::Config*   myConfig;
    bool                 myIsLoaded;
#endif

};

#endif //__StSettings_h_

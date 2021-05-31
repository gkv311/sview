/**
 * Copyright © 2007-2016 Kirill Gavrilov
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSettings_h_
#define __StSettings_h_

#include <StStrings/StString.h>
#include <StTemplates/StRect.h>
#include <StTemplates/StVec4.h>
#include <StThreads/StResourceManager.h>
#include <StSettings/StParam.h>
#include <StSettings/StFloat32Param.h>

#ifdef __OBJC__
    @class NSMutableDictionary;
#elif defined(__APPLE__)
    struct NSMutableDictionary;
#else
    namespace libconfig { class Config; }
#endif

class StAction;

/**
 * This is class implements properties persistence.
 * All methods:
 *      1) Take parameter name as first argument (const StString& param, ...);
 *      2) Take values as second+ argument (by address);
 *      3) Return true/false to indicate load/save state;
 *      4) NOT ever broke output values if load fails (values must be unchanged).
 *         This also means that uninitialized values will be kept uninitialized!
 */
class StSettings {

        public:

    /**
     * Main constructor.
     * @param theResMgr     file resources manager
     * @param theModuleName module name to distinguish settings with the same name for different components/applications
     */
    ST_CPPEXPORT StSettings(const StHandle<StResourceManager>& theResMgr,
                            const StString&                    theModuleName);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StSettings();

    /**
     * Immediately write settings into external storage (e.g. file).
     * Has no effect on implementations storing data on saving each individual parameter.
     *
     * Usually settings will be stored automatically within class destruction.
     * This method can be used to perform this more frequently in environments
     * where application can be destroyed in unsafe way.
     *
     * @return true if settings have been saved
     */
    ST_CPPEXPORT bool flush();

        public: //! @name persistence implementation

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

        public: //! @name standard methods for more types that reuse virtual methods

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
     * @param theInt32Param handle to the parameter
     * @return true if parameter was loaded
     */
    ST_CPPEXPORT bool loadParam(StHandle<StInt32ParamNamed>& theInt32Param);

    /**
     * Method to save int32_t parameter.
     * @param theInt32Param handle to the parameter
     * @return true if parameter was saved
     */
    ST_CPPEXPORT bool saveParam(const StHandle<StInt32ParamNamed>& theInt32Param);

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
     * @param theBoolParam handle to the parameter
     * @return true if parameter was loaded
     */
    ST_CPPEXPORT bool loadParam(StHandle<StBoolParamNamed>& theBoolParam);

    /**
     * Method to save boolean parameter.
     * @param theBoolParam handle to the parameter
     * @return true if parameter was saved
     */
    ST_CPPEXPORT bool saveParam(const StHandle<StBoolParamNamed>& theBoolParam);

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

    /**
     * Method to load float parameter.
     * @param theLabel      parameter label (should be unique)
     * @param theFloatParam handle to the parameter
     * @return true if parameter was loaded
     */
    ST_CPPEXPORT bool loadParam(const StString&           theLabel,
                                StHandle<StFloat32Param>& theFloatParam);

    /**
     * Method to load float parameter.
     * @param theFloatParam handle to the parameter
     * @return true if parameter was loaded
     */
    ST_CPPEXPORT bool loadParam(StHandle<StFloat32Param>& theFloatParam);

    /**
     * Method to save float parameter.
     * @param theLabel      parameter label (should be unique)
     * @param theFloatParam handle to the parameter
     * @return true if parameter was saved
     */
    ST_CPPEXPORT bool saveParam(const StString&                 theLabel,
                                const StHandle<StFloat32Param>& theFloatParam);

    /**
     * Method to save float parameter.
     * @param theFloatParam handle to the parameter
     * @return true if parameter was saved
     */
    ST_CPPEXPORT bool saveParam(const StHandle<StFloat32Param>& theFloatParam);

    /**
     * Method to load hot key for action triggering.
     * @param theAction handle to the action
     * @return true if parameter was loaded
     */
    ST_CPPEXPORT bool loadHotKey(StHandle<StAction>& theAction);

    /**
     * Method to save hot keys for action triggering.
     * @param theAction handle to the action
     * @return true if parameter was saved
     */
    ST_CPPEXPORT bool saveHotKey(const StHandle<StAction>& theAction);

        private:

    /**
     * Read settings from external storage.
     * Called within main constructor.
     */
    ST_LOCAL bool load();

        private:

#ifdef _WIN32
    StStringUtfWide      mySettingsSet;
    StStringUtfWide      myRegisterPath;
#elif defined(__APPLE__)
    StString             myFilePath;
    NSMutableDictionary* myDict;
#else
    StString             myFullFileName;  //!< path to the file
    libconfig::Config*   myConfig;        //!< config instance
    bool                 myIsLoaded;      //!< flag indicating last parsing state
#endif
    bool                 myToFlush;       //!< settings have been changed but not yet saved

};

#endif // __StSettings_h_

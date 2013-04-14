/**
 * Copyright © 2007-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StConfigInterface_h_
#define __StConfigInterface_h_

#include <StStrings/StString.h>
#include <StTemplates/StRect.h>
#include <StSettings/StParam.h>

/**
 * This is common interface to provide save/load configuration settings.
 * All methods should:
 *      1) Take parameter name as first argument (const StString& param, ...);
 *      2) Take values as second+ argument (by address);
 *      3) Return true/false to indicate load/save state;
 *      4) NOT ever broke output values if load failes (values must be unchanged).
 */
class StConfigInterface {

        public:

    static const size_t MAX_STRING_LENGHT = 4096U; //!< currently all string parameters are limited within this length

        public: //!< here is virtual methods that actually process storing / restoring parameters values

    virtual ~StConfigInterface() {} // never forget it!

    virtual bool loadInt32(const StString& theLabel, int32_t&       theValue) = 0;
    virtual bool saveInt32(const StString& theLabel, const int32_t& theValue) = 0;

    virtual bool loadString(const StString& theLabel, StString&       theValue) = 0;
    virtual bool saveString(const StString& theLabel, const StString& theValue) = 0;

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

};

#endif //__StConfigInterface_h_
